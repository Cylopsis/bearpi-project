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

#include "audio_device_manager.h"

#include "telephony_log_wrapper.h"

#include "audio_control_manager.h"
#include "devices_inactive.h"
#include "enable_bluetooth_device.h"
#include "enable_mic_device.h"
#include "enable_speaker_device.h"
#include "enable_wired_headset_device.h"

namespace OHOS {
namespace Telephony {
bool AudioDeviceManager::isMicAvailable_ = true; // default available
bool AudioDeviceManager::isSpeakerAvailable_ = true; // default available
bool AudioDeviceManager::isWiredHeadsetAvailable_ = false;
bool AudioDeviceManager::isBtScoAvailable_ = false;

AudioDeviceManager::AudioDeviceManager()
    : currentAudioDevice_(AudioDevice::DEVICE_UNKNOWN), audioState_(nullptr), isAudioActive_(false)
{}

AudioDeviceManager::~AudioDeviceManager() {}

void AudioDeviceManager::Init()
{
    TELEPHONY_LOGI("audio device manager init");
    audioState_ = std::make_unique<DevicesInactive>();
    if (audioState_ == nullptr) {
        TELEPHONY_LOGI("audio state nullptr");
    }
}

bool AudioDeviceManager::InitAudioDevice()
{
    TELEPHONY_LOGI("init audio device");
    // when audio deactivate interrupt , reinitialize
    // when audio external device available state changed , reinitialize
    auto device = DelayedSingleton<AudioControlManager>::GetInstance()->GetInitAudioDevice();
    return SwitchDevice(device); // audio device init
}

bool AudioDeviceManager::ProcessEvent(int32_t event)
{
    TELEPHONY_LOGI("audio device manager process event: %{public}d", event);
    if (audioState_ == nullptr) {
        TELEPHONY_LOGE("audio state nullptr");
        return false;
    }
    bool result = false;
    switch (event) {
        case AudioDeviceManager::ENABLE_DEVICE_MIC:
            result = SwitchDevice(AudioDevice::DEVICE_MIC);
            break;
        case AudioDeviceManager::ENABLE_DEVICE_SPEAKER:
            result = SwitchDevice(AudioDevice::DEVICE_SPEAKER);
            break;
        case AudioDeviceManager::ENABLE_DEVICE_BLUETOOTH:
            result = SwitchDevice(AudioDevice::DEVICE_BLUETOOTH_SCO);
            break;
        case AudioDeviceManager::ENABLE_DEVICE_WIRED_HEADSET:
            result = SwitchDevice(AudioDevice::DEVICE_WIRED_HEADSET);
            break;
        case AudioDeviceManager::DEVICES_INACTIVE:
            result = SwitchDevice(AudioDevice::DEVICE_DISABLE);
            break;
        case AudioDeviceManager::AUDIO_INTERRUPTED:
        case AudioDeviceManager::AUDIO_RINGING:
            if (!isAudioActive_) {
                isAudioActive_ = true;
                result = audioState_->ProcessEvent(event);
            }
            break;
        case AudioDeviceManager::AUDIO_UN_INTERRUPT:
            if (isAudioActive_) {
                isAudioActive_ = false;
                result = InitAudioDevice();
            }
            break;
        case AudioDeviceManager::WIRED_HEADSET_AVAILABLE:
        case AudioDeviceManager::WIRED_HEADSET_UNAVAILABLE:
        case AudioDeviceManager::BLUETOOTH_SCO_AVAILABLE:
        case AudioDeviceManager::BLUETOOTH_SCO_UNAVAILABLE:
            result = audioState_->ProcessEvent(event);
            break;
        case AudioDeviceManager::INIT_AUDIO_DEVICE:
            result = InitAudioDevice();
            break;
        default:
            break;
    }
    return result;
}

bool AudioDeviceManager::SwitchDevice(AudioDevice device)
{
    if (currentAudioDevice_ == device) {
        TELEPHONY_LOGI("no need to switch device");
        return true;
    }
    bool result = false;
    std::lock_guard<std::mutex> lock(mutex_);
    switch (device) {
        case AudioDevice::DEVICE_MIC:
            result = EnableMic();
            break;
        case AudioDevice::DEVICE_SPEAKER:
            result = EnableSpeaker();
            break;
        case AudioDevice::DEVICE_WIRED_HEADSET:
            result = EnableWiredHeadset();
            break;
        case AudioDevice::DEVICE_BLUETOOTH_SCO:
            result = EnableBtSco();
            break;
        case AudioDevice::DEVICE_DISABLE:
            result = DisableAll();
            break;
        default:
            break;
    }
    TELEPHONY_LOGI("switch device lock release");
    return result;
}

bool AudioDeviceManager::EnableSpeaker()
{
    if (isSpeakerAvailable_ && DelayedSingleton<AudioProxy>::GetInstance()->SetSpeakerDevActive(true)) {
        audioState_ = std::make_unique<EnableSpeakerDevice>();
        if (audioState_ == nullptr) {
            return false;
        }
        TELEPHONY_LOGI("current audio device : speaker");
        SetCurrentAudioDevice(AudioDevice::DEVICE_SPEAKER);
        SetSpeakerDevEnable();
        return true;
    }
    TELEPHONY_LOGI("enable speaker dev failed");
    return false;
}

bool AudioDeviceManager::EnableMic()
{
    if (isMicAvailable_ && DelayedSingleton<AudioProxy>::GetInstance()->SetMicDevActive(true)) {
        audioState_ = std::make_unique<EnableMicDevice>();
        if (audioState_ == nullptr) {
            return false;
        }
        TELEPHONY_LOGI("current audio device : mic");
        SetCurrentAudioDevice(AudioDevice::DEVICE_MIC);
        SetMicDevEnable();
        return true;
    }
    TELEPHONY_LOGI("enable mic dev failed");
    return false;
}

bool AudioDeviceManager::EnableWiredHeadset()
{
    if (isWiredHeadsetAvailable_ && DelayedSingleton<AudioProxy>::GetInstance()->SetWiredHeadsetDevActive(true)) {
        audioState_ = std::make_unique<EnableWiredHeadsetDevice>();
        if (audioState_ == nullptr) {
            return false;
        }
        TELEPHONY_LOGI("current audio device : wired headset");
        SetCurrentAudioDevice(AudioDevice::DEVICE_WIRED_HEADSET);
        SetWiredHeadsetDevEnable();
        return true;
    }
    TELEPHONY_LOGI("enable wired headset dev failed");
    return false;
}

bool AudioDeviceManager::EnableBtSco()
{
    if (isBtScoAvailable_ && DelayedSingleton<AudioProxy>::GetInstance()->SetBluetoothDevActive(true)) {
        audioState_ = std::make_unique<EnableBluetoothDevice>();
        if (audioState_ == nullptr) {
            return false;
        }
        TELEPHONY_LOGI("current audio device : bluetooth sco");
        SetCurrentAudioDevice(AudioDevice::DEVICE_BLUETOOTH_SCO);
        SetBtScoDevEnable();
        return true;
    }
    TELEPHONY_LOGI("enable bluetooth sco dev failed");
    return false;
}

bool AudioDeviceManager::DisableAll()
{
    isBtScoDevEnable_ = false;
    isWiredHeadsetDevEnable_ = false;
    isSpeakerDevEnable_ = false;
    isMicDevEnable_ = false;
    audioState_ = std::make_unique<DevicesInactive>();
    if (audioState_ == nullptr) {
        return false;
    }
    TELEPHONY_LOGI("current audio device : device inactive");
    return true;
}

void AudioDeviceManager::SetCurrentAudioDevice(AudioDevice device)
{
    currentAudioDevice_ = device;
}

AudioDevice AudioDeviceManager::GetCurrentAudioDevice()
{
    return currentAudioDevice_;
}

void AudioDeviceManager::SetMicDevEnable()
{
    isMicDevEnable_ = true;
    isBtScoDevEnable_ = false;
    isWiredHeadsetDevEnable_ = false;
    isSpeakerDevEnable_ = false;
}

void AudioDeviceManager::SetSpeakerDevEnable()
{
    isSpeakerDevEnable_ = true;
    isWiredHeadsetDevEnable_ = false;
    isBtScoDevEnable_ = false;
    isMicDevEnable_ = false;
}

void AudioDeviceManager::SetBtScoDevEnable()
{
    isBtScoDevEnable_ = true;
    isWiredHeadsetDevEnable_ = false;
    isSpeakerDevEnable_ = false;
    isMicDevEnable_ = false;
}

void AudioDeviceManager::SetWiredHeadsetDevEnable()
{
    isWiredHeadsetDevEnable_ = true;
    isBtScoDevEnable_ = false;
    isSpeakerDevEnable_ = false;
    isMicDevEnable_ = false;
}

bool AudioDeviceManager::DoSwitch(AudioDevice device)
{
    bool result = false;
    switch (device) {
        case AudioDevice::DEVICE_BLUETOOTH_SCO:
            result = EnableBtSco();
            break;
        case AudioDevice::DEVICE_WIRED_HEADSET:
            result = EnableWiredHeadset();
            break;
        case AudioDevice::DEVICE_SPEAKER:
            result = EnableSpeaker();
            break;
        case AudioDevice::DEVICE_MIC:
            result = EnableMic();
            break;
        case AudioDevice::DEVICE_DISABLE:
            result = DisableAll();
            break;
        default:
            break;
    }
    return result;
}

bool AudioDeviceManager::HandleEnableOrDisableAudioDeviceEvent(int32_t event)
{
    bool result = false;
    switch (event) {
        case AudioDeviceManager::ENABLE_DEVICE_MIC:
            result = SwitchDevice(AudioDevice::DEVICE_MIC);
            break;
        case AudioDeviceManager::ENABLE_DEVICE_SPEAKER:
            result = SwitchDevice(AudioDevice::DEVICE_SPEAKER);
            break;
        case AudioDeviceManager::ENABLE_DEVICE_BLUETOOTH:
            result = SwitchDevice(AudioDevice::DEVICE_BLUETOOTH_SCO);
            break;
        case AudioDeviceManager::ENABLE_DEVICE_WIRED_HEADSET:
            result = SwitchDevice(AudioDevice::DEVICE_WIRED_HEADSET);
            break;
        case AudioDeviceManager::DEVICES_INACTIVE:
            result = SwitchDevice(AudioDevice::DEVICE_DISABLE);
            break;
        default:
            break;
    }
    return result;
}

bool AudioDeviceManager::IsMicDevEnable()
{
    return isMicDevEnable_;
}

bool AudioDeviceManager::IsWiredHeadsetDevEnable()
{
    return isWiredHeadsetDevEnable_;
}

bool AudioDeviceManager::IsSpeakerDevEnable()
{
    return isSpeakerDevEnable_;
}

bool AudioDeviceManager::IsBtScoDevEnable()
{
    return isBtScoDevEnable_;
}

bool AudioDeviceManager::IsBtScoAvailable()
{
    return isBtScoAvailable_;
}

void AudioDeviceManager::SetBtScoAvailable(bool available)
{
    isBtScoAvailable_ = available;
}

bool AudioDeviceManager::IsWiredHeadsetAvailable()
{
    return isWiredHeadsetAvailable_;
}

void AudioDeviceManager::SetWiredHeadsetAvailable(bool available)
{
    isWiredHeadsetAvailable_ = available;
}

bool AudioDeviceManager::IsSpeakerAvailable()
{
    return isSpeakerAvailable_;
}

void AudioDeviceManager::SetSpeakerAvailable(bool available)
{
    isSpeakerAvailable_ = available;
}
} // namespace Telephony
} // namespace OHOS