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

#ifndef ST_AUDIO_POLICY_SERVICE_H
#define ST_AUDIO_POLICY_SERVICE_H

#include "audio_info.h"
#include "audio_policy_manager_factory.h"
#include "iaudio_policy_interface.h"
#include "iport_observer.h"
#include "parser_factory.h"

#include <list>
#include <string>
#include <unordered_map>

namespace OHOS {
namespace AudioStandard {
class AudioPolicyService : public IPortObserver {
public:
    const std::string HDI_SINK = "hdi_output";
    const std::string HDI_SOURCE = "hdi_input";
    const std::string BLUEZ_SINK = "fifo_output";
    const std::string BLUEZ_SOURCE = "fifo_input";
    const std::string PORT_NONE = "none";

    static AudioPolicyService& GetAudioPolicyService()
    {
        static AudioPolicyService audioPolicyService;
        return audioPolicyService;
    }

    bool Init(void);
    void Deinit(void);

    int32_t SetStreamVolume(AudioStreamType streamType, float volume) const;

    float GetStreamVolume(AudioStreamType streamType) const;

    int32_t SetStreamMute(AudioStreamType streamType, bool mute) const;

    bool GetStreamMute(AudioStreamType streamType) const;

    bool IsStreamActive(AudioStreamType streamType) const;

    int32_t SetDeviceActive(InternalDeviceType deviceType, bool active);

    bool IsDeviceActive(InternalDeviceType deviceType) const;

    int32_t SetRingerMode(AudioRingerMode ringMode);

    AudioRingerMode GetRingerMode() const;

    // Parser callbacks
    void OnAudioPortAvailable(std::unique_ptr<AudioPortInfo> portInfo);

    void OnAudioPortPinAvailable(std::unique_ptr<AudioPortPinInfo> portInfo);

    void OnDefaultOutputPortPin(InternalDeviceType device);

    void OnDefaultInputPortPin(InternalDeviceType device);

private:

    AudioPolicyService()
        : mAudioPolicyManager(AudioPolicyManagerFactory::GetAudioPolicyManager()),
          mConfigParser(ParserFactory::GetInstance().CreateParser(*this))
    {
    }

    virtual ~AudioPolicyService() {}

    AudioIOHandle GetAudioIOHandle(InternalDeviceType deviceType);
    std::list<InternalDeviceType>& GetActiveDevicesList(InternalDeviceType deviceType)
    {
        switch (deviceType) {
            case InternalDeviceType::SPEAKER:
            case InternalDeviceType::BLUETOOTH_SCO:
                return mActiveOutputDevices;
            case InternalDeviceType::MIC:
                return mActiveInputDevices;
            default:
                return mActiveOutputDevices; // Default case return Output device
        }
    }

    IAudioPolicyInterface& mAudioPolicyManager;
    Parser& mConfigParser;
    std::unordered_map<std::string, AudioIOHandle> mIOHandles;
    std::list<InternalDeviceType> mActiveOutputDevices;
    std::list<InternalDeviceType> mActiveInputDevices;
    std::string GetPortName(InternalDeviceType deviceType);
};
} // namespace AudioStandard
} // namespace OHOS

#endif // ST_AUDIO_POLICY_SERVICE_H
