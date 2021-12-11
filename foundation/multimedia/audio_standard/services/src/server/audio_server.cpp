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

#include "audio_capturer_source.h"
#include "audio_server.h"
#include "iservice_registry.h"
#include "media_log.h"
#include "system_ability_definition.h"

#define PA
#ifdef PA
extern "C" {
    extern int ohos_pa_main(int argc, char *argv[]);
}
#endif

namespace OHOS {
namespace AudioStandard {
std::map<std::string, std::string> AudioServer::audioParameters;

REGISTER_SYSTEM_ABILITY_BY_ID(AudioServer, AUDIO_DISTRIBUTED_SERVICE_ID, true)

#ifdef PA
constexpr int PA_ARG_COUNT = 1;

void *AudioServer::paDaemonThread(void *arg)
{
    /* Load the mandatory pulseaudio modules at start */
    char *argv[] = {
        (char*)"pulseaudio",
    };

    MEDIA_INFO_LOG("Calling ohos_pa_main\n");
    ohos_pa_main(PA_ARG_COUNT, argv);
    MEDIA_INFO_LOG("Exiting ohos_pa_main\n");

    return nullptr;
}
#endif

AudioServer::AudioServer(int32_t systemAbilityId, bool runOnCreate)
    : SystemAbility(systemAbilityId, runOnCreate)
{}

void AudioServer::OnDump()
{}

void AudioServer::OnStart()
{
    MEDIA_DEBUG_LOG("AudioService OnStart");
    bool res = Publish(this);
    if (res) {
        MEDIA_DEBUG_LOG("AudioService OnStart res=%{public}d", res);
    }

#ifdef PA
    int32_t ret = pthread_create(&m_paDaemonThread, nullptr, AudioServer::paDaemonThread, nullptr);
    if (ret != 0) {
        MEDIA_ERR_LOG("pthread_create failed %d", ret);
    }
    MEDIA_INFO_LOG("Created paDaemonThread\n");
#endif
}

void AudioServer::OnStop()
{
    MEDIA_DEBUG_LOG("AudioService OnStop");
}

void AudioServer::SetAudioParameter(const std::string key, const std::string value)
{
    MEDIA_DEBUG_LOG("server: set audio parameter");
    AudioServer::audioParameters[key] = value;
}

const std::string AudioServer::GetAudioParameter(const std::string key)
{
    MEDIA_DEBUG_LOG("server: get audio parameter");

    if (AudioServer::audioParameters.count(key)) {
        return AudioServer::audioParameters[key];
    } else {
        const std::string value = "";
        return value;
    }
}

int32_t AudioServer::GetMaxVolume(AudioSystemManager::AudioVolumeType volumeType)
{
    MEDIA_DEBUG_LOG("GetMaxVolume server");
    return MAX_VOLUME;
}

int32_t AudioServer::GetMinVolume(AudioSystemManager::AudioVolumeType volumeType)
{
    MEDIA_DEBUG_LOG("GetMinVolume server");
    return MIN_VOLUME;
}

int32_t AudioServer::SetMicrophoneMute(bool isMute)
{
    AudioCapturerSource *audioCapturerSourceInstance = AudioCapturerSource::GetInstance();

    if (audioCapturerSourceInstance->capturerInited_ == false) {
            MEDIA_INFO_LOG("Capturer is not initialized. Set the flag mute state flag");
            AudioCapturerSource::micMuteState_ = isMute;
            return 0;
    }

    return audioCapturerSourceInstance->SetMute(isMute);
}

bool AudioServer::IsMicrophoneMute()
{
    AudioCapturerSource *audioCapturerSourceInstance = AudioCapturerSource::GetInstance();
    bool isMute = false;

    if (audioCapturerSourceInstance->capturerInited_ == false) {
        MEDIA_INFO_LOG("Capturer is not initialized. Get the mic mute state flag value!");
        return AudioCapturerSource::micMuteState_;
    }

    if (audioCapturerSourceInstance->GetMute(isMute)) {
        MEDIA_ERR_LOG("GetMute status in capturer returned Error !");
    }

    return isMute;
}

std::vector<sptr<AudioDeviceDescriptor>> AudioServer::GetDevices(AudioDeviceDescriptor::DeviceFlag deviceFlag)
{
    MEDIA_DEBUG_LOG("GetDevices server");
    audioDeviceDescriptor_.clear();
    sptr<AudioDeviceDescriptor> audioDescriptor = new(std::nothrow) AudioDeviceDescriptor();
    if (audioDescriptor == nullptr) {
        MEDIA_ERR_LOG("new AudioDeviceDescriptor fail");
        return audioDeviceDescriptor_;
    }

    if (AudioDeviceDescriptor::DeviceFlag::INPUT_DEVICES_FLAG == deviceFlag) {
        audioDescriptor->deviceType_ = AudioDeviceDescriptor::DeviceType::MIC;
        audioDescriptor->deviceRole_ = AudioDeviceDescriptor::DeviceRole::INPUT_DEVICE;
    } else if (AudioDeviceDescriptor::DeviceFlag::OUTPUT_DEVICES_FLAG == deviceFlag) {
        audioDescriptor->deviceType_ = AudioDeviceDescriptor::DeviceType::SPEAKER;
        audioDescriptor->deviceRole_ = AudioDeviceDescriptor::DeviceRole::OUTPUT_DEVICE;
    } else if (AudioDeviceDescriptor::DeviceFlag::ALL_DEVICES_FLAG == deviceFlag) {
        sptr<AudioDeviceDescriptor> audioDescriptor_inputDevice = new(std::nothrow) AudioDeviceDescriptor();
        sptr<AudioDeviceDescriptor> audioDescriptor_outputDevice = new(std::nothrow) AudioDeviceDescriptor();
        audioDescriptor_inputDevice->deviceType_ = AudioDeviceDescriptor::DeviceType::MIC;
        audioDescriptor_inputDevice->deviceRole_ = AudioDeviceDescriptor::DeviceRole::INPUT_DEVICE;
        audioDeviceDescriptor_.push_back(audioDescriptor_inputDevice);
        audioDescriptor_outputDevice->deviceType_ = AudioDeviceDescriptor::DeviceType::SPEAKER;
        audioDescriptor_outputDevice->deviceRole_ = AudioDeviceDescriptor::DeviceRole::OUTPUT_DEVICE;
        audioDeviceDescriptor_.push_back(audioDescriptor_outputDevice);
        return audioDeviceDescriptor_;
    }

    audioDeviceDescriptor_.push_back(audioDescriptor);
    return audioDeviceDescriptor_;
}
} // namespace AudioStandard
} // namespace OHOS
