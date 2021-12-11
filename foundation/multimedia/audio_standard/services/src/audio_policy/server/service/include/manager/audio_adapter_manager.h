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

#ifndef ST_AUDIO_ADAPTER_MANAGER_H
#define ST_AUDIO_ADAPTER_MANAGER_H

#include <list>
#include <unordered_map>

#include "audio_service_adapter.h"
#include "distributed_kv_data_manager.h"
#include "iaudio_policy_interface.h"
#include "types.h"

namespace OHOS {
namespace AudioStandard {
using namespace OHOS::DistributedKv;

class AudioAdapterManager : public IAudioPolicyInterface {
public:
    static constexpr char HDI_SINK[] = "libmodule-hdi-sink.z.so";
    static constexpr char HDI_SOURCE[] = "libmodule-hdi-source.z.so";
    static constexpr char PIPE_SINK[] = "libmodule-pipe-sink.z.so";
    static constexpr char PIPE_SOURCE[] = "libmodule-pipe-source.z.so";
    static constexpr uint32_t KVSTORE_CONNECT_RETRY_COUNT = 5;
    static constexpr uint32_t KVSTORE_CONNECT_RETRY_DELAY_TIME = 200000;
    static constexpr float MAX_VOLUME = 1.0f;
    static constexpr float MIN_VOLUME = 0.0f;

    bool Init();
    void Deinit(void);

    std::string GetPolicyManagerName();

    static IAudioPolicyInterface& GetInstance()
    {
        static AudioAdapterManager audioAdapterManager;
        return audioAdapterManager;
    }

    int32_t SetStreamVolume(AudioStreamType streamType, float volume);

    float GetStreamVolume(AudioStreamType streamType);

    int32_t SetStreamMute(AudioStreamType streamType, bool mute);

    bool GetStreamMute(AudioStreamType streamType);

    bool IsStreamActive(AudioStreamType streamType);

    AudioIOHandle OpenAudioPort(std::unique_ptr<AudioPortInfo> &audioPortInfo);

    int32_t CloseAudioPort(AudioIOHandle ioHandle);

    int32_t SetDeviceActive(AudioIOHandle ioHandle, InternalDeviceType deviceType, std::string name, bool active);

    int32_t SetRingerMode(AudioRingerMode ringerMode);

    AudioRingerMode GetRingerMode(void);

    virtual ~AudioAdapterManager() {}

private:
    struct UserData {
        AudioAdapterManager *thiz;
        AudioStreamType streamType;
        float volume;
        bool mute;
        bool isCorked;
        uint32_t idx;
    };

    AudioAdapterManager()
        : mRingerMode(RINGER_MODE_NORMAL),
          mAudioPolicyKvStore(nullptr)
    {
        mVolumeMap[STREAM_MUSIC] = MAX_VOLUME;
        mVolumeMap[STREAM_RING] = MAX_VOLUME;
    }

    bool ConnectToPulseAudio(void);
    std::string GetModuleArgs(std::unique_ptr<AudioPortInfo> &audioPortInfo);
    std::string GetStreamNameByStreamType(AudioStreamType streamType);
    AudioStreamType GetStreamIDByType(std::string streamType);
    bool InitAudioPolicyKvStore(bool& isFirstBoot);
    void InitVolumeMap(bool isFirstBoot);
    bool LoadVolumeMap(void);
    void WriteVolumeToKvStore(AudioStreamType streamType, float volume);
    bool LoadVolumeFromKvStore(AudioStreamType streamType);
    void InitRingerMode(bool isFirstBoot);
    bool LoadRingerMode(void);
    void WriteRingerModeToKvStore(AudioRingerMode ringerMode);

    std::unique_ptr<AudioServiceAdapter> mAudioServiceAdapter;
    std::unordered_map<AudioStreamType, float> mVolumeMap;
    AudioRingerMode mRingerMode;
    std::unique_ptr<SingleKvStore> mAudioPolicyKvStore;
    friend class PolicyCallbackImpl;
};

class PolicyCallbackImpl : public AudioServiceAdapterCallback {
public:
    explicit PolicyCallbackImpl(std::unique_ptr<AudioAdapterManager> &audioAdapterManager)
    {
        audioAdapterManager_ = std::move(audioAdapterManager);
    }

    ~PolicyCallbackImpl()
    {
        audioAdapterManager_ = nullptr;
    }

    float OnGetVolumeCb(std::string streamType)
    {
        if (audioAdapterManager_->mRingerMode != RINGER_MODE_NORMAL) {
            if (!streamType.compare("ring")) {
                return AudioAdapterManager::MIN_VOLUME;
            }
        }
        AudioStreamType streamID = audioAdapterManager_->GetStreamIDByType(streamType);
        return audioAdapterManager_->mVolumeMap[streamID];
    }
private:
    std::unique_ptr<AudioAdapterManager> audioAdapterManager_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_PULSEAUDIO_ADAPTER_MANAGER_H
