/*
 * Copyright (C) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "call_manager_proxy.h"

#include "iservice_registry.h"
#include "system_ability.h"
#include "system_ability_definition.h"

#include "call_ability_callback.h"
#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
CallManagerProxy::CallManagerProxy()
    : systemAbilityId_(TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID), callManagerServicePtr_(nullptr),
      callAbilityCallbackPtr_(nullptr)
{}

CallManagerProxy::~CallManagerProxy()
{
    UnInit();
}

void CallManagerProxy::Init(int32_t systemAbilityId)
{
    systemAbilityId_ = systemAbilityId;
    int32_t result = ConnectService();
    if (result != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("connect service failed,errCode: %{public}d", result);
        Timer::start(CONNECT_SERVICE_WAIT_TIME, CallManagerProxy::task);
        return;
    }
    TELEPHONY_LOGD("connected to call manager service successfully!");
}

void CallManagerProxy::UnInit()
{
    DisconnectService();
    TELEPHONY_LOGD("disconnect service\n");
}

void CallManagerProxy::task()
{
    int32_t ret = DelayedSingleton<CallManagerProxy>::GetInstance()->ConnectService();
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("call manager service reconnection failed! errCode:%{public}d", ret);
        return;
    }
    DelayedSingleton<CallManagerProxy>::GetInstance()->ThreadExit();
    TELEPHONY_LOGI("call manager service reconnection successfully!");
}

int32_t CallManagerProxy::ConnectService()
{
    Utils::UniqueWriteGuard<Utils::RWLock> guard(rwClientLock_);
    if (callManagerServicePtr_ != nullptr) {
        return TELEPHONY_SUCCESS;
    }
    sptr<ISystemAbilityManager> managerPtr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (managerPtr == nullptr) {
        return TELEPHONY_FAIL;
    }
    sptr<ICallManagerService> callManagerServicePtr = nullptr;
    sptr<IRemoteObject> iRemoteObjectPtr = managerPtr->GetSystemAbility(systemAbilityId_);
    if (iRemoteObjectPtr == nullptr) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    callManagerServicePtr = iface_cast<ICallManagerService>(iRemoteObjectPtr);
    if (!callManagerServicePtr) {
        return TELEPHONY_LOCAL_PTR_NULL;
    }
    callManagerServicePtr_ = callManagerServicePtr;
    int32_t ret = RegisterCallBack();
    if (ret != TELEPHONY_SUCCESS) {
        return ret;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerProxy::ReConnectService()
{
    if (callManagerServicePtr_ == nullptr) {
        int32_t result = ConnectService();
        if (result != TELEPHONY_SUCCESS) {
            TELEPHONY_LOGE("Connect service: %{public}d", result);
            return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
        }
    }
    return TELEPHONY_SUCCESS;
}

void CallManagerProxy::DisconnectService()
{
    Clean();
}

void CallManagerProxy::Clean()
{
    Utils::UniqueWriteGuard<Utils::RWLock> guard(rwClientLock_);
    if (callManagerServicePtr_ != nullptr) {
        callManagerServicePtr_.clear();
        callManagerServicePtr_ = nullptr;
    }
    if (callAbilityCallbackPtr_ != nullptr) {
        callAbilityCallbackPtr_.clear();
        callAbilityCallbackPtr_ = nullptr;
    }
}

void CallManagerProxy::OnDeath()
{
    Clean();
    NotifyDeath();
}

void CallManagerProxy::NotifyDeath()
{
    TELEPHONY_LOGD("NotifyDeath");
    Timer::start(CONNECT_SERVICE_WAIT_TIME, CallManagerProxy::task);
}

int32_t CallManagerProxy::DialCall(std::u16string number, AppExecFwk::PacMap &extras)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t errCode = callManagerServicePtr_->DialCall(number, extras);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("DialCall failed, errcode:%{public}d", errCode);
        return errCode;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerProxy::AnswerCall(int32_t callId, int32_t videoState)
{
    TELEPHONY_LOGE("start");
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t errCode = callManagerServicePtr_->AnswerCall(callId, videoState);
    TELEPHONY_LOGE("errCode = %{public}d", errCode);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("AnswerCall failed, errcode:%{public}d", errCode);
        return errCode;
    }
    TELEPHONY_LOGE("end");
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerProxy::RejectCall(int32_t callId, bool isSendSms, std::u16string content)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t errCode = callManagerServicePtr_->RejectCall(callId, isSendSms, content);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("RejectCall failed, errcode:%{public}d", errCode);
        return errCode;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerProxy::HangUpCall(int32_t callId)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t errCode = callManagerServicePtr_->HangUpCall(callId);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("HangUpCall failed, errcode:%{public}d", errCode);
        return errCode;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerProxy::GetCallState()
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t errCode = callManagerServicePtr_->GetCallState();
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("GetCallState failed, errcode:%{public}d", errCode);
        return errCode;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerProxy::HoldCall(int32_t callId)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t errCode = callManagerServicePtr_->HoldCall(callId);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("HoldCall failed, errcode:%{public}d", errCode);
        return errCode;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerProxy::UnHoldCall(int32_t callId)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t errCode = callManagerServicePtr_->UnHoldCall(callId);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("UnHoldCall failed, errcode:%{public}d", errCode);
        return errCode;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerProxy::SwitchCall(int32_t callId)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t errCode = callManagerServicePtr_->SwitchCall(callId);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("SwitchCall failed, errcode:%{public}d", errCode);
        return errCode;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerProxy::CombineConference(int32_t callId)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t errCode = callManagerServicePtr_->CombineConference(callId);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("CombineConference failed, errcode:%{public}d", errCode);
        return errCode;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerProxy::GetMainCallId(int32_t &callId)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t errCode = callManagerServicePtr_->GetMainCallId(callId);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("GetMainCallId failed, errcode:%{public}d", errCode);
        return errCode;
    }
    return TELEPHONY_SUCCESS;
}

std::vector<std::u16string> CallManagerProxy::GetSubCallIdList(int32_t callId)
{
    std::vector<std::u16string> list;
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return list;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    list = callManagerServicePtr_->GetSubCallIdList(callId);
    return list;
}

std::vector<std::u16string> CallManagerProxy::GetCallIdListForConference(int32_t callId)
{
    std::vector<std::u16string> list;
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return list;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    list = callManagerServicePtr_->GetCallIdListForConference(callId);
    return list;
}

int32_t CallManagerProxy::GetCallWaiting(int32_t slotId)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t errCode = callManagerServicePtr_->GetCallWaiting(slotId);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("GetCallWaiting failed, errcode:%{public}d", errCode);
        return errCode;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerProxy::SetCallWaiting(int32_t slotId, bool activate)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t errCode = callManagerServicePtr_->SetCallWaiting(slotId, activate);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("SetCallWaiting failed, errcode:%{public}d", errCode);
        return errCode;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerProxy::StartDtmf(int32_t callId, char str)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t errCode = callManagerServicePtr_->StartDtmf(callId, str);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("StartDtmf failed, errcode:%{public}d", errCode);
        return errCode;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerProxy::StopDtmf(int32_t callId)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t errCode = callManagerServicePtr_->StopDtmf(callId);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("StopDtmf failed, errcode:%{public}d", errCode);
        return errCode;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerProxy::SendDtmf(int32_t callId, char str)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t errCode = callManagerServicePtr_->SendDtmf(callId, str);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("SendDtmf failed, errcode:%{public}d", errCode);
        return errCode;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerProxy::SendBurstDtmf(int32_t callId, std::u16string str, int32_t on, int32_t off)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t errCode = callManagerServicePtr_->SendBurstDtmf(callId, str, on, off);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("SendBurstDtmf failed, errcode:%{public}d", errCode);
        return errCode;
    }
    return TELEPHONY_SUCCESS;
}

bool CallManagerProxy::IsRinging()
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    return callManagerServicePtr_->IsRinging();
}

bool CallManagerProxy::HasCall()
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    return callManagerServicePtr_->HasCall();
}

bool CallManagerProxy::IsNewCallAllowed()
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    return callManagerServicePtr_->IsNewCallAllowed();
}

bool CallManagerProxy::IsInEmergencyCall()
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    return callManagerServicePtr_->IsInEmergencyCall();
}

bool CallManagerProxy::IsEmergencyPhoneNumber(std::u16string &number, int32_t slotId, int32_t &errorCode)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    return callManagerServicePtr_->IsEmergencyPhoneNumber(number, slotId, errorCode);
}

int32_t CallManagerProxy::FormatPhoneNumber(
    std::u16string &number, std::u16string &countryCode, std::u16string &formatNumber)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t errCode = callManagerServicePtr_->FormatPhoneNumber(number, countryCode, formatNumber);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("FormatPhoneNumber failed, errcode:%{public}d", errCode);
        return errCode;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerProxy::FormatPhoneNumberToE164(
    std::u16string &number, std::u16string &countryCode, std::u16string &formatNumber)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int32_t errCode = callManagerServicePtr_->FormatPhoneNumberToE164(number, countryCode, formatNumber);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("FormatPhoneNumberToE164 failed, errcode:%{public}d", errCode);
        return errCode;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerProxy::RegisterCallBack()
{
    if (callManagerServicePtr_ == nullptr) {
        TELEPHONY_LOGE("callManagerServicePtr_ is null");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
    callAbilityCallbackPtr_ = (std::make_unique<CallAbilityCallback>()).release();
    if (callAbilityCallbackPtr_ == nullptr) {
        Clean();
        TELEPHONY_LOGE("create CallAbilityCallback object failed!");
        return TELEPHONY_FAIL;
    }

    int32_t ret = callManagerServicePtr_->RegisterCallBack(callAbilityCallbackPtr_);
    if (ret != TELEPHONY_SUCCESS) {
        Clean();
        TELEPHONY_LOGE("register callback to call manager service failed,result: %{public}d", ret);
        return TELEPHONY_REGISTER_CALLBACK_FAIL;
    }
    TELEPHONY_LOGI("register call ability callback success!");
    return TELEPHONY_SUCCESS;
}
} // namespace Telephony
} // namespace OHOS