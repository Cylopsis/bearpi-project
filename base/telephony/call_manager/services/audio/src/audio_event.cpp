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

#include "audio_event.h"

#include "telephony_log_wrapper.h"

#include "audio_control_manager.h"

namespace OHOS {
namespace Telephony {
AudioEvent::AudioEvent() : callStateManager_(nullptr), audioDeviceManager_(nullptr) {}

AudioEvent::~AudioEvent() {}

void AudioEvent::Init()
{
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
}

bool AudioEvent::HandleEvent(int32_t event)
{
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
            result = HandleCallStateEvent(event);
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
            result = HandleAudioDeviceEvent(event);
            break;
        default:
            TELEPHONY_LOGE("invalid audio event");
            break;
    }
    return result;
}

bool AudioEvent::HandleCallStateEvent(int32_t event)
{
    if (callStateManager_ == nullptr) {
        TELEPHONY_LOGE("call state manager nullptr");
        return false;
    }
    return callStateManager_->ProcessEvent(event);
}

bool AudioEvent::HandleAudioDeviceEvent(int32_t event)
{
    if (audioDeviceManager_ == nullptr) {
        TELEPHONY_LOGE("audio device manager nullptr");
        return false;
    }
    return audioDeviceManager_->ProcessEvent(event);
}

bool AudioEvent::SetAudioDevice(AudioDevice device)
{
    if (audioDeviceManager_ == nullptr) {
        TELEPHONY_LOGE("audio device manager nullptr");
        return false;
    }
    return audioDeviceManager_->SwitchDevice(device);
}
} // namespace Telephony
} // namespace OHOS