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

#include "call_recording_tone.h"

#include "want.h"

#include "audio_control_manager.h"

namespace OHOS {
namespace Telephony {
CallRecordingTone::CallRecordingTone(const EventFwk::CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo), isRecording_(false)
{}

void CallRecordingTone::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    EventFwk::Want want = data.GetWant();
    if (want.GetAction() == CALL_RECORDING_ACTION) {
        want.GetBoolParam("isRecording", false) ? PlayRecordingTone() : StopRecordingTone();
    }
}

void CallRecordingTone::CallDestroyed(sptr<CallBase> &callObjectPtr)
{
    if (isRecording_ && !DelayedSingleton<AudioControlManager>::GetInstance()->IsActiveCallExists()) {
        StopRecordingTone();
    }
}

void CallRecordingTone::NewCallCreated(sptr<CallBase> &callObjectPtr) {}

void CallRecordingTone::IncomingCallActivated(sptr<CallBase> &callObjectPtr) {}

void CallRecordingTone::IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content) {}

void CallRecordingTone::CallStateUpdated(
    sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState)
{}

int32_t CallRecordingTone::PlayRecordingTone()
{
    // the recording call's call state should be active
    if (DelayedSingleton<AudioControlManager>::GetInstance()->GetCurrentCall() == nullptr) {
        return TELEPHONY_FAIL;
    }
    if (DelayedSingleton<AudioControlManager>::GetInstance()->PlayCallTone(ToneDescriptor::TONE_CALL_RECORDING) ==
        TELEPHONY_SUCCESS) {
        isRecording_ = true;
        return TELEPHONY_SUCCESS;
    }
    return TELEPHONY_FAIL;
}

int32_t CallRecordingTone::StopRecordingTone()
{
    if (!isRecording_) {
        return TELEPHONY_SUCCESS;
    }
    if (DelayedSingleton<AudioControlManager>::GetInstance()->StopCallTone() == TELEPHONY_SUCCESS) {
        isRecording_ = false;
        return TELEPHONY_SUCCESS;
    }
    return TELEPHONY_FAIL;
}
} // namespace Telephony
} // namespace OHOS