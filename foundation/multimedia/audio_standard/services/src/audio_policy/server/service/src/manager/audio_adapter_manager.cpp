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

#include <memory>
#include <unistd.h>

#include "audio_errors.h"
#include "media_log.h"
#include "audio_adapter_manager.h"

namespace OHOS {
namespace AudioStandard {
bool AudioAdapterManager::Init()
{
    std::unique_ptr<AudioAdapterManager> audioAdapterManager(this);
    std::unique_ptr<PolicyCallbackImpl> policyCallbackImpl = std::make_unique<PolicyCallbackImpl>(audioAdapterManager);
    mAudioServiceAdapter = AudioServiceAdapter::CreateAudioAdapter(std::move(policyCallbackImpl));
    bool result = mAudioServiceAdapter->Connect();
    if (!result) {
        MEDIA_ERR_LOG("[AudioAdapterManager] Error in connecting audio adapter");
        return false;
    }
    bool isFirstBoot = false;
    InitAudioPolicyKvStore(isFirstBoot);
    InitVolumeMap(isFirstBoot);
    InitRingerMode(isFirstBoot);

    return true;
}

void AudioAdapterManager::Deinit(void)
{
    return  mAudioServiceAdapter->Disconnect();
}

int32_t AudioAdapterManager::SetStreamVolume(AudioStreamType streamType, float volume)
{
    // Incase if KvStore didnot connect during  bootup
    if (mAudioPolicyKvStore == nullptr) {
        bool isFirstBoot = false;
        InitAudioPolicyKvStore(isFirstBoot);
    }
    mVolumeMap[streamType] = volume;
    WriteVolumeToKvStore(streamType, volume);

    return mAudioServiceAdapter->SetVolume(streamType, volume);
}

float AudioAdapterManager::GetStreamVolume(AudioStreamType streamType)
{
    return mVolumeMap[streamType];
}

int32_t AudioAdapterManager::SetStreamMute(AudioStreamType streamType, bool mute)
{
    bool result =  mAudioServiceAdapter->SetMute(streamType, mute);
    return result;
}

bool AudioAdapterManager::GetStreamMute(AudioStreamType streamType)
{
    bool result =  mAudioServiceAdapter->IsMute(streamType);
    return result;
}

bool AudioAdapterManager::IsStreamActive(AudioStreamType streamType)
{
    bool result = mAudioServiceAdapter->IsStreamActive(streamType);
    return result;
}

int32_t AudioAdapterManager::SetDeviceActive(AudioIOHandle ioHandle, InternalDeviceType deviceType,
    std::string name, bool active)
{
    switch (deviceType) {
        case InternalDeviceType::SPEAKER:
        case InternalDeviceType::BLUETOOTH_SCO: {
            return mAudioServiceAdapter->SetDefaultSink(name);
        }
        case InternalDeviceType::MIC: {
            return mAudioServiceAdapter->SetDefaultSource(name);
        }
        default:
            break;
    }
    return SUCCESS;
}

int32_t AudioAdapterManager::SetRingerMode(AudioRingerMode ringerMode)
{
    mRingerMode = ringerMode;

    // Incase if KvStore didnot connect during  bootup
    if (mAudioPolicyKvStore == nullptr) {
        bool isFirstBoot = false;
        InitAudioPolicyKvStore(isFirstBoot);
    }

    WriteRingerModeToKvStore(ringerMode);
    return SUCCESS;
}

AudioRingerMode AudioAdapterManager::GetRingerMode()
{
    return mRingerMode;
}

AudioIOHandle AudioAdapterManager::OpenAudioPort(std::unique_ptr<AudioPortInfo> &audioPortInfo)
{
    std::string moduleArgs = GetModuleArgs(audioPortInfo);
    MEDIA_INFO_LOG("[AudioAdapterManager] load-module %{public}s %{public}s", audioPortInfo->name, moduleArgs.c_str());
    if (!strcmp(audioPortInfo->name, PIPE_SOURCE) || !strcmp(audioPortInfo->name, PIPE_SINK)) {
        if (audioPortInfo->fileName != nullptr && access(audioPortInfo->fileName, F_OK) == 0) {
            int32_t ret = std::remove(audioPortInfo->fileName);
            if (ret) {
                MEDIA_ERR_LOG("[AudioAdapterManager] Error Removing file: %{public}s Failed! ret val: %{public}d",
                    audioPortInfo->fileName, ret);
            }
        } else {
            MEDIA_ERR_LOG("[AudioAdapterManager] Error audioPortInfo->fileName is null! or file not exists");
        }
    }
    AudioIOHandle ioHandle = mAudioServiceAdapter->OpenAudioPort(audioPortInfo->name, moduleArgs.c_str());
    return ioHandle;
}

int32_t AudioAdapterManager::CloseAudioPort(AudioIOHandle ioHandle)
{
    return mAudioServiceAdapter->CloseAudioPort(ioHandle);
}

// Private Members
std::string AudioAdapterManager::GetModuleArgs(std::unique_ptr<AudioPortInfo> &audioPortInfo)
{
    std::string args;

    if (!strcmp(audioPortInfo->name, HDI_SINK)) {
        if (audioPortInfo->rate != nullptr) {
            args = "rate=";
            args.append(audioPortInfo->rate);
        }

        if (audioPortInfo->channels != nullptr) {
            args.append(" channels=");
            args.append(audioPortInfo->channels);
        }

        if (audioPortInfo->buffer_size != nullptr) {
            args.append(" buffer_size=");
            args.append(audioPortInfo->buffer_size);
        }
    } else if (!strcmp(audioPortInfo->name, HDI_SOURCE)) {
        if (audioPortInfo->rate != nullptr) {
            args = "rate=";
            args.append(audioPortInfo->rate);
        }

        if (audioPortInfo->channels != nullptr) {
            args.append(" channels=");
            args.append(audioPortInfo->channels);
        }

        if (audioPortInfo->buffer_size != nullptr) {
            args.append(" buffer_size=");
            args.append(audioPortInfo->buffer_size);
        }
    } else if (!strcmp(audioPortInfo->name, PIPE_SINK)) {
        if (audioPortInfo->fileName != nullptr) {
            args = "file=";
            args.append(audioPortInfo->fileName);
        }
    } else if (!strcmp(audioPortInfo->name, PIPE_SOURCE)) {
        if (audioPortInfo->fileName != nullptr) {
            args = "file=";
            args.append(audioPortInfo->fileName);
        }
    }

    return args;
}

std::string AudioAdapterManager::GetStreamNameByStreamType(AudioStreamType streamType)
{
    switch (streamType) {
        case STREAM_MUSIC:
            return "music";
        case STREAM_RING:
            return "ring";
        case STREAM_SYSTEM:
            return "system";
        case STREAM_NOTIFICATION:
            return "notification";
        case STREAM_ALARM:
            return "alarm";
        case STREAM_DTMF:
            return "dtmf";
        default:
            return "";
    }
}

AudioStreamType AudioAdapterManager::GetStreamIDByType(std::string streamType)
{
    AudioStreamType stream = STREAM_MUSIC;

    if (!streamType.compare(std::string("music")))
        stream = STREAM_MUSIC;
    else if (!streamType.compare(std::string("ring")))
        stream = STREAM_RING;
    else if (!streamType.compare(std::string("system")))
        stream = STREAM_SYSTEM;
    else if (!streamType.compare(std::string("notification")))
        stream = STREAM_NOTIFICATION;
    else if (!streamType.compare(std::string("alarm")))
        stream = STREAM_ALARM;

    return stream;
}

bool AudioAdapterManager::InitAudioPolicyKvStore(bool& isFirstBoot)
{
    DistributedKvDataManager manager;
    Options options;

    options.createIfMissing = false;
    options.encrypt = false;
    options.autoSync = true;
    options.kvStoreType = KvStoreType::SINGLE_VERSION;

    AppId appId;
    appId.appId = "policymanager";
    StoreId storeId;
    storeId.storeId = "audiopolicy";
    Status status = Status::SUCCESS;

    // open and initialize kvstore instance.
    if (mAudioPolicyKvStore == nullptr) {
        uint32_t retries = 0;

        do {
            manager.GetSingleKvStore(
                options, appId, storeId, [&](Status paramStatus, std::unique_ptr<SingleKvStore> singleKvStore) {
                status = paramStatus;

                if (status == Status::SUCCESS) {
                    mAudioPolicyKvStore = std::move(singleKvStore);
                } else if (status == Status::STORE_NOT_FOUND) {
                    MEDIA_ERR_LOG("[AudioAdapterManager] InitAudioPolicyKvStore: STORE_NOT_FOUND!");
                    return;
                } else {
                    MEDIA_ERR_LOG("[AudioAdapterManager] InitAudioPolicyKvStore: Kvstore Connect failed! Retrying.");
                    return;
                }
            });

            if ((status == Status::SUCCESS) || (status == Status::STORE_NOT_FOUND)) {
                break;
            } else {
                retries++;
                usleep(KVSTORE_CONNECT_RETRY_DELAY_TIME);
            }
        } while (retries <= KVSTORE_CONNECT_RETRY_COUNT);
    }

    if (mAudioPolicyKvStore == nullptr) {
        if (status == Status::STORE_NOT_FOUND) {
            MEDIA_INFO_LOG("[AudioAdapterManager] First Boot: Create AudioPolicyKvStore");
            options.createIfMissing = true;
            // [create and] open and initialize kvstore instance.
            manager.GetSingleKvStore(
                options, appId, storeId, [&](Status status, std::unique_ptr<SingleKvStore> singleKvStore) {
                if (status != Status::SUCCESS) {
                    MEDIA_ERR_LOG("[AudioAdapterManager] Create AudioPolicyKvStore Failed!");
                    return;
                }

                mAudioPolicyKvStore = std::move(singleKvStore);
                isFirstBoot = true;
            });
        }
    }

    if (mAudioPolicyKvStore == nullptr) {
        MEDIA_ERR_LOG("[AudioAdapterManager] InitAudioPolicyKvStore: Failed!");
        return false;
    }

    return true;
}

void AudioAdapterManager::InitVolumeMap(bool isFirstBoot)
{
    if (isFirstBoot == true) {
        WriteVolumeToKvStore(STREAM_MUSIC, MAX_VOLUME);
        WriteVolumeToKvStore(STREAM_RING, MAX_VOLUME);
        MEDIA_INFO_LOG("[AudioAdapterManager] Wrote default stream volumes to KvStore");
    } else {
        LoadVolumeMap();
    }
    return;
}

void AudioAdapterManager::InitRingerMode(bool isFirstBoot)
{
    if (mAudioPolicyKvStore == nullptr) {
        MEDIA_ERR_LOG("[AudioAdapterManager] mAudioPolicyKvStore is null!");
        return;
    }

    if (isFirstBoot == true) {
        mRingerMode = RINGER_MODE_NORMAL;
        WriteRingerModeToKvStore(RINGER_MODE_NORMAL);
        MEDIA_INFO_LOG("[AudioAdapterManager] Wrote default ringer mode to KvStore");
    } else {
        LoadRingerMode();
    }
}

bool AudioAdapterManager::LoadVolumeFromKvStore(AudioStreamType streamType)
{
    Key key;
    Value value;

    switch (streamType) {
        case STREAM_MUSIC:
            key = "music";
            break;
        case STREAM_RING:
            key = "ring";
            break;
        default:
            return false;
    }

    Status status = mAudioPolicyKvStore->Get(key, value);
    if (status == Status::SUCCESS) {
        float volume = TransferByteArrayToType<float>(value.Data());
        mVolumeMap[streamType] = volume;
        MEDIA_DEBUG_LOG("[AudioAdapterManager] volume from kvStore %{public}f for streamType:%{public}d",
                        volume, streamType);
        return true;
    }

    return false;
}

bool AudioAdapterManager::LoadVolumeMap(void)
{
    if (mAudioPolicyKvStore == nullptr) {
        MEDIA_ERR_LOG("[AudioAdapterManager] LoadVolumeMap: mAudioPolicyKvStore is null!");
        return false;
    }

    if (!LoadVolumeFromKvStore(STREAM_MUSIC))
        MEDIA_ERR_LOG("[AudioAdapterManager] LoadVolumeMap: Couldnot load volume for Music from kvStore!");

    if (!LoadVolumeFromKvStore(STREAM_RING))
        MEDIA_ERR_LOG("[AudioAdapterManager] LoadVolumeMap: Couldnot load volume for Ring from kvStore!");

    return true;
}

bool AudioAdapterManager::LoadRingerMode(void)
{
    if (mAudioPolicyKvStore == nullptr) {
        MEDIA_ERR_LOG("[AudioAdapterManager] LoadRingerMap: mAudioPolicyKvStore is null!");
        return false;
    }

    // get ringer mode value from kvstore.
    Key key = "ringermode";
    Value value;
    Status status = mAudioPolicyKvStore->Get(key, value);
    if (status == Status::SUCCESS) {
        mRingerMode = static_cast<AudioRingerMode>(TransferByteArrayToType<int>(value.Data()));
        MEDIA_DEBUG_LOG("[AudioAdapterManager] Ringer Mode from kvStore %{public}d", mRingerMode);
    }

    return true;
}

void AudioAdapterManager::WriteVolumeToKvStore(AudioStreamType streamType, float volume)
{
    if (mAudioPolicyKvStore == nullptr)
        return;

    Key key = GetStreamNameByStreamType(streamType);
    Value value = Value(TransferTypeToByteArray<float>(volume));

    Status status = mAudioPolicyKvStore->Put(key, value);
    if (status == Status::SUCCESS) {
        MEDIA_INFO_LOG("[AudioAdapterManager] volume %{public}f for %{public}s updated in kvStore", volume,
            GetStreamNameByStreamType(streamType).c_str());
    } else {
        MEDIA_ERR_LOG("[AudioAdapterManager] volume %{public}f for %{public}s failed to update in kvStore!", volume,
            GetStreamNameByStreamType(streamType).c_str());
    }

    return;
}

void AudioAdapterManager::WriteRingerModeToKvStore(AudioRingerMode ringerMode)
{
    if (mAudioPolicyKvStore == nullptr)
        return;

    Key key = "ringermode";
    Value value = Value(TransferTypeToByteArray<int>(ringerMode));

    Status status = mAudioPolicyKvStore->Put(key, value);
    if (status == Status::SUCCESS) {
        MEDIA_INFO_LOG("[AudioAdapterManager] Wrote RingerMode:%d to kvStore", ringerMode);
    } else {
        MEDIA_ERR_LOG("[AudioAdapterManager] Writing RingerMode:%d to kvStore failed!", ringerMode);
    }

    return;
}
} // namespace AudioStandard
} // namespace OHOS
