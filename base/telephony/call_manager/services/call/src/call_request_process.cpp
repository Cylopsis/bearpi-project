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

#include "call_request_process.h"

#include "telephony_log_wrapper.h"

#include "call_manager_errors.h"
#include "cs_call.h"
#include "ims_call.h"
#include "ott_call.h"
#include "common_type.h"
#include "call_control_manager.h"

namespace OHOS {
namespace Telephony {
CallRequestProcess::CallRequestProcess() {}

CallRequestProcess::~CallRequestProcess() {}

void CallRequestProcess::DialRequest()
{
    DelayedSingleton<CallControlManager>::GetInstance()->DialProcess();
}

void CallRequestProcess::AnswerRequest(int32_t callId, int32_t videoState)
{
    int32_t ret = TELEPHONY_FAIL;
    sptr<CallBase> call = GetOneCallObject(callId);
    if (call == nullptr) {
        TELEPHONY_LOGE("the call object is nullptr, callId:%{public}d", callId);
        return;
    }
    ret = call->AnswerCall(videoState);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("AnswerCall failed!");
        return;
    }
    DelayedSingleton<CallControlManager>::GetInstance()->NotifyIncomingCallAnswered(call);
}

void CallRequestProcess::RejectRequest(int32_t callId, bool isSendSms, std::string &content)
{
    int32_t ret = TELEPHONY_FAIL;
    sptr<CallBase> call = GetOneCallObject(callId);
    if (call == nullptr) {
        TELEPHONY_LOGE("the call object is nullptr, callId:%{public}d", callId);
        return;
    }

    ret = call->RejectCall(isSendSms, content);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("RejectCall failed!");
        return;
    }
    DelayedSingleton<CallControlManager>::GetInstance()->NotifyIncomingCallRejected(call, isSendSms, content);
}

void CallRequestProcess::HangUpRequest(int32_t callId)
{
    sptr<CallBase> call = GetOneCallObject(callId);
    if (call == nullptr) {
        TELEPHONY_LOGE("the call object is nullptr, callId:%{public}d", callId);
        return;
    }
    call->HangUpCall();
}

void CallRequestProcess::HoldRequest(int32_t callId)
{
    sptr<CallBase> call = GetOneCallObject(callId);
    if (call == nullptr) {
        TELEPHONY_LOGE("the call object is nullptr, callId:%{public}d", callId);
        return;
    }
    call->HoldCall();
}

void CallRequestProcess::UnHoldRequest(int32_t callId)
{
    sptr<CallBase> call = GetOneCallObject(callId);
    if (call == nullptr) {
        TELEPHONY_LOGE("the call object is nullptr, callId:%{public}d", callId);
        return;
    }
    call->UnHoldCall();
}

void CallRequestProcess::SwitchRequest(int32_t callId)
{
    sptr<CallBase> call = GetOneCallObject(callId);
    if (call == nullptr) {
        TELEPHONY_LOGE("the call object is nullptr, callId:%{public}d", callId);
        return;
    }
    call->SwitchCall();
}

void CallRequestProcess::CombineConferenceRequest(int32_t mainCallId)
{
    sptr<CallBase> call = GetOneCallObject(mainCallId);
    if (call == nullptr) {
        TELEPHONY_LOGE("the call object is nullptr, mainCallId:%{public}d", mainCallId);
        return;
    }
    int32_t ret = call->CombineConference();
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("CombineConference failed");
    }
}

void CallRequestProcess::SeparateConferenceRequest(int32_t callId)
{
    sptr<CallBase> call = GetOneCallObject(callId);
    if (call == nullptr) {
        TELEPHONY_LOGE("the call object is nullptr, callId:%{public}d", callId);
        return;
    }
}
} // namespace Telephony
} // namespace OHOS