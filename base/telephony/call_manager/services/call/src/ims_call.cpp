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

#include "ims_call.h"

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
IMSCall::IMSCall() {}

IMSCall::~IMSCall() {}

void IMSCall::OutCallInit(const CallReportInfo &info, AppExecFwk::PacMap &extras, int32_t callId)
{
    InitCarrierOutCallInfo(info, extras, callId);
    callType_ = CallType::TYPE_IMS;
}

void IMSCall::InCallInit(const CallReportInfo &info, int32_t callId)
{
    InitCarrierInCallInfo(info, callId);
    callType_ = CallType::TYPE_IMS;
}

int32_t IMSCall::DialingProcess()
{
    return CarrierDialingProcess();
}

int32_t IMSCall::AnswerCall(int32_t videoState)
{
    return CarrierAcceptCall(videoState);
}

int32_t IMSCall::RejectCall(bool isSendSms, std::string &content)
{
    return CarrierRejectCall(isSendSms, content);
}

int32_t IMSCall::HangUpCall()
{
    return CarrierHangUpCall();
}

int32_t IMSCall::HoldCall()
{
    return CarrierHoldCall();
}

int32_t IMSCall::UnHoldCall()
{
    return CarrierUnHoldCall();
}

int32_t IMSCall::SwitchCall()
{
    return CarrierSwitchCall();
}

void IMSCall::GetCallAttributeInfo(CallAttributeInfo &info)
{
    GetCallAttributeCarrierInfo(info);
}

int32_t IMSCall::CombineConference()
{
    return TELEPHONY_SUCCESS;
}

int32_t IMSCall::CanCombineConference()
{
    return TELEPHONY_SUCCESS;
}

int32_t IMSCall::SubCallCombineToConference()
{
    return TELEPHONY_SUCCESS;
}

int32_t IMSCall::SubCallSeparateFromConference()
{
    return TELEPHONY_SUCCESS;
}

int32_t IMSCall::CanSeparateConference()
{
    return TELEPHONY_SUCCESS;
}

int32_t IMSCall::GetMainCallId()
{
    return TELEPHONY_SUCCESS;
}

std::vector<std::u16string> IMSCall::GetSubCallIdList()
{
    std::vector<std::u16string> vec;
    return vec;
}

std::vector<std::u16string> IMSCall::GetCallIdListForConference()
{
    std::vector<std::u16string> vec;
    return vec;
}
} // namespace Telephony
} // namespace OHOS
