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

#include "ott_call.h"

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
OTTCall::OTTCall() {}

OTTCall::~OTTCall() {}

void OTTCall::OutCallInit(const CallReportInfo &info, AppExecFwk::PacMap &extras, int32_t callId) {}

void OTTCall::InCallInit(const CallReportInfo &info, int32_t callId) {}

int32_t OTTCall::DialingProcess()
{
    return TELEPHONY_SUCCESS;
}

int32_t OTTCall::AnswerCall(int32_t videoState)
{
    return TELEPHONY_SUCCESS;
}

int32_t OTTCall::RejectCall(bool isSendSms, std::string &content)
{
    return TELEPHONY_SUCCESS;
}

int32_t OTTCall::HangUpCall()
{
    return TELEPHONY_SUCCESS;
}

int32_t OTTCall::HoldCall()
{
    return CALL_MANAGER_ILLEGAL_CALL_OPERATION;
}

int32_t OTTCall::UnHoldCall()
{
    return CALL_MANAGER_ILLEGAL_CALL_OPERATION;
}

int32_t OTTCall::SwitchCall()
{
    return CALL_MANAGER_ILLEGAL_CALL_OPERATION;
}

void OTTCall::GetCallAttributeInfo(CallAttributeInfo &info)
{
    GetCallAttributeBaseInfo(info);
}

bool OTTCall::GetEmergencyState()
{
    return false;
}

int32_t OTTCall::StartDtmf(std::string &phoneNum, char str)
{
    return CALL_MANAGER_ILLEGAL_CALL_OPERATION;
}

int32_t OTTCall::StopDtmf(std::string &phoneNum)
{
    return CALL_MANAGER_ILLEGAL_CALL_OPERATION;
}

int32_t OTTCall::SendDtmf(std::string &phoneNum, char str)
{
    return CALL_MANAGER_ILLEGAL_CALL_OPERATION;
}

int32_t OTTCall::SendBurstDtmf(std::string &phoneNum, std::string str, int32_t on, int32_t off)
{
    return CALL_MANAGER_ILLEGAL_CALL_OPERATION;
}

int32_t OTTCall::GetSlotId()
{
    return TELEPHONY_SUCCESS;
}

int32_t OTTCall::CombineConference()
{
    return TELEPHONY_SUCCESS;
}

int32_t OTTCall::CanCombineConference()
{
    return TELEPHONY_SUCCESS;
}

int32_t OTTCall::SubCallCombineToConference()
{
    return TELEPHONY_SUCCESS;
}

int32_t OTTCall::SubCallSeparateFromConference()
{
    return TELEPHONY_SUCCESS;
}

int32_t OTTCall::CanSeparateConference()
{
    return TELEPHONY_SUCCESS;
}

int32_t OTTCall::GetMainCallId()
{
    return TELEPHONY_SUCCESS;
}

std::vector<std::u16string> OTTCall::GetSubCallIdList()
{
    std::vector<std::u16string> vec;
    return vec;
}

std::vector<std::u16string> OTTCall::GetCallIdListForConference()
{
    std::vector<std::u16string> vec;
    return vec;
}
} // namespace Telephony
} // namespace OHOS
