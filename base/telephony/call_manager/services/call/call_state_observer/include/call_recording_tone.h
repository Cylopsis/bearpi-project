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

#ifndef CALL_RECORDING_TONE_H
#define CALL_RECORDING_TONE_H

#include <cstdint>
#include <map>

#include "common_event.h"
#include "common_event_manager.h"

#include "call_state_listener_base.h"

namespace OHOS {
namespace Telephony {
class CallRecordingTone : public EventFwk::CommonEventSubscriber, public CallStateListenerBase {
public:
    explicit CallRecordingTone(const EventFwk::CommonEventSubscribeInfo &subscriberInfo);
    ~CallRecordingTone() = default;
    void OnReceiveEvent(const EventFwk::CommonEventData &data) override;
    void NewCallCreated(sptr<CallBase> &callObjectPtr) override;
    void CallDestroyed(sptr<CallBase> &callObjectPtr) override;
    void IncomingCallActivated(sptr<CallBase> &callObjectPtr) override;
    void IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content) override;
    void CallStateUpdated(sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState) override;

private:
    bool isRecording_;
    const std::string CALL_RECORDING_ACTION = "ohos.action.CALL_RECORDING_ACTION";
    int32_t PlayRecordingTone();
    int32_t StopRecordingTone();
};
} // namespace Telephony
} // namespace OHOS
#endif