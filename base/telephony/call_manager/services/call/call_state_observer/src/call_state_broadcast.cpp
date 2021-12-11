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

#include "call_state_broadcast.h"

#include "common_event_manager.h"
#include "want.h"

#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
void CallStateBroadcast::NewCallCreated(sptr<CallBase> &callObjectPtr)
{
    if (callObjectPtr == nullptr) {
        TELEPHONY_LOGE("call object ptr nullptr");
        return;
    }
    PublishCommonEvent(callObjectPtr, ++eventCode_);
}

void CallStateBroadcast::CallDestroyed(sptr<CallBase> &callObjectPtr)
{
    if (callObjectPtr == nullptr) {
        TELEPHONY_LOGE("call object ptr nullptr");
        return;
    }
    PublishCommonEvent(callObjectPtr, ++eventCode_);
}

void CallStateBroadcast::CallStateUpdated(
    sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState)
{
    if (callObjectPtr == nullptr) {
        TELEPHONY_LOGE("call object ptr nullptr");
        return;
    }
    PublishCommonEvent(callObjectPtr, ++eventCode_);
}

void CallStateBroadcast::PublishCommonEvent(sptr<CallBase> &callObjectPtr, int32_t eventCode)
{
    AAFwk::Want want;
    want.SetParam("callId", callObjectPtr->GetCallID());
    want.SetParam("callState", (int32_t)callObjectPtr->GetTelCallState());
    want.SetParam("phoneNumber", callObjectPtr->GetAccountNumber());
    want.SetAction(CALL_STATE_UPDATE);
    EventFwk::CommonEventData data;
    data.SetWant(want);
    data.SetCode(eventCode);
    data.SetData(EVENT_DATA);
    EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    bool result = EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo, nullptr);
    if (result) {
        TELEPHONY_LOGD("call state update published successfully");
    } else {
        TELEPHONY_LOGE("call state update published failed");
    }
}

void CallStateBroadcast::IncomingCallActivated(sptr<CallBase> &callObjectPtr) {}

void CallStateBroadcast::IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content) {}
} // namespace Telephony
} // namespace OHOS
