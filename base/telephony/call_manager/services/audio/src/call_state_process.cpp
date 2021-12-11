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

#include "call_state_process.h"

#include "telephony_log_wrapper.h"

#include "audio_control_manager.h"
#include "cs_call_state.h"
#include "holding_state.h"
#include "ims_call_state.h"
#include "inactive_state.h"
#include "ringing_state.h"

namespace OHOS {
namespace Telephony {
CallStateProcess::CallStateProcess()
    : audioState_(nullptr), currentCallState_(IN_IDLE), currentInCallState_(UNKNOWN_STATE)
{}

CallStateProcess::~CallStateProcess() {}

void CallStateProcess::Init()
{
    TELEPHONY_LOGI("call state manager init");
    audioState_ = std::make_unique<InActiveState>();
    if (audioState_ == nullptr) {
        TELEPHONY_LOGI("audio state nullptr");
    }
}

bool CallStateProcess::ProcessEvent(int32_t event)
{
    if (audioState_ == nullptr) {
        TELEPHONY_LOGE("audio state nullptr");
        return false;
    }
    bool result = false;
    switch (event) {
        case CallStateProcess::SWITCH_CS_CALL_STATE:
            result = SwitchState(InCallState::CS_CALL_STATE);
            break;
        case CallStateProcess::SWITCH_IMS_CALL_STATE:
            result = SwitchState(InCallState::IMS_CALL_STATE);
            break;
        case CallStateProcess::SWITCH_RINGING_STATE:
            result = SwitchState(InCallState::RINGING_STATE);
            break;
        case CallStateProcess::SWITCH_HOLDING_STATE:
            result = SwitchState(InCallState::HOLDING_STATE);
            break;
        case CallStateProcess::SWITCH_AUDIO_INACTIVE_STATE:
            result = SwitchState(InCallState::INACTIVE_STATE);
            break;
        case CallStateProcess::NEW_ACTIVE_CS_CALL:
        case CallStateProcess::NEW_ACTIVE_IMS_CALL:
        case CallStateProcess::NEW_INCOMING_CALL:
        case CallStateProcess::NO_MORE_ACTIVE_CALL:
        case CallStateProcess::NO_MORE_INCOMING_CALL:
            result = audioState_->ProcessEvent(event);
            break;
        default:
            break;
    }
    return result;
}

bool CallStateProcess::SwitchState(InCallState state)
{
    if (currentInCallState_ == state) {
        return true;
    }
    bool result = false;
    std::lock_guard<std::mutex> lock(mutex_);
    switch (state) {
        case InCallState::INACTIVE_STATE:
            result = SwitchInactive();
            break;
        case InCallState::RINGING_STATE:
            result = SwitchRinging();
            break;
        case InCallState::CS_CALL_STATE:
            result = SwitchCSCall();
            break;
        case InCallState::IMS_CALL_STATE:
            result = SwitchIMSCall();
            break;
        case InCallState::HOLDING_STATE:
            result = SwitchHolding();
            break;
        default:
            TELEPHONY_LOGD("invalid state");
            break;
    }
    TELEPHONY_LOGI("switch call state lock release");
    return result;
}

bool CallStateProcess::SwitchCSCall()
{
    if (!ActivateAudio()) {
        return false;
    }
    if (DelayedSingleton<AudioProxy>::GetInstance()->UpdateCallState(AudioCallState::IN_CALL)) {
        SetCurrentCallState(AudioCallState::IN_CALL);
        DelayedSingleton<AudioControlManager>::GetInstance()->SetAudioInterruptState(
            AudioInterruptState::IN_INTERRUPT);
        audioState_ = std::make_unique<CSCallState>();
        if (audioState_ == nullptr) {
            return false;
        }
        currentInCallState_ = CS_CALL_STATE;
        return true;
    }
    return false;
}

bool CallStateProcess::SwitchIMSCall()
{
    if (!ActivateAudio()) {
        return false;
    }
    if (DelayedSingleton<AudioProxy>::GetInstance()->UpdateCallState(AudioCallState::IN_VOIP)) {
        SetCurrentCallState(AudioCallState::IN_VOIP);
        DelayedSingleton<AudioControlManager>::GetInstance()->SetAudioInterruptState(
            AudioInterruptState::IN_INTERRUPT);
        audioState_ = std::make_unique<IMSCallState>();
        if (audioState_ == nullptr) {
            return false;
        }
        currentInCallState_ = IMS_CALL_STATE;
        return true;
    }
    return false;
}

bool CallStateProcess::SwitchHolding()
{
    if (!ActivateAudio()) {
        return false;
    }
    if (DelayedSingleton<AudioProxy>::GetInstance()->UpdateCallState(currentCallState_)) {
        DelayedSingleton<AudioControlManager>::GetInstance()->SetAudioInterruptState(
            AudioInterruptState::IN_INTERRUPT);
        audioState_ = std::make_unique<HoldingState>();
        if (audioState_ == nullptr) {
            return false;
        }
        currentInCallState_ = HOLDING_STATE;
        return true;
    }
    return false;
}

bool CallStateProcess::SwitchInactive()
{
    if (!DeactivateAudio()) {
        return false;
    }
    if (DelayedSingleton<AudioProxy>::GetInstance()->UpdateCallState(AudioCallState::IN_IDLE)) {
        SetCurrentCallState(AudioCallState::IN_IDLE);
        DelayedSingleton<AudioControlManager>::GetInstance()->SetAudioInterruptState(
            AudioInterruptState::UN_INTERRUPT);
        audioState_ = std::make_unique<InActiveState>();
        if (audioState_ == nullptr) {
            return false;
        }
        currentInCallState_ = INACTIVE_STATE;
        TELEPHONY_LOGD("current call state : inactive state");
        return true;
    }
    return false;
}

bool CallStateProcess::SwitchOTTCall()
{
    return true;
}

bool CallStateProcess::SwitchRinging()
{
    if (!ActivateAudio()) {
        return false;
    }
    if (DelayedSingleton<AudioProxy>::GetInstance()->UpdateCallState(AudioCallState::RINGTONE)) {
        SetCurrentCallState(AudioCallState::RINGTONE);
        DelayedSingleton<AudioControlManager>::GetInstance()->SetAudioInterruptState(
            AudioInterruptState::IN_RINGING);
        audioState_ = std::make_unique<RingingState>();
        if (audioState_ == nullptr) {
            return false;
        }
        currentInCallState_ = RINGING_STATE;
        TELEPHONY_LOGD("current call state : ringing state");
        return true;
    }
    return false;
}

void CallStateProcess::SetCurrentCallState(AudioCallState state)
{
    currentCallState_ = state;
}

AudioCallState CallStateProcess::GetCurrentCallState() const
{
    return currentCallState_;
}

bool CallStateProcess::ActivateAudio()
{
    if (DelayedSingleton<AudioControlManager>::GetInstance()->IsAudioActive()) {
        return true;
    }
    if (DelayedSingleton<AudioProxy>::GetInstance()->ActivateAudioInterrupt()) {
        return true;
    }
    return false;
}

bool CallStateProcess::DeactivateAudio()
{
    if (!DelayedSingleton<AudioControlManager>::GetInstance()->IsAudioActive()) {
        return true;
    }
    if (DelayedSingleton<AudioProxy>::GetInstance()->DeactivateAudioInterrupt()) {
        return true;
    }
    return false;
}

bool CallStateProcess::DoSwitch(InCallState state)
{
    bool result = false;
    switch (state) {
        case InCallState::INACTIVE_STATE:
            result = SwitchInactive();
            break;
        case InCallState::RINGING_STATE:
            result = SwitchRinging();
            break;
        case InCallState::CS_CALL_STATE:
            result = SwitchCSCall();
            break;
        case InCallState::IMS_CALL_STATE:
            result = SwitchIMSCall();
            break;
        case InCallState::HOLDING_STATE:
            result = SwitchHolding();
            break;
        default:
            break;
    }
    return result;
}
} // namespace Telephony
} // namespace OHOS