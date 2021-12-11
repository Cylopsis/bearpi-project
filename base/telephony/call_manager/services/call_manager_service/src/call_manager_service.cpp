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

#include "call_manager_service.h"

#include <ctime>
#include <string_ex.h>

#include "telephony_log_wrapper.h"

#include "call_manager_errors.h"
#include "call_ability_report_ipc_proxy.h"
#include "common_type.h"
#include "cellular_call_info_handler.h"
#include "call_ability_handler.h"
#include "cellular_call_ipc_interface_proxy.h"
#include "audio_control_manager.h"

namespace OHOS {
namespace Telephony {
const bool g_registerResult =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<CallManagerService>::GetInstance().get());

CallManagerService::CallManagerService()
    : SystemAbility(TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID, true), callControlManagerPtr_(nullptr),
      audioControlManagerPtr_(nullptr)
{}

CallManagerService::~CallManagerService()
{
    UnInit();
}

bool CallManagerService::Init()
{
    DelayedSingleton<CallStateReportProxy>::GetInstance()->Init(TELEPHONY_STATE_REGISTRY_SYS_ABILITY_ID);
    if (!DelayedSingleton<CallControlManager>::GetInstance()->Init()) {
        TELEPHONY_LOGE("CallControlManager init failed!");
        return false;
    }
    callControlManagerPtr_ = DelayedSingleton<CallControlManager>::GetInstance();
    if (callControlManagerPtr_ == nullptr) {
        TELEPHONY_LOGE("callControlManagerPtr_ is nullptr!");
        return false;
    }
    audioControlManagerPtr_ = DelayedSingleton<AudioControlManager>::GetInstance();
    if (audioControlManagerPtr_ == nullptr) {
        TELEPHONY_LOGE("audioControlManagerPtr_ is nullptr!");
        return false;
    }
    callSettingManagerPtr_ = DelayedSingleton<CallSettingManager>::GetInstance();
    if (callSettingManagerPtr_ == nullptr) {
        TELEPHONY_LOGE("callSettingManagerPtr_ is nullptr!");
        return false;
    }
    callNumberUtilsPtr_ = DelayedSingleton<CallNumberUtils>::GetInstance();
    if (callNumberUtilsPtr_ == nullptr) {
        TELEPHONY_LOGE("callNumberUtilsPtr_ is nullptr!");
        return false;
    }
    DelayedSingleton<AudioControlManager>::GetInstance()->Init();
    DelayedSingleton<CellularCallInfoHandlerService>::GetInstance()->Start();
    DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->Init(TELEPHONY_CELLULAR_CALL_SYS_ABILITY_ID);
    DelayedSingleton<CallAbilityHandlerService>::GetInstance()->Start();
    return true;
}

void CallManagerService::UnInit()
{
    if (callControlManagerPtr_ != nullptr) {
        callControlManagerPtr_ = nullptr;
    }
}

void CallManagerService::OnDump()
{
    std::lock_guard<std::mutex> guard(lock_);
    struct tm *timeNow;
    time_t second = time(0);
    timeNow = localtime(&second);
    if (timeNow != nullptr) {
        TELEPHONY_LOGI(
            "CallManagerService dump time:%{public}d-%{public}d-%{public}d %{public}d:%{public}d:%{public}d",
            timeNow->tm_year + startTime_, timeNow->tm_mon + extraMonth_, timeNow->tm_mday, timeNow->tm_hour,
            timeNow->tm_min, timeNow->tm_sec);
    }
}

void CallManagerService::OnStart()
{
    std::lock_guard<std::mutex> guard(lock_);
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        return;
    }

    if (!Init()) {
        TELEPHONY_LOGE("Leave, init failed!");
        return;
    }

    bool ret = SystemAbility::Publish(DelayedSingleton<CallManagerService>::GetInstance().get());
    if (!ret) {
        TELEPHONY_LOGE("Leave, publishing CallManagerService failed!");
        return;
    }
    TELEPHONY_LOGI("Publish CallManagerService SUCCESS");

