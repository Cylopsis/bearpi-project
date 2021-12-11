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

#include "call_manager_ipc_client.h"

#include "iservice_registry.h"
#include "system_ability.h"
#include "system_ability_definition.h"

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
CallManagerIpcClient::CallManagerIpcClient()
    : systemAbilityId_(TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID), callManagerServicePtr_(nullptr)
{}

CallManagerIpcClient::~CallManagerIpcClient()
{
    if (callManagerServicePtr_) {
        callManagerServicePtr_.clear();
        callManagerServicePtr_ = nullptr;
    }
}

int32_t CallManagerIpcClient::Init(int32_t systemAbilityId)
{
    systemAbilityId_ = systemAbilityId;
    int32_t result = ConnectService();
    TELEPHONY_LOGD("Connect service: %X\n", result);
    return result;
}

void CallManagerIpcClient::UnInit()
{
    DisconnectService();
    TELEPHONY_LOGD("Disconnect service\n");
}

int32_t CallManagerIpcClient::ConnectService()
{
    int32_t ret = TELEPHONY_FAIL;
    if (callManagerServicePtr_ != nullptr) {
        TELEPHONY_LOGE("Already get telephony base service\n");
        return ret;
    }

    sptr<ISystemAbilityManager> managerPtr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (managerPtr == nullptr) {
        TELEPHONY_LOGE("GetSystemAbilityManager null\n");
        return ret;
    }

    sptr<ICallManagerService> callManagerServicePtr =
        iface_cast<ICallManagerService>(managerPtr->GetSystemAbility(systemAbilityId_));
    if (callManagerServicePtr == nullptr) {
        TELEPHONY_LOGE("GetSystemAbility null\n");
        return ret;
    }

    TELEPHONY_LOGD("Connect callManagerService success\n");
    callManagerServicePtr_ = callManagerServicePtr;
    return TELEPHONY_SUCCESS;
}

void CallManagerIpcClient::DisconnectService()
{
    if (callManagerServicePtr_ == nullptr) {
        callManagerServicePtr_ = nullptr;
    }
}

int32_t CallManagerIpcClient::DialCall(std::u16string number, AppExecFwk::PacMap &extras) const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->DialCall(number, extras);
    }
    return TELEPHONY_FAIL;
}

int32_t CallManagerIpcClient::AnswerCall(int32_t callId, int32_t videoState) const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->AnswerCall(callId, videoState);
    }
    return TELEPHONY_FAIL;
}

int32_t CallManagerIpcClient::RejectCall(int32_t callId, bool isSendSms, std::u16string content) const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->RejectCall(callId, isSendSms, content);
    }
    return TELEPHONY_FAIL;
}

int32_t CallManagerIpcClient::HoldCall(int32_t callId) const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->HoldCall(callId);
    }
    return TELEPHONY_FAIL;
}

int32_t CallManagerIpcClient::UnHoldCall(int32_t callId) const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->UnHoldCall(callId);
    }
    return TELEPHONY_FAIL;
}

int32_t CallManagerIpcClient::HangUpCall(int32_t callId) const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->HangUpCall(callId);
    }
    return TELEPHONY_FAIL;
}

int32_t CallManagerIpcClient::GetCallState() const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->GetCallState();
    }
    return TELEPHONY_FAIL;
}

int32_t CallManagerIpcClient::SwitchCall(int32_t callId) const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->SwitchCall(callId);
    }
    return TELEPHONY_FAIL;
}

bool CallManagerIpcClient::HasCall() const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->HasCall();
    }
    return false;
}

bool CallManagerIpcClient::IsNewCallAllowed() const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->IsNewCallAllowed();
    }
    return false;
}

bool CallManagerIpcClient::IsRinging() const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->IsRinging();
    }
    return false;
}

bool CallManagerIpcClient::IsInEmergencyCall() const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->IsInEmergencyCall();
    }
    return false;
}

int32_t CallManagerIpcClient::StartDtmf(int32_t callId, char c) const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->StartDtmf(callId, c);
    }
    return TELEPHONY_FAIL;
}

int32_t CallManagerIpcClient::SendDtmf(int32_t callId, char c) const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->SendDtmf(callId, c);
    }
    return TELEPHONY_FAIL;
}

int32_t CallManagerIpcClient::StopDtmf(int32_t callId) const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->StopDtmf(callId);
    }
    return TELEPHONY_FAIL;
}

int32_t CallManagerIpcClient::SendBurstDtmf(int32_t callId, std::u16string str, int32_t on, int32_t off) const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->SendBurstDtmf(callId, str, on, off);
    }
    return TELEPHONY_FAIL;
}

int32_t CallManagerIpcClient::GetCallWaiting(int32_t slotId) const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->GetCallWaiting(slotId);
    }
    return TELEPHONY_FAIL;
}

int32_t CallManagerIpcClient::SetCallWaiting(int32_t slotId, bool activate) const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->SetCallWaiting(slotId, activate);
    }
    return TELEPHONY_FAIL;
}

int32_t CallManagerIpcClient::CombineConference(int32_t mainCallId) const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->CombineConference(mainCallId);
    }
    return TELEPHONY_FAIL;
}

bool CallManagerIpcClient::IsEmergencyPhoneNumber(std::u16string &number, int32_t slotId, int32_t &errorCode)
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->IsEmergencyPhoneNumber(number, slotId, errorCode);
    }
    return false;
}

int32_t CallManagerIpcClient::FormatPhoneNumber(
    std::u16string &number, std::u16string &countryCode, std::u16string &formatNumber) const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->FormatPhoneNumber(number, countryCode, formatNumber);
    }
    return TELEPHONY_FAIL;
}

int32_t CallManagerIpcClient::FormatPhoneNumberToE164(
    std::u16string &number, std::u16string &countryCode, std::u16string &formatNumber) const
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->FormatPhoneNumberToE164(number, countryCode, formatNumber);
    }
    return TELEPHONY_FAIL;
}

int32_t CallManagerIpcClient::GetMainCallId(int32_t callId)
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->GetMainCallId(callId);
    }
    return TELEPHONY_FAIL;
}

std::vector<std::u16string> CallManagerIpcClient::GetSubCallIdList(int32_t callId)
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->GetSubCallIdList(callId);
    }
    std::vector<std::u16string> vec;
    vec.clear();
    return vec;
}

std::vector<std::u16string> CallManagerIpcClient::GetCallIdListForConference(int32_t callId)
{
    if (callManagerServicePtr_ != nullptr) {
        return callManagerServicePtr_->GetCallIdListForConference(callId);
    }
    std::vector<std::u16string> vec;
    vec.clear();
    return vec;
}
} // namespace Telephony
} // namespace OHOS