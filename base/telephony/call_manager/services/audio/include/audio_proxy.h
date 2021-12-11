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

#ifndef AUDIO_PROXY_H
#define AUDIO_PROXY_H

#include <cstdint>
#include <memory>
#include <mutex>

#include "audio_manager_proxy.h"
#include "singleton.h"

#include "call_manager_errors.h"

namespace OHOS {
namespace Telephony {
class AudioProxy : public std::enable_shared_from_this<AudioProxy> {
    DECLARE_DELAYED_SINGLETON(AudioProxy)
public:
    void Init();
    bool ActivateAudioInterrupt();
    bool DeactivateAudioInterrupt();
    bool UpdateCallState(int32_t state); // set audio mode
#ifdef ABILITY_AUDIO_SUPPORT
    bool IsDeviceActive(AudioDeviceDescriptor::DeviceType deviceType);
    bool SetDeviceActive(AudioDeviceDescriptor::DeviceType deviceType, bool state);
#endif
    std::string GetDefaultRingerPath() const;
    void SetVolumeAudible();
    bool IsMicrophoneMute();
    int32_t SetMicrophoneMute(bool mute);
    bool SetMicDevActive(bool active);
    bool SetSpeakerDevActive(bool active);
    bool SetBluetoothDevActive(bool active);
    bool SetWiredHeadsetDevActive(bool active);
#ifdef ABILITY_AUDIO_SUPPORT
    AudioRingerMode GetRingerMode() const;
    float GetVolume(AudioSystemManager::AudioVolumeType audioVolumeType);
    int32_t SetVolume(AudioSystemManager::AudioVolumeType audioVolumeType, float volume);
    bool IsStreamActive(AudioSystemManager::AudioVolumeType audioVolumeType);
    bool IsStreamMute(AudioSystemManager::AudioVolumeType audioVolumeType);
    float GetMaxVolume(AudioSystemManager::AudioVolumeType audioVolumeType);
    float GetMinVolume(AudioSystemManager::AudioVolumeType audioVolumeType);
#endif
    void SetAudioDeviceChangeObserver();
    void SetOnCreateCompleteListener();
    bool IsVibrateMode() const;
    int32_t CreateRing(const std::string &ringPath);
    int32_t CreateTone(int32_t toneDescriptor, int32_t durationMs);
    int32_t Play();
    int32_t Play(int32_t toneDescriptor, int32_t duration); // play tone
    int32_t Play(int32_t id, float volume, int32_t loopNum, float speed); // play ringtone
    int32_t StopCallTone();
    int32_t StopRing(int32_t id);
    int32_t Release();
    int32_t Release(int32_t id);
    int32_t Pause(int32_t id);
    int32_t Resume(int32_t id);
    int32_t StartVibrate();
    int32_t CancelVibrate();

private:
    const std::string defaultRingerPath_ = "/system/media/audio/ringtones/default.wav";
};
} // namespace Telephony
} // namespace OHOS
#endif // AUDIO_PROXY_H