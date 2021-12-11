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

#ifndef AUDIO_DEVICE_MANAGER_H
#define AUDIO_DEVICE_MANAGER_H

#include "audio_state.h"

#include "call_manager_inner_type.h"

namespace OHOS {
namespace Telephony {
/**
 * @class AudioDeviceManager
 * describes the available devices of a call.
 */
class AudioDeviceManager {
public:
    AudioDeviceManager();
    virtual ~AudioDeviceManager();
    void Init();
    bool InitAudioDevice();
    bool ProcessEvent(int32_t event);
    static bool IsSpeakerAvailable();
    static bool IsBtScoAvailable();
    static bool IsWiredHeadsetAvailable();
    static void SetSpeakerAvailable(bool available);
    static void SetWiredHeadsetAvailable(bool available);
    static void SetBtScoAvailable(bool available);
    bool SwitchDevice(AudioDevice device);
    enum AudioEventType {
        AUDIO_INTERRUPTED = 200,
        AUDIO_UN_INTERRUPT,
        AUDIO_RINGING,
        ENABLE_DEVICE_MIC,
        ENABLE_DEVICE_SPEAKER,
        ENABLE_DEVICE_WIRED_HEADSET,
        ENABLE_DEVICE_BLUETOOTH,
        DEVICES_INACTIVE,
        WIRED_HEADSET_AVAILABLE,
        WIRED_HEADSET_UNAVAILABLE,
        BLUETOOTH_SCO_AVAILABLE,
        BLUETOOTH_SCO_UNAVAILABLE,
        INIT_AUDIO_DEVICE
    };

private:
    std::mutex mutex_;
    AudioDevice currentAudioDevice_;
    bool isBtScoDevEnable_ = false;
    bool isWiredHeadsetDevEnable_ = false;
    bool isSpeakerDevEnable_ = false;
    bool isMicDevEnable_ = false;
    std::unique_ptr<AudioState> audioState_;
    static bool isMicAvailable_;
    static bool isSpeakerAvailable_;
    static bool isWiredHeadsetAvailable_;
    static bool isBtScoAvailable_;
    bool isAudioActive_;
    bool DoSwitch(AudioDevice device);
    bool EnableBtSco();
    bool EnableWiredHeadset();
    bool EnableSpeaker();
    bool EnableMic();
    bool DisableAll();
    bool IsBtScoDevEnable();
    bool IsWiredHeadsetDevEnable();
    bool IsSpeakerDevEnable();
    bool IsMicDevEnable();
    void SetBtScoDevEnable();
    void SetWiredHeadsetDevEnable();
    void SetSpeakerDevEnable();
    void SetMicDevEnable();
    AudioDevice GetCurrentAudioDevice();
    void SetCurrentAudioDevice(AudioDevice device);
    bool HandleEnableOrDisableAudioDeviceEvent(int32_t event);
};
} // namespace Telephony
} // namespace OHOS
#endif // AUDIO_DEVICE_MANAGER_H