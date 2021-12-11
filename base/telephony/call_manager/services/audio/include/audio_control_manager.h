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

#ifndef TELEPHONY_AUDIO_MANAGER_H
#define TELEPHONY_AUDIO_MANAGER_H

#include <mutex>
#include <set>

#include "singleton.h"

#include "call_manager_inner_type.h"

#include "audio_proxy.h"
#include "call_state_listener_base.h"
#include "tone.h"
#include "ring.h"
#include "audio_device_manager.h"
#include "call_state_process.h"

namespace OHOS {
namespace Telephony {
enum AudioInterruptState {
    UN_INTERRUPT = 0,
    IN_INTERRUPT,
    IN_RINGING,
};

struct CallRecord {
    std::string phoneNum;
    bool isIms;
};

class AudioControlManager : public CallStateListenerBase, public std::enable_shared_from_this<AudioControlManager> {
    DECLARE_DELAYED_SINGLETON(AudioControlManager)
public:
    void Init();
    bool ProcessEvent(int32_t event);
    bool SetAudioDevice(int32_t device);
    bool SetAudioDevice(AudioDevice device);
    bool PlayRingtone(); // plays the default ringtone
    bool PlayRingtone(const std::string &phoneNum); // plays the default ringtone
    bool PlayRingtone(const std::string &phoneNum, const std::string &ringtonePath); // plays the specific ringtone
    bool StopRingtone();
    int32_t PlayDtmf(char digit);
    int32_t PlayRingback();
    int32_t StopRingback();
    int32_t PlayWaitingTone();
    int32_t StopWaitingTone();
    int32_t PlayHoldingTone();
    int32_t StopHoldingTone();
    int32_t PlayCallEndedTone(ToneDescriptor type);
    int32_t PlayCallTone(ToneDescriptor type);
    int32_t StopCallTone();
    void TurnOffVoice();
    void SetVolumeAudible();
    bool IsCurrentVideoCall() const;
    bool IsTonePlaying() const;
    bool IsCurrentRinging() const;
    bool IsAudioActive() const;
    bool IsActiveCallExists() const;
    bool ExistOnlyOneActiveCall() const;
    bool ExistOnlyOneIncomingCall() const;
    int32_t SetMute(bool on);
    int32_t MuteRinger(bool isMute);
    int32_t GetRingingCallId();
    AudioDevice GetInitAudioDevice() const;
    sptr<CallBase> GetCurrentCall() const;
    sptr<CallBase> GetRingingCall() const;
    std::set<sptr<CallBase>> GetCallList();
    AudioInterruptState GetAudioInterruptState();
    bool UpdateCallStateWhenNoMoreActiveCall();
    void SetAudioInterruptState(AudioInterruptState state);
    void NewCallCreated(sptr<CallBase> &callObjectPtr) override;
    void CallDestroyed(sptr<CallBase> &callObjectPtr) override;
    void IncomingCallActivated(sptr<CallBase> &callObjectPtr) override;
    void IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content) override;
    void CallStateUpdated(sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState) override;

private:
    const uint32_t existOnlyOneCall_ = 1;
    RingState ringState_ = RingState::STOPPED;
    void AddCall(sptr<CallBase> &callObjectPtr, TelCallState stateType);
    void DeleteCall(sptr<CallBase> &callObjectPtr, TelCallState stateType);
    void HandleCallStateChanged(TelCallState stateType, bool isAdded, CallType callType);
    bool IsBlackNumber(const std::string &phoneNum);
    void PlayCallEndedTone(TelCallState priorState, TelCallState nextState, CallEndedType type);
    sptr<CallBase> GetCallBase(const std::string &phoneNum) const;
    std::string GetIncomingCallRingtonePath();
    AudioInterruptState audioInterruptState_ = AudioInterruptState::UN_INTERRUPT;
    bool IsRingingCallExists() const;
    bool IsHoldingCallExists() const;
    bool IsEmergencyCallExists() const;
    bool isTonePlaying_;
    std::string ringingCallNumber_;
    std::set<std::string> alertingCalls_;
    std::set<std::string> activeCalls_;
    std::set<std::string> incomingCalls_;
    std::set<std::string> holdingCalls_;
    std::set<sptr<CallBase>> totalCalls_;
    std::unique_ptr<Ring> ring_;
    std::unique_ptr<Tone> tone_;
    std::unique_ptr<CallStateProcess> callStateManager_;
    std::unique_ptr<AudioDeviceManager> audioDeviceManager_;
};
} // namespace Telephony
} // namespace OHOS
#endif // TELEPHONY_AUDIO_MANAGER_H
