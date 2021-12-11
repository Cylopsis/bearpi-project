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

#ifndef CALL_STATE_PROCESS_H
#define CALL_STATE_PROCESS_H

#include "audio_state.h"

namespace OHOS {
namespace Telephony {
enum InCallState {
    INACTIVE_STATE = 0,
    RINGING_STATE,
    CS_CALL_STATE,
    IMS_CALL_STATE,
    HOLDING_STATE,
    UNKNOWN_STATE
};

enum AudioCallState {
    IN_IDLE = 0,
    IN_CALL,
    IN_VOIP,
    RINGTONE,
};

enum AudioMode {
    MODE_NORMAL = 0,
    MODE_IN_CALL,
    MODE_IN_COMMUNICATION,
    MODE_RINGTONE,
};

/**
 * @class CallStateProcess
 * describes the available call states of a call.
 */
class CallStateProcess {
public:
    CallStateProcess();
    ~CallStateProcess();
    void Init();
    bool ProcessEvent(int32_t event);
    enum CallStateEventType {
        SWITCH_CS_CALL_STATE = 100,
        SWITCH_IMS_CALL_STATE,
        SWITCH_OTT_CALL_STATE,
        SWITCH_RINGING_STATE,
        SWITCH_HOLDING_STATE,
        SWITCH_AUDIO_INACTIVE_STATE,
        NEW_ACTIVE_CS_CALL,
        NEW_ACTIVE_IMS_CALL,
        NEW_ACTIVE_OTT_CALL,
        NEW_INCOMING_CALL,
        NO_MORE_ACTIVE_CALL,
        NO_MORE_INCOMING_CALL,
        VIDEO_STATE_CHANGED
    };

private:
    bool SwitchState(InCallState state);
    bool DoSwitch(InCallState state);
    bool SwitchCSCall();
    bool SwitchIMSCall();
    bool SwitchHolding();
    bool SwitchInactive();
    bool SwitchOTTCall();
    bool SwitchRinging();
    void SetCurrentCallState(AudioCallState state); // audio call state
    AudioCallState GetCurrentCallState() const; // audio call state
    bool ActivateAudio();
    bool DeactivateAudio();

    std::unique_ptr<AudioState> audioState_;
    AudioCallState currentCallState_; // audio call state
    InCallState currentInCallState_; // in call state
    std::mutex mutex_;
};
} // namespace Telephony
} // namespace OHOS
#endif // CALL_STATE_PROCESS_H
