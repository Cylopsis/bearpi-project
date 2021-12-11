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

#include "cs_call.h"

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
CSCall::CSCall() {}

CSCall::~CSCall() {}

void CSCall::OutCallInit(const CallReportInfo &info, AppExecFwk::PacMap &extras, int32_t callId)
{
    InitCarrierOutCallInfo(info, extras, callId);
    callType_ = CallType::TYPE_CS;
}

void CSCall::InCallInit(const CallReportInfo &info, int32_t callId)
{
    InitCarrierInCallInfo(info, callId);
    callType_ = CallType::TYPE_CS;
}

int32_t CSCall::DialingProcess()
{
    return CarrierDialingProcess();
}

int32_t CSCall::AnswerCall(int32_t videoState)
{
    return CarrierAcceptCall(videoState);
}

int32_t CSCall::RejectCall(bool isSendSms, std::string &content)
{
    return CarrierRejectCall(isSendSms, content);
}

int32_t CSCall::HangUpCall()
{
    return CarrierHangUpCall();
}

int32_t CSCall::HoldCall()
{
    return CarrierHoldCall();
}

int32_t CSCall::UnHoldCall()
{
    return CarrierUnHoldCall();
}

int32_t CSCall::SwitchCall()
{
    return CarrierSwitchCall();
}

void CSCall::GetCallAttributeInfo(CallAttributeInfo &info)
{
    GetCallAttributeCarrierInfo(info);
}

int32_t CSCall::CombineConference()
{
    int32_t ret = TELEPHONY_ERROR;
    ret = SetMainCall(GetCallID());
    if (ret != TELEPHONY_SUCCESS) {
        return ret;
    }
    return CarrierCombineConference();
}

int32_t CSCall::CanCombineConference()
{
    return CanCombineCsConference();
}

int32_t CSCall::SubCallCombineToConference()
{
    return SubCallCombineToCsConference(GetCallID());
}

int32_t CSCall::SubCallSeparateFromConference()
{
    return SubCallSeparateFromCsConference(GetCallID());
}

int32_t CSCall::CanSeparateConference()
{
    return CanSeparateCsConference();
}

int32_t CSCall::GetMainCallId()
{
    return GetMainCsCallId(GetCallID());
}

std::vector<std::u16string> CSCall::GetSubCallIdList()
{
    return GetSubCsCallIdList(GetCallID());
}

std::vector<std::u16string> CSCall::GetCallIdListForConference()
{
    return GetCsCallIdListForConference(GetCallID());
}
} // namespace Telephony
} // namespace OHOS