    state_ = ServiceRunningState::STATE_RUNNING;
    struct tm *timeNow;
    time_t second = time(0);
    timeNow = localtime(&second);
    if (timeNow != nullptr) {
        TELEPHONY_LOGI(
            "CallManagerService start time:%{public}d-%{public}d-%{public}d %{public}d:%{public}d:%{public}d",
            timeNow->tm_year + startTime_, timeNow->tm_mon + extraMonth_, timeNow->tm_mday, timeNow->tm_hour,
            timeNow->tm_min, timeNow->tm_sec);
    }
}

void CallManagerService::OnStop()
{
    std::lock_guard<std::mutex> guard(lock_);
    state_ = ServiceRunningState::STATE_STOPPED;
}

int32_t CallManagerService::RegisterCallBack(const sptr<ICallAbilityCallback> &callback)
{
    return DelayedSingleton<CallAbilityReportIpcProxy>::GetInstance()->RegisterCallBack(callback);
}

int32_t CallManagerService::DialCall(std::u16string number, AppExecFwk::PacMap &extras)
{
    if (callControlManagerPtr_ != nullptr) {
        return callControlManagerPtr_->DialCall(number, extras);
    } else {
        TELEPHONY_LOGE("callControlManagerPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

int32_t CallManagerService::AnswerCall(int32_t callId, int32_t videoState)
{
    if (callControlManagerPtr_ != nullptr) {
        return callControlManagerPtr_->AnswerCall(callId, videoState);
    } else {
        TELEPHONY_LOGE("callControlManagerPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

int32_t CallManagerService::RejectCall(int32_t callId, bool rejectWithMessage, std::u16string textMessage)
{
    if (callControlManagerPtr_ != nullptr) {
        return callControlManagerPtr_->RejectCall(callId, rejectWithMessage, textMessage);
    } else {
        TELEPHONY_LOGE("callControlManagerPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

int32_t CallManagerService::HangUpCall(int32_t callId)
{
    if (callControlManagerPtr_ != nullptr) {
        return callControlManagerPtr_->HangUpCall(callId);
    } else {
        TELEPHONY_LOGE("callControlManagerPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

int32_t CallManagerService::GetCallState()
{
    if (callControlManagerPtr_ != nullptr) {
        return callControlManagerPtr_->GetCallState();
    } else {
        TELEPHONY_LOGE("callControlManagerPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

int32_t CallManagerService::HoldCall(int32_t callId)
{
    if (callControlManagerPtr_ != nullptr) {
        return callControlManagerPtr_->HoldCall(callId);
    } else {
        TELEPHONY_LOGE("callControlManagerPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

int32_t CallManagerService::UnHoldCall(int32_t callId)
{
    if (callControlManagerPtr_ != nullptr) {
        return callControlManagerPtr_->UnHoldCall(callId);
    } else {
        TELEPHONY_LOGE("callControlManagerPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

int32_t CallManagerService::SwitchCall(int32_t callId)
{
    if (callControlManagerPtr_ != nullptr) {
        return callControlManagerPtr_->SwitchCall();
    } else {
        TELEPHONY_LOGE("callControlManagerPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

bool CallManagerService::HasCall()
{
    if (callControlManagerPtr_ != nullptr) {
        return callControlManagerPtr_->HasCall();
    } else {
        TELEPHONY_LOGE("callControlManagerPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

bool CallManagerService::IsNewCallAllowed()
{
    if (callControlManagerPtr_ != nullptr) {
        return callControlManagerPtr_->IsNewCallAllowed();
    } else {
        TELEPHONY_LOGE("callControlManagerPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

bool CallManagerService::IsRinging()
{
    if (callControlManagerPtr_ != nullptr) {
        return callControlManagerPtr_->IsRinging();
    } else {
        TELEPHONY_LOGE("callControlManagerPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

bool CallManagerService::IsInEmergencyCall()
{
    if (callControlManagerPtr_ != nullptr) {
        return callControlManagerPtr_->HasEmergency();
    } else {
        TELEPHONY_LOGE("callControlManagerPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

int32_t CallManagerService::StartDtmf(int32_t callId, char str)
{
    if (callControlManagerPtr_ != nullptr) {
        return callControlManagerPtr_->StartDtmf(callId, str);
    } else {
        TELEPHONY_LOGE("callControlManagerPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

int32_t CallManagerService::SendDtmf(int32_t callId, char str)
{
    if (callControlManagerPtr_ != nullptr) {
        return callControlManagerPtr_->SendDtmf(callId, str);
    } else {
        TELEPHONY_LOGE("callControlManagerPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

int32_t CallManagerService::SendBurstDtmf(int32_t callId, std::u16string str, int32_t on, int32_t off)
{
    if (callControlManagerPtr_ != nullptr) {
        return callControlManagerPtr_->SendBurstDtmf(callId, str, on, off);
    } else {
        TELEPHONY_LOGE("callControlManagerPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

int32_t CallManagerService::StopDtmf(int32_t callId)
{
    if (callControlManagerPtr_ != nullptr) {
        return callControlManagerPtr_->StopDtmf(callId);
    } else {
        TELEPHONY_LOGE("callControlManagerPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

int32_t CallManagerService::GetCallWaiting(int32_t slotId)
{
    if (callSettingManagerPtr_ != nullptr) {
        return callSettingManagerPtr_->GetCallWaiting(slotId);
    } else {
        TELEPHONY_LOGE("callSettingManagerPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

int32_t CallManagerService::SetCallWaiting(int32_t slotId, bool activate)
{
    if (callSettingManagerPtr_ != nullptr) {
        return callSettingManagerPtr_->SetCallWaiting(slotId, activate);
    } else {
        TELEPHONY_LOGE("callSettingManagerPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

int32_t CallManagerService::CombineConference(int32_t mainCallId)
{
    RETURN_FAILURE_IF_NULLPTR(callControlManagerPtr_, "callControlManagerPtr_ is null", TELEPHONY_LOCAL_PTR_NULL);
    return callControlManagerPtr_->CombineConference(mainCallId);
    TELEPHONY_LOGD("leave");
}

bool CallManagerService::IsEmergencyPhoneNumber(std::u16string &number, int32_t slotId, int32_t &errorCode)
{
    if (callNumberUtilsPtr_ != nullptr) {
        return callNumberUtilsPtr_->CheckNumberIsEmergency(Str16ToStr8(number), slotId, errorCode);
    } else {
        TELEPHONY_LOGE("callNumberUtilsPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

int32_t CallManagerService::FormatPhoneNumber(
    std::u16string &number, std::u16string &countryCode, std::u16string &formatNumber)
{
    if (callNumberUtilsPtr_ != nullptr) {
        std::string tmpStr("");
        int32_t ret = callNumberUtilsPtr_->FormatPhoneNumber(Str16ToStr8(number), Str16ToStr8(countryCode), tmpStr);
        formatNumber = Str8ToStr16(tmpStr);
        return ret;
    } else {
        TELEPHONY_LOGE("callNumberUtilsPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

int32_t CallManagerService::FormatPhoneNumberToE164(
    std::u16string &number, std::u16string &countryCode, std::u16string &formatNumber)
{
    if (callNumberUtilsPtr_ != nullptr) {
        std::string tmpStr("");
        int32_t ret =
            callNumberUtilsPtr_->FormatPhoneNumberToE164(Str16ToStr8(number), Str16ToStr8(countryCode), tmpStr);
        formatNumber = Str8ToStr16(tmpStr);
        return ret;
    } else {
        TELEPHONY_LOGE("callNumberUtilsPtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
}

int32_t CallManagerService::GetMainCallId(int32_t callId)
{
    RETURN_FAILURE_IF_NULLPTR(callControlManagerPtr_, "callControlManagerPtr_ is null", TELEPHONY_LOCAL_PTR_NULL);
    return callControlManagerPtr_->GetMainCallId(callId);
}

std::vector<std::u16string> CallManagerService::GetSubCallIdList(int32_t callId)
{
    if (callControlManagerPtr_ != nullptr) {
        return callControlManagerPtr_->GetSubCallIdList(callId);
    }
    std::vector<std::u16string> vec;
    vec.clear();
    return vec;
}

std::vector<std::u16string> CallManagerService::GetCallIdListForConference(int32_t callId)
{
    if (callControlManagerPtr_ != nullptr) {
        return callControlManagerPtr_->GetCallIdListForConference(callId);
    }
    std::vector<std::u16string> vec;
    vec.clear();
    return vec;
}
} // namespace Telephony
} // namespace OHOS
