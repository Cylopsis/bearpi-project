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

#include "audio_control_manager.h"

#include "telephony_log_wrapper.h"

#include "call_control_manager.h"

namespace OHOS {
namespace Telephony {
AudioControlManager::AudioControlManager()
    : isTonePlaying_(false), ringingCallNumber_(""), ring_(nullptr), tone_(nullptr)
{}

AudioControlManager::~AudioControlManager()
{
    activeCalls_.clear();
    alertingCalls_.clear();
    incomingCalls_.clear();
    holdingCalls_.clear();
}

void AudioControlManager::Init()
{
    TELEPHONY_LOGD("audio control manager init");
    callStateManager_ = std::make_unique<CallStateProcess>();
    if (callStateManager_ == nullptr) {
        TELEPHONY_LOGE("call state manager nullptr");
        return;
    }
    audioDeviceManager_ = std::make_unique<AudioDeviceManager>();
    if (audioDeviceManager_ == nullptr) {
        TELEPHONY_LOGE("audio device manager nullptr");
        return;
    }
    callStateManager_->Init();
    audioDeviceManager_->Init();
    DelayedSingleton<AudioProxy>::GetInstance()->Init();
}

void AudioControlManager::NewCallCreated(sptr<CallBase> &callObjectPtr)
{
    if (callObjectPtr == nullptr || totalCalls_.count(callObjectPtr) > 0) {
        TELEPHONY_LOGE("call object ptr nullptr or call already added");
        return;
    }
    totalCalls_.insert(callObjectPtr);
    AddCall(callObjectPtr, callObjectPtr->GetTelCallState());
}

void AudioControlManager::CallDestroyed(sptr<CallBase> &callObjectPtr)
{
    if (callObjectPtr == nullptr || totalCalls_.count(callObjectPtr) == 0) {
        TELEPHONY_LOGE("call object ptr nullptr or call not included");
        return;
    }
    totalCalls_.erase(callObjectPtr);
    DeleteCall(callObjectPtr, callObjectPtr->GetTelCallState());
}

void AudioControlManager::CallStateUpdated(
    sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState)
{
    if (callObjectPtr == nullptr || totalCalls_.count(callObjectPtr) == 0) {
        TELEPHONY_LOGE("call object ptr nullptr or call not included");
        return;
    }
    DeleteCall(callObjectPtr, priorState);
    AddCall(callObjectPtr, nextState);
}

void AudioControlManager::IncomingCallActivated(sptr<CallBase> &callObjectPtr)
{
    if (callObjectPtr == nullptr || totalCalls_.count(callObjectPtr) == 0) {
        TELEPHONY_LOGE("call object ptr nullptr or call not included");
        return;
    }
    DeleteCall(callObjectPtr, TelCallState::CALL_STATUS_INCOMING);
    AddCall(callObjectPtr, TelCallState::CALL_STATUS_ACTIVE);
    SetMute(false); // unmute microphone
}

void AudioControlManager::IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content)
{
    if (callObjectPtr == nullptr || totalCalls_.count(callObjectPtr) == 0) {
        TELEPHONY_LOGE("call object ptr nullptr or call not included");
        return;
    }
    DeleteCall(callObjectPtr, TelCallState::CALL_STATUS_INCOMING);
}

void AudioControlManager::HandleCallStateChanged(TelCallState stateType, bool isAdded, CallType callType)
{
    switch (stateType) {
        case TelCallState::CALL_STATUS_ALERTING:
            if (alertingCalls_.empty()) {
                StopRingback(); // should stop ringback tone while no more alerting calls
            } else if (isAdded && alertingCalls_.size() == existOnlyOneCall_) {
                PlayRingback(); // should play ringback tone while there exists only one alerting call
            }
            break;
        case TelCallState::CALL_STATUS_ACTIVE:
            if (activeCalls_.empty()) {
                ProcessEvent(CallStateProcess::NO_MORE_ACTIVE_CALL);
            } else if (isAdded) {
                switch (callType) {
                    case CallType::TYPE_CS:
                        ProcessEvent(CallStateProcess::NEW_ACTIVE_CS_CALL);
                        break;
                    case CallType::TYPE_IMS:
                        ProcessEvent(CallStateProcess::NEW_ACTIVE_CS_CALL);
                        break;
                    default:
                        break;
                }
            }
            break;
        case TelCallState::CALL_STATUS_INCOMING:
            if (incomingCalls_.empty()) {
                StopRingtone(); // should stop ringtone while no more incoming calls
                ProcessEvent(CallStateProcess::NO_MORE_INCOMING_CALL);
            } else if (isAdded) {
                ProcessEvent(CallStateProcess::NEW_INCOMING_CALL);
            }
            break;
        default:
            break;
    }
}

void AudioControlManager::AddCall(sptr<CallBase> &callObjectPtr, TelCallState stateType)
{
    if (callObjectPtr == nullptr || callObjectPtr->GetAccountNumber().empty()) {
        return;
    }
    bool isCallAdded = true;
    switch (stateType) {
        case TelCallState::CALL_STATUS_ACTIVE:
            activeCalls_.insert(callObjectPtr->GetAccountNumber());
            break;
        case TelCallState::CALL_STATUS_ALERTING:
            alertingCalls_.insert(callObjectPtr->GetAccountNumber());
            break;
        case TelCallState::CALL_STATUS_INCOMING:
            incomingCalls_.insert(callObjectPtr->GetAccountNumber());
            break;
        case TelCallState::CALL_STATUS_HOLDING:
            holdingCalls_.insert(callObjectPtr->GetAccountNumber());
            break;
        default:
            isCallAdded = false;
            break;
    }
    if (isCallAdded) {
        HandleCallStateChanged(stateType, true, callObjectPtr->GetCallType());
    }
}

void AudioControlManager::DeleteCall(sptr<CallBase> &callObjectPtr, TelCallState stateType)
{
    if (callObjectPtr == nullptr || callObjectPtr->GetAccountNumber().empty()) {
        return;
    }
    bool isCallDeleted = true;
    switch (stateType) {
        case TelCallState::CALL_STATUS_ACTIVE:
            activeCalls_.erase(callObjectPtr->GetAccountNumber());
            break;
        case TelCallState::CALL_STATUS_ALERTING:
            alertingCalls_.erase(callObjectPtr->GetAccountNumber());
            break;
        case TelCallState::CALL_STATUS_INCOMING:
            incomingCalls_.erase(callObjectPtr->GetAccountNumber());
            break;
        case TelCallState::CALL_STATUS_HOLDING:
            holdingCalls_.erase(callObjectPtr->GetAccountNumber());
            break;
        default:
            isCallDeleted = false;
            break;
    }
    if (isCallDeleted) {
        HandleCallStateChanged(stateType, false, callObjectPtr->GetCallType());
    }
}

bool AudioControlManager::SetAudioDevice(int32_t device)
{
    AudioDevice audioDevice = AudioDevice::DEVICE_UNKNOWN;
    switch (device) {
        case (int32_t)AudioDevice::DEVICE_BLUETOOTH_SCO:
            audioDevice = AudioDevice::DEVICE_BLUETOOTH_SCO;
            break;
        case (int32_t)AudioDevice::DEVICE_WIRED_HEADSET:
            audioDevice = AudioDevice::DEVICE_WIRED_HEADSET;
            break;
        case (int32_t)AudioDevice::DEVICE_SPEAKER:
            audioDevice = AudioDevice::DEVICE_SPEAKER;
            break;
        case (int32_t)AudioDevice::DEVICE_MIC:
            audioDevice = AudioDevice::DEVICE_MIC;
            break;
        default:
            break;
    }
    return SetAudioDevice(audioDevice);
}

/**
 * @param device , audio device
 * usually called by the ui interaction , in purpose of switching to another audio device
 */
bool AudioControlManager::SetAudioDevice(AudioDevice device)
{
    if (audioDeviceManager_ == nullptr) {
        return false;
    }
    return audioDeviceManager_->SwitchDevice(device);
}

bool AudioControlManager::PlayRingtone()
{
    if (ring_ != nullptr) {
        ring_ = nullptr;
    }
    ring_ = std::make_unique<Ring>();
    if (ring_ == nullptr) {
        TELEPHONY_LOGE("create ring object failed");
        return false;
    }
    if (ring_->Start() != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("play ringtone failed");
        return false;
    }
    if (ExistOnlyOneIncomingCall()) {
        ringingCallNumber_ = *incomingCalls_.begin();
    }
    TELEPHONY_LOGD("play ringtone succeed");
    ringState_ = RingState::RINGING;
    return true;
}

// play default ring
bool AudioControlManager::PlayRingtone(const std::string &phoneNum)
{
    if (phoneNum.empty() || IsBlackNumber(phoneNum)) {
        return false;
    }
    if (ring_ != nullptr) {
        ring_ = nullptr;
    }
    ring_ = std::make_unique<Ring>();
    if (ring_ == nullptr) {
        TELEPHONY_LOGE("create ring object failed");
        return false;
    }
    if (ring_->Start() != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("play ringtone failed");
        return false;
    }
    if (ExistOnlyOneIncomingCall()) {
        ringingCallNumber_ = *incomingCalls_.begin();
    }
    TELEPHONY_LOGD("play ringtone succeed");
    ringState_ = RingState::RINGING;
    return true;
}

// play specific ring
bool AudioControlManager::PlayRingtone(const std::string &phoneNum, const std::string &ringtonePath)
{
    if (phoneNum.empty() || IsBlackNumber(phoneNum)) {
        return false;
    }
    if (ring_ != nullptr) {
        ring_ = nullptr;
    }
    ring_ = std::make_unique<Ring>(ringtonePath);
    if (ring_ == nullptr) {
        TELEPHONY_LOGE("create ring object failed");
        return false;
    }
    if (ring_->Start() == TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("play ringtone failed");
        return false;
    }
    if (ExistOnlyOneIncomingCall()) {
        ringingCallNumber_ = *incomingCalls_.begin();
    }
    TELEPHONY_LOGD("play ringtone succeed");
    ringState_ = RingState::RINGING;
    return true;
}

bool AudioControlManager::StopRingtone()
{
    if (ringState_ == RingState::STOPPED) {
        return true;
    }
    if (ring_ != nullptr && ring_->Stop() == TELEPHONY_SUCCESS) {
        ringState_ = RingState::STOPPED;
        ringingCallNumber_ = "";
        return true;
    }
    TELEPHONY_LOGE("stop ringtone failed");
    return false;
}

bool AudioControlManager::ProcessEvent(int32_t event)
{
    if (callStateManager_ == nullptr || audioDeviceManager_ == nullptr) {
        return false;
    }
    bool result = false;
    switch (event) {
        case CallStateProcess::SWITCH_CS_CALL_STATE:
        case CallStateProcess::SWITCH_IMS_CALL_STATE:
        case CallStateProcess::SWITCH_RINGING_STATE:
        case CallStateProcess::SWITCH_HOLDING_STATE:
        case CallStateProcess::SWITCH_AUDIO_INACTIVE_STATE:
        case CallStateProcess::NEW_ACTIVE_CS_CALL:
        case CallStateProcess::NEW_ACTIVE_IMS_CALL:
        case CallStateProcess::NEW_INCOMING_CALL:
        case CallStateProcess::NO_MORE_ACTIVE_CALL:
        case CallStateProcess::NO_MORE_INCOMING_CALL:
            result = callStateManager_->ProcessEvent(event);
            break;
        case AudioDeviceManager::ENABLE_DEVICE_BLUETOOTH:
        case AudioDeviceManager::ENABLE_DEVICE_WIRED_HEADSET:
        case AudioDeviceManager::ENABLE_DEVICE_SPEAKER:
        case AudioDeviceManager::ENABLE_DEVICE_MIC:
        case AudioDeviceManager::DEVICES_INACTIVE:
        case AudioDeviceManager::WIRED_HEADSET_AVAILABLE:
        case AudioDeviceManager::WIRED_HEADSET_UNAVAILABLE:
        case AudioDeviceManager::BLUETOOTH_SCO_AVAILABLE:
        case AudioDeviceManager::BLUETOOTH_SCO_UNAVAILABLE:
        case AudioDeviceManager::AUDIO_INTERRUPTED:
        case AudioDeviceManager::AUDIO_UN_INTERRUPT:
        case AudioDeviceManager::AUDIO_RINGING:
        case AudioDeviceManager::INIT_AUDIO_DEVICE:
            result = audioDeviceManager_->ProcessEvent(event);
            break;
        default:
            break;
    }
    return result;
}

void AudioControlManager::SetAudioInterruptState(AudioInterruptState state)
{
    audioInterruptState_ = state;
    /**
     * send audio interrupt event to audio device manager , maybe need to reinitialize the audio device
     */
    switch (audioInterruptState_) {
        case AudioInterruptState::UN_INTERRUPT:
            ProcessEvent(AudioDeviceManager::AUDIO_UN_INTERRUPT);
            break;
        case AudioInterruptState::IN_RINGING:
            ProcessEvent(AudioDeviceManager::AUDIO_RINGING);
            break;
        case AudioInterruptState::IN_INTERRUPT:
            ProcessEvent(AudioDeviceManager::AUDIO_INTERRUPTED);
            break;
        default:
            break;
    }
}

/**
 * while audio state changed , maybe need to reinitialize the audio device
 * in order to get the initialization status of audio device , need to consider varieties of  audio conditions
 */
AudioDevice AudioControlManager::GetInitAudioDevice() const
{
    if (audioInterruptState_ == AudioInterruptState::UN_INTERRUPT) {
        // disable device
        return AudioDevice::DEVICE_DISABLE;
    } else {
        // interrupted or ringing , priority : bt sco , wired headset , speaker , mic
        if (AudioDeviceManager::IsBtScoAvailable()) {
            return AudioDevice::DEVICE_BLUETOOTH_SCO;
        }
        if (AudioDeviceManager::IsWiredHeadsetAvailable()) {
            return AudioDevice::DEVICE_WIRED_HEADSET;
        }
        if (audioInterruptState_ == AudioInterruptState::IN_RINGING) {
            return AudioDevice::DEVICE_SPEAKER;
        }
        if (GetCurrentCall() != nullptr && GetCurrentCall()->IsSpeakerphoneOn()) {
            return AudioDevice::DEVICE_SPEAKER;
        }
        return AudioDevice::DEVICE_MIC;
    }
}

bool AudioControlManager::UpdateCallStateWhenNoMoreActiveCall()
{
    if (IsActiveCallExists()) {
        return false;
    }
    if (IsHoldingCallExists()) {
        return ProcessEvent(CallStateProcess::SWITCH_HOLDING_STATE);
    } else if (IsRingingCallExists()) {
        return ProcessEvent(CallStateProcess::SWITCH_RINGING_STATE);
    } else {
        return ProcessEvent(CallStateProcess::SWITCH_AUDIO_INACTIVE_STATE);
    }
}

// mute or unmute microphone
int32_t AudioControlManager::SetMute(bool on)
{
    if (DelayedSingleton<CallControlManager>::GetInstance()->HasEmergency()) {
        on = false;
    }
    if (DelayedSingleton<AudioProxy>::GetInstance()->SetMicrophoneMute(on)) {
        return TELEPHONY_SUCCESS;
    }
    return TELEPHONY_FAIL;
}

int32_t AudioControlManager::MuteRinger(bool isMute)
{
    if (isMute) {
        if (ringState_ == RingState::RINGING && ring_ != nullptr && ring_->Pause() == TELEPHONY_SUCCESS) {
            ringState_ = RingState::PAUSED;
            return TELEPHONY_SUCCESS;
        }
        return TELEPHONY_FAIL;
    } else {
        if (ringState_ == RingState::PAUSED && ring_ != nullptr && ring_->Resume() == TELEPHONY_SUCCESS) {
            ringState_ = RingState::RINGING;
            return TELEPHONY_SUCCESS;
        }
        return TELEPHONY_FAIL;
    }
}

void AudioControlManager::PlayCallEndedTone(TelCallState priorState, TelCallState nextState, CallEndedType type)
{
    if (nextState != TelCallState::CALL_STATUS_DISCONNECTED) {
        return;
    }
    if (priorState == TelCallState::CALL_STATUS_ACTIVE || priorState == TelCallState::CALL_STATUS_DIALING ||
        priorState == TelCallState::CALL_STATUS_HOLDING) {
        switch (type) {
            case CallEndedType::PHONE_IS_BUSY:
                PlayCallTone(ToneDescriptor::TONE_ENGAGED);
                break;
            case CallEndedType::CALL_ENDED_NORMALLY:
                PlayCallTone(ToneDescriptor::TONE_FINISHED);
                break;
            case CallEndedType::UNKNOWN:
                PlayCallTone(ToneDescriptor::TONE_UNKNOWN);
                break;
            case CallEndedType::INVALID_NUMBER:
                PlayCallTone(ToneDescriptor::TONE_INVALID_NUMBER);
                break;
            default:
                break;
        }
    }
}

sptr<CallBase> AudioControlManager::GetCallBase(const std::string &phoneNum) const
{
    sptr<CallBase> retPtr;
    if (phoneNum.empty()) {
        return retPtr;
    }
    for (auto &call : totalCalls_) {
        if (call->GetAccountNumber() == phoneNum) {
            retPtr = call;
            break;
        }
    }
    return retPtr;
}

std::string AudioControlManager::GetIncomingCallRingtonePath()
{
    if (incomingCalls_.size() == existOnlyOneCall_) {
        ContactInfo info;
        sptr<CallBase> incomingCall = GetCallBase(*incomingCalls_.begin());
        if (incomingCall != nullptr) {
            incomingCall->GetCallerInfo(info);
            std::string path(info.ringtonePath);
            return path;
        }
    }
    return "";
}

sptr<CallBase> AudioControlManager::GetCurrentCall() const
{
    if (ExistOnlyOneActiveCall()) {
        return GetCallBase(*activeCalls_.begin());
    }
    return nullptr;
}

sptr<CallBase> AudioControlManager::GetRingingCall() const
{
    if (ringingCallNumber_.empty()) {
        return nullptr;
    }
    return GetCallBase(ringingCallNumber_);
}

std::set<sptr<CallBase>> AudioControlManager::GetCallList()
{
    return totalCalls_;
}

AudioInterruptState AudioControlManager::GetAudioInterruptState()
{
    return audioInterruptState_;
}

void AudioControlManager::SetVolumeAudible()
{
    DelayedSingleton<AudioProxy>::GetInstance()->SetVolumeAudible();
}

bool AudioControlManager::IsBlackNumber(const std::string &phoneNum)
{
    // check whether the phone number is black or not , black number should not ring
    return false;
}

bool AudioControlManager::IsCurrentVideoCall() const
{
    return false;
}

bool AudioControlManager::IsActiveCallExists() const
{
    return !activeCalls_.empty();
}

bool AudioControlManager::IsHoldingCallExists() const
{
    return !holdingCalls_.empty();
}

bool AudioControlManager::IsRingingCallExists() const
{
    return !incomingCalls_.empty();
}

bool AudioControlManager::IsEmergencyCallExists() const
{
    for (auto call : totalCalls_) {
        if (call->GetEmergencyState()) {
            return true;
        }
    }
    return false;
}

bool AudioControlManager::IsTonePlaying() const
{
    return isTonePlaying_;
}

bool AudioControlManager::IsCurrentRinging() const
{
    return ringState_ == RingState::RINGING;
}

bool AudioControlManager::IsAudioActive() const
{
    return audioInterruptState_ != AudioInterruptState::UN_INTERRUPT;
}

bool AudioControlManager::ExistOnlyOneActiveCall() const
{
    return activeCalls_.size() == existOnlyOneCall_;
}

bool AudioControlManager::ExistOnlyOneIncomingCall() const
{
    return incomingCalls_.size() == existOnlyOneCall_;
}

int32_t AudioControlManager::PlayCallTone(ToneDescriptor type)
{
    if (tone_ != nullptr) {
        tone_ = nullptr;
    }
    tone_ = std::make_unique<Tone>(type);
    if (tone_ == nullptr) {
        TELEPHONY_LOGE("create tone object failed");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
    if (tone_->Start() == TELEPHONY_SUCCESS) {
        isTonePlaying_ = true;
        return TELEPHONY_SUCCESS;
    }
    return TELEPHONY_FAIL;
}

int32_t AudioControlManager::StopCallTone()
{
    if (tone_ != nullptr && tone_->Stop() == TELEPHONY_SUCCESS) {
        isTonePlaying_ = false;
        return TELEPHONY_SUCCESS;
    }
    return TELEPHONY_FAIL;
}

void AudioControlManager::TurnOffVoice()
{
    if (ring_ != nullptr) {
        ring_->Stop();
    }
    if (tone_ != nullptr) {
        tone_->Stop();
    }
}

int32_t AudioControlManager::PlayDtmf(char digit)
{
    return PlayCallTone(Tone::ConvertDigitToTone(digit));
}

int32_t AudioControlManager::PlayRingback()
{
    return PlayCallTone(ToneDescriptor::TONE_RING_BACK);
}

int32_t AudioControlManager::StopRingback()
{
    return StopCallTone();
}

int32_t AudioControlManager::PlayWaitingTone()
{
    return PlayCallTone(ToneDescriptor::TONE_WAITING);
}

int32_t AudioControlManager::StopWaitingTone()
{
    return StopCallTone();
}

int32_t AudioControlManager::PlayHoldingTone()
{
    return PlayCallTone(ToneDescriptor::TONE_WAITING);
}

int32_t AudioControlManager::StopHoldingTone()
{
    return StopCallTone();
}
} // namespace Telephony
} // namespace OHOS