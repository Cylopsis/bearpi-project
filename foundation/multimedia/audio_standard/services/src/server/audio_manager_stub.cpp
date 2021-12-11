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

#include "audio_manager_base.h"
#include "audio_system_manager.h"
#include "media_log.h"

namespace OHOS {
namespace AudioStandard {
int AudioManagerStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    MEDIA_DEBUG_LOG("OnRemoteRequest, cmd = %{public}u", code);
    if (!IsPermissionValid()) {
        MEDIA_ERR_LOG("caller app not acquired audio permission");
        return MEDIA_PERMISSION_DENIED;
    }

    switch (code) {
        case GET_MAX_VOLUME: {
            MEDIA_DEBUG_LOG("GET_MAX_VOLUME AudioManagerStub");
            int volumeType = data.ReadInt32();
            MEDIA_DEBUG_LOG("GET_MAX_VOLUME volumeType received from client= %{public}d", volumeType);
            AudioSystemManager::AudioVolumeType volumeStreamConfig =
                   static_cast<AudioSystemManager::AudioVolumeType>(volumeType);
            MEDIA_DEBUG_LOG("GET_MAX_VOLUME volumeType= %{public}d", volumeStreamConfig);
            int32_t ret = GetMaxVolume(volumeStreamConfig);
            reply.WriteInt32(ret);
            return MEDIA_OK;
        }
        case GET_MIN_VOLUME: {
            MEDIA_DEBUG_LOG("GET_MIN_VOLUME AudioManagerStub");
            int volumeType = data.ReadInt32();
            MEDIA_DEBUG_LOG("GET_MIN_VOLUME volumeType received from client= %{public}d", volumeType);
            AudioSystemManager::AudioVolumeType volumeStreamConfig =
                   static_cast<AudioSystemManager::AudioVolumeType>(volumeType);
            MEDIA_DEBUG_LOG("GET_MIN_VOLUME volumeType= %{public}d", volumeStreamConfig);
            int32_t ret = GetMinVolume(volumeStreamConfig);
            reply.WriteInt32(ret);
            return MEDIA_OK;
        }
        case GET_DEVICES: {
            MEDIA_DEBUG_LOG("GET_DEVICES AudioManagerStub");
            int deviceFlag = data.ReadInt32();
            AudioDeviceDescriptor::DeviceFlag deviceFlagConfig =
                   static_cast<AudioDeviceDescriptor::DeviceFlag>(deviceFlag);
            std::vector<sptr<AudioDeviceDescriptor>> devices = GetDevices(deviceFlagConfig);
            int32_t size = devices.size();
            MEDIA_DEBUG_LOG("GET_DEVICES size= %{public}d", size);
            reply.WriteInt32(size);
            for (int i = 0; i < size; i++) {
                devices[i]->Marshalling(reply);
            }
            return MEDIA_OK;
        }
        case SET_AUDIO_PARAMETER: {
            MEDIA_DEBUG_LOG("SET_AUDIO_PARAMETER AudioManagerStub");
            const std::string key = data.ReadString();
            const std::string value = data.ReadString();
            MEDIA_DEBUG_LOG("SET_AUDIO_PARAMETER key-value pair from client= %{public}s, %{public}s",
                            key.c_str(), value.c_str());
            SetAudioParameter(key, value);
            return MEDIA_OK;
        }
        case GET_AUDIO_PARAMETER: {
            MEDIA_DEBUG_LOG("GET_AUDIO_PARAMETER AudioManagerStub");
            const std::string key = data.ReadString();
            MEDIA_DEBUG_LOG("GET_AUDIO_PARAMETER key received from client= %{public}s", key.c_str());
            const std::string value = GetAudioParameter(key);
            reply.WriteString(value);
            return MEDIA_OK;
        }
        case SET_MICROPHONE_MUTE: {
            MEDIA_DEBUG_LOG("SET_MICROPHONE_MUTE AudioManagerStub");
            bool isMute = data.ReadBool();
            MEDIA_DEBUG_LOG("SET_MICROPHONE_MUTE isMute value from client= %{public}d", isMute);
            int32_t result = SetMicrophoneMute(isMute);
            reply.WriteInt32(result);
            return MEDIA_OK;
        }
        case IS_MICROPHONE_MUTE: {
            MEDIA_DEBUG_LOG("IS_MICROPHONE_MUTE AudioManagerStub");
            bool isMute = IsMicrophoneMute();
            reply.WriteBool(isMute);
            return MEDIA_OK;
        }
        default: {
            MEDIA_ERR_LOG("default case, need check AudioManagerStub");
            return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
        }
    }
}

bool AudioManagerStub::IsPermissionValid()
{
    return true;
}
} // namespace AudioStandard
} // namespace OHOS
