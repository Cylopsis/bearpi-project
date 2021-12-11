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

#ifndef ST_AUDIO_SYSTEM_MANAGER_H
#define ST_AUDIO_SYSTEM_MANAGER_H

#include <cstdlib>

#include "parcel.h"
#include "audio_info.h"

namespace OHOS {
namespace AudioStandard {
class AudioDeviceDescriptor;
class AudioDeviceDescriptor : public Parcelable {
    friend class AudioSystemManager;
public:
enum DeviceFlag {
        /**
         * Device flag none.
         */
        DEVICE_FLAG_NONE = -1,
        /**
         * Indicates all output audio devices.
         */
        OUTPUT_DEVICES_FLAG = 1,
        /**
         * Indicates all input audio devices.
         */
        INPUT_DEVICES_FLAG = 2,
        /**
         * Indicates all audio devices.
         */
        ALL_DEVICES_FLAG = 3,
        /**
         * Device flag max count.
         */
        DEVICE_FLAG_MAX
};

enum DeviceRole {
        /**
         * Device role none.
         */
        DEVICE_ROLE_NONE = -1,
        /**
         * Input device role.
         */
        INPUT_DEVICE = 1,
        /**
         * Output device role.
         */
        OUTPUT_DEVICE = 2,
        /**
         * Device role max count.
         */
        DEVICE_ROLE_MAX
};

enum DeviceType {
        /**
         * Indicates device type none.
         */
        DEVICE_TYPE_NONE = -1,
        /**
         * Indicates invalid device
         */
        INVALID = 0,
        /**
         * Indicates earpiece
         */
        EARPIECE = 1,
        /**
         * Indicates a speaker built in a device.
         */
        SPEAKER = 2,
        /**
         * Indicates a headset, which is the combination of a pair of headphones and a microphone.
         */
        WIRED_HEADSET = 3,
        /**
         * Indicates a Bluetooth device used for telephony.
         */
        BLUETOOTH_SCO = 7,
        /**
         * Indicates a Bluetooth device supporting the Advanced Audio Distribution Profile (A2DP).
         */
        BLUETOOTH_A2DP = 8,
        /**
         * Indicates a microphone built in a device.
         */
        MIC = 15,
        /**
         * Indicates device type max count.
         */
        DEVICE_TYPE_MAX
};

    DeviceType getType();
    DeviceRole getRole();
    DeviceType deviceType_;
    DeviceRole deviceRole_;
    AudioDeviceDescriptor();
    virtual ~AudioDeviceDescriptor();
    bool Marshalling(Parcel &parcel) const override;
    static AudioDeviceDescriptor *Unmarshalling(Parcel &parcel);
};

/**
 * @brief The AudioSystemManager class is an abstract definition of audio manager.
 *        Provides a series of client/interfaces for audio management
 */

class AudioSystemManager {
public:
enum AudioVolumeType {
        /**
         * Indicates audio streams of voices in calls.
         */
        STREAM_VOICE_CALL = 1,
        /**
         * Indicates audio streams for system sounds.
         */
        STREAM_SYSTEM = 2,
        /**
         * Indicates audio streams for ringtones.
         */
        STREAM_RING = 3,
        /**
         * Indicates audio streams for music playback.
         */
        STREAM_MUSIC = 4,
        /**
         * Indicates audio streams for alarms.
         */
        STREAM_ALARM = 5,
        /**
         * Indicates audio streams for notifications.
         */
        STREAM_NOTIFICATION = 6,
        /**
         * Indicates audio streams for voice calls routed through a connected Bluetooth device.
         */
        STREAM_BLUETOOTH_SCO = 7,
        /**
         * Indicates audio streams for dual-tone multi-frequency (DTMF) tones.
         */
        STREAM_DTMF = 8,
        /**
         * Indicates audio streams exclusively transmitted through the speaker (text-to-speech) of a device.
         */
        STREAM_TTS =  9,
        /**
         * Indicates audio streams used for prompts in terms of accessibility.
         */
        STREAM_ACCESSIBILITY = 10
    };

    static AudioSystemManager *GetInstance();
    static float MapVolumeToHDI(int32_t volume);
    static int32_t MapVolumeFromHDI(float volume);
    int32_t SetVolume(AudioSystemManager::AudioVolumeType volumeType, int32_t volume) const;
    int32_t GetVolume(AudioSystemManager::AudioVolumeType volumeType) const;
    int32_t GetMaxVolume(AudioSystemManager::AudioVolumeType volumeType) const;
    int32_t GetMinVolume(AudioSystemManager::AudioVolumeType volumeType) const;
    int32_t SetMute(AudioSystemManager::AudioVolumeType volumeType, bool mute) const;
    bool IsStreamMute(AudioSystemManager::AudioVolumeType volumeType) const;
    int32_t SetMicrophoneMute(bool isMute) const;
    bool IsMicrophoneMute(void) const;
    std::vector<sptr<AudioDeviceDescriptor>> GetDevices(AudioDeviceDescriptor::DeviceFlag deviceFlag) const;
    const std::string GetAudioParameter(const std::string key) const;
    void SetAudioParameter(const std::string key, const std::string value) const;
    int32_t SetDeviceActive(ActiveDeviceType deviceType, bool flag) const;
    bool IsDeviceActive(ActiveDeviceType deviceType) const;
    bool IsStreamActive(AudioSystemManager::AudioVolumeType volumeType) const;
    bool SetRingerMode(AudioRingerMode ringMode) const;
    AudioRingerMode GetRingerMode() const;
private:
    AudioSystemManager();
    virtual ~AudioSystemManager();
    void init();
    static constexpr int32_t MAX_VOLUME_LEVEL = 15;
    static constexpr int32_t MIN_VOLUME_LEVEL = 0;
    static constexpr int32_t CONST_FACTOR = 100;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_SYSTEM_MANAGER_H
