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

#include "audio_proxy.h"

#ifdef ABILITY_AUDIO_SUPPORT
#include "audio_service_client.h"
#include "audio_system_manager.h"
#endif
#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability.h"
#include "system_ability_definition.h"

#ifdef ABILITY_AUDIO_SUPPORT
#include "audio_player.h"
#endif

#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
AudioProxy::AudioProxy() {}

AudioProxy::~AudioProxy() {}

void AudioProxy::Init()
{
    TELEPHONY_LOGI("audio proxy init");
}

bool AudioProxy::ActivateAudioInterrupt()
{
#ifdef ABILITY_AUDIO_SUPPORT
    return AudioSystemManager::GetInstance()->ActivateAudioInterrupt();
#endif
    return true;
}

bool AudioProxy::DeactivateAudioInterrupt()
{
#ifdef ABILITY_AUDIO_SUPPORT
    return AudioSystemManager::GetInstance()->DeactivateAudioInterrupt();
#endif
    return true;
}

bool AudioProxy::UpdateCallState(int32_t state)
{
#ifdef ABILITY_AUDIO_SUPPORT
    return AudioSystemManager::GetInstance()->UpdateCallState(state);
#endif
    return true;
}

int32_t AudioProxy::CreateRing(const std::string &ringPath)
{
#ifdef ABILITY_AUDIO_SUPPORT
    return AudioPlayer::GetInstance()->CreatePlayer(ringPath);
#endif
    return TELEPHONY_SUCCESS;
}

int32_t AudioProxy::CreateTone(int32_t toneDescriptor, int32_t durationMs)
{
#ifdef ABILITY_AUDIO_SUPPORT
    return AudioPlayer::GetInstance()->CreatePlayer(toneDescriptor, durationMs);
#endif
    return TELEPHONY_SUCCESS;
}

int32_t AudioProxy::Play(int32_t id, float volume, int32_t loopNum, float speed)
{
#ifdef ABILITY_AUDIO_SUPPORT
    return AudioPlayer::GetInstance()->Play(id, volume, loopNum, speed);
#endif
    return TELEPHONY_SUCCESS;
}

int32_t AudioProxy::Play(int32_t toneDescriptor, int32_t duration)
{
#ifdef ABILITY_AUDIO_SUPPORT
    return AudioPlayer::GetInstance()->Play(toneDescriptor, duration);
#endif
    return TELEPHONY_SUCCESS;
}

int32_t AudioProxy::Play()
{
#ifdef ABILITY_AUDIO_SUPPORT
    return AudioPlayer::GetInstance()->Play();
#endif
    return TELEPHONY_SUCCESS;
}

int32_t AudioProxy::StopCallTone()
{
#ifdef ABILITY_AUDIO_SUPPORT
    return AudioPlayer::GetInstance()->StopCallTone();
#endif
    return TELEPHONY_SUCCESS;
}

int32_t AudioProxy::StopRing(int32_t id)
{
#ifdef ABILITY_AUDIO_SUPPORT
    return AudioPlayer::GetInstance()->StopRing(id);
#endif
    return TELEPHONY_SUCCESS;
}

int32_t AudioProxy::Resume(int32_t id)
{
#ifdef ABILITY_AUDIO_SUPPORT
    return AudioPlayer::GetInstance()->Resume(id);
#endif
    return TELEPHONY_SUCCESS;
}

int32_t AudioProxy::Pause(int32_t id)
{
#ifdef ABILITY_AUDIO_SUPPORT
    return AudioPlayer::GetInstance()->Pause(id);
#endif
    return TELEPHONY_SUCCESS;
}

int32_t AudioProxy::Release()
{
#ifdef ABILITY_AUDIO_SUPPORT
    return AudioPlayer::GetInstance()->Release();
#endif
    return TELEPHONY_SUCCESS;
}

int32_t AudioProxy::Release(int32_t id)
{
#ifdef ABILITY_AUDIO_SUPPORT
    return AudioPlayer::GetInstance()->Release(id);
#endif
    return TELEPHONY_SUCCESS;
}

std::string AudioProxy::GetDefaultRingerPath() const
{
    return defaultRingerPath_;
}

int32_t AudioProxy::StartVibrate()
{
    return TELEPHONY_SUCCESS; // return default value
}

int32_t AudioProxy::CancelVibrate()
{
    return TELEPHONY_SUCCESS; // return default value
}

void AudioProxy::SetAudioDeviceChangeObserver() {}

void AudioProxy::SetOnCreateCompleteListener() {}

#ifdef ABILITY_AUDIO_SUPPORT
bool AudioProxy::SetDeviceActive(AudioDeviceDescriptor::DeviceType deviceType, bool state)
{
    return AudioSystemManager::GetInstance()->SetDeviceActive(deviceType, state) == TELEPHONY_SUCCESS;
}

bool AudioProxy::IsDeviceActive(AudioDeviceDescriptor::DeviceType deviceType)
{
    return AudioSystemManager::GetInstance()->IsDeviceActive(deviceType);
}
#endif

bool AudioProxy::SetBluetoothDevActive(bool active)
{
#ifdef ABILITY_AUDIO_SUPPORT
    return SetDeviceActive(AudioDeviceDescriptor::DeviceType::BLUETOOTH_SCO, active);
#endif
    return true;
}

bool AudioProxy::SetWiredHeadsetDevActive(bool active)
{
#ifdef ABILITY_AUDIO_SUPPORT
    return SetDeviceActive(AudioDeviceDescriptor::DeviceType::WIRED_HEADSET, active);
#endif
    return true;
}

bool AudioProxy::SetSpeakerDevActive(bool active)
{
#ifdef ABILITY_AUDIO_SUPPORT
    return SetDeviceActive(AudioDeviceDescriptor::DeviceType::SPEAKER, active);
#endif
    return true;
}

bool AudioProxy::SetMicDevActive(bool active)
{
#ifdef ABILITY_AUDIO_SUPPORT
    return SetDeviceActive(AudioDeviceDescriptor::DeviceType::MIC, active);
#endif
    return true;
}

#ifdef ABILITY_AUDIO_SUPPORT
float AudioProxy::GetVolume(AudioSystemManager::AudioVolumeType audioVolumeType)
{
    return AudioSystemManager::GetInstance()->GetVolume(audioVolumeType);
}

int32_t AudioProxy::SetVolume(AudioSystemManager::AudioVolumeType audioVolumeType, float volume)
{
    return AudioSystemManager::GetInstance()->SetVolume(audioVolumeType, volume);
}
#endif

void AudioProxy::SetVolumeAudible()
{
#ifdef ABILITY_AUDIO_SUPPORT
    if (GetVolume(AudioSystemManager::AudioVolumeType::STREAM_VOICE_CALL) == 0) {
        float volume = GetMaxVolume(AudioSystemManager::AudioVolumeType::STREAM_VOICE_CALL);
        SetVolume(AudioSystemManager::AudioVolumeType::STREAM_VOICE_CALL, volume);
    }
#endif
}

#ifdef ABILITY_AUDIO_SUPPORT
bool AudioProxy::IsStreamActive(AudioSystemManager::AudioVolumeType audioVolumeType)
{
    return AudioSystemManager::GetInstance()->IsStreamActive(audioVolumeType);
}

bool AudioProxy::IsStreamMute(AudioSystemManager::AudioVolumeType audioVolumeType)
{
    return AudioSystemManager::GetInstance()->IsStreamMute(audioVolumeType);
}

float AudioProxy::GetMaxVolume(AudioSystemManager::AudioVolumeType audioVolumeType)
{
    return AudioSystemManager::GetInstance()->GetMaxVolume(audioVolumeType);
}

float AudioProxy::GetMinVolume(AudioSystemManager::AudioVolumeType audioVolumeType)
{
    return AudioSystemManager::GetInstance()->GetMinVolume(audioVolumeType);
}
#endif

bool AudioProxy::IsMicrophoneMute()
{
#ifdef ABILITY_AUDIO_SUPPORT
    return AudioSystemManager::GetInstance()->IsMicrophoneMute();
#endif
    return false;
}

int32_t AudioProxy::SetMicrophoneMute(bool mute)
{
#ifdef ABILITY_AUDIO_SUPPORT
    return AudioSystemManager::GetInstance()->SetMicrophoneMute(mute);
#endif
    return TELEPHONY_SUCCESS;
}

#ifdef ABILITY_AUDIO_SUPPORT
AudioRingerMode AudioProxy::GetRingerMode() const
{
    return AudioSystemManager::GetInstance()->GetRingerMode();
}

bool AudioProxy::IsVibrateMode() const
{
    return AudioRingerMode::RINGER_MODE_VIBRATE == GetRingerMode();
}
#endif
} // namespace Telephony
} // namespace OHOS