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

#include <cstring>
#include <dlfcn.h>

#include "audio_capturer_source.h"
#include "audio_errors.h"
#include "media_log.h"

namespace OHOS {
namespace AudioStandard {
#ifdef CAPTURE_DUMP
const char *g_audioOutTestFilePath = "/data/local/tmp/audio_capture.pcm";
#endif // CAPTURE_DUMP

bool AudioCapturerSource::micMuteState_ = false;

AudioCapturerSource::AudioCapturerSource()
    : capturerInited_(false), started_(false), paused_(false), leftVolume_(MAX_VOLUME_LEVEL),
      rightVolume_(MAX_VOLUME_LEVEL), audioManager_(nullptr), audioAdapter_(nullptr), audioCapture_(nullptr),
      handle_(nullptr)
{
    attr_ = {};
#ifdef CAPTURE_DUMP
    pfd = nullptr;
#endif // CAPTURE_DUMP
}

AudioCapturerSource::~AudioCapturerSource()
{
    DeInit();
}

AudioCapturerSource *AudioCapturerSource::GetInstance()
{
    static AudioCapturerSource audioCapturer_;
    return &audioCapturer_;
}

void AudioCapturerSource::DeInit()
{
    started_ = false;
    capturerInited_ = false;

    if ((audioCapture_ != nullptr) && (audioAdapter_ != nullptr)) {
        audioAdapter_->DestroyCapture(audioAdapter_, audioCapture_);
    }
    audioCapture_ = nullptr;

    if ((audioManager_ != nullptr) && (audioAdapter_ != nullptr)) {
        audioManager_->UnloadAdapter(audioManager_, audioAdapter_);
    }
    audioAdapter_ = nullptr;
    audioManager_ = nullptr;
    dlclose(handle_);
#ifdef CAPTURE_DUMP
    if (pfd) {
        fclose(pfd);
        pfd = nullptr;
    }
#endif // CAPTURE_DUMP
}

int32_t InitAttrsCapture(struct AudioSampleAttributes &attrs)
{
    /* Initialization of audio parameters for playback */
    attrs.format = AUDIO_FORMAT_PCM_16_BIT;
    attrs.channelCount = AUDIO_CHANNELCOUNT;
    attrs.sampleRate = AUDIO_SAMPLE_RATE_48K;
    attrs.interleaved = 0;
    attrs.type = AUDIO_IN_MEDIA;
    attrs.period = DEEP_BUFFER_CAPTURE_PERIOD_SIZE;
    attrs.frameSize = PCM_16_BIT * attrs.channelCount / PCM_8_BIT;
    attrs.isBigEndian = false;
    attrs.isSignedData = true;
    attrs.startThreshold = DEEP_BUFFER_CAPTURE_PERIOD_SIZE / (attrs.frameSize);
    attrs.stopThreshold = INT_32_MAX;
    /* 16 * 1024 */
    attrs.silenceThreshold = AUDIO_BUFF_SIZE;

    return SUCCESS;
}

int32_t SwitchAdapterCapture(struct AudioAdapterDescriptor *descs, const char *adapterNameCase,
    enum AudioPortDirection portFlag, struct AudioPort *capturePort, const int32_t size)
{
    if (descs == nullptr || adapterNameCase == nullptr || capturePort == nullptr) {
        return ERROR;
    }

    for (int32_t index = 0; index < size; index++) {
        struct AudioAdapterDescriptor *desc = &descs[index];
        if (desc == nullptr) {
            continue;
        }
        if (!strcmp(desc->adapterName, adapterNameCase)) {
            for (uint32_t port = 0; ((desc != nullptr) && (port < desc->portNum)); port++) {
                // Only find out the port of out in the sound card
                if (desc->ports[port].dir == portFlag) {
                    *capturePort = desc->ports[port];
                    return index;
                }
            }
        }
    }
    MEDIA_ERR_LOG("SwitchAdapter Fail");

    return ERR_INVALID_INDEX;
}

int32_t AudioCapturerSource::InitAudioManager()
{
    MEDIA_INFO_LOG("AudioCapturerSource: Initialize audio proxy manager");
    char resolvedPath[100] = "/system/lib/libaudio_hdi_proxy_server.z.so";
    struct AudioProxyManager *(*getAudioManager)() = nullptr;

    handle_ = dlopen(resolvedPath, 1);
    if (handle_ == nullptr) {
        MEDIA_ERR_LOG("Open Capturer so Fail");
        return ERR_INVALID_HANDLE;
    }

    getAudioManager = (struct AudioProxyManager *(*)())(dlsym(handle_, "GetAudioProxyManagerFuncs"));
    if (getAudioManager == nullptr) {
        return ERR_INVALID_HANDLE;
    }

    audioManager_ = getAudioManager();
    if (audioManager_ == nullptr) {
        return ERR_INVALID_HANDLE;
    }

    return 0;
}

int32_t AudioCapturerSource::CreateCapture(struct AudioPort &capturePort)
{
    // Initialization port information, can fill through mode and other parameters
    int32_t ret = audioAdapter_->InitAllPorts(audioAdapter_);
    if (ret != 0) {
        MEDIA_ERR_LOG("InitAllPorts failed");
        return ERR_DEVICE_INIT;
    }

    struct AudioSampleAttributes param;
    // User needs to set
    InitAttrsCapture(param);
    param.sampleRate = attr_.sampleRate;
    param.format = attr_.format;
    param.channelCount = attr_.channel;

    struct AudioDeviceDescriptor deviceDesc;
    deviceDesc.portId = capturePort.portId;
    deviceDesc.pins = PIN_IN_MIC;
    deviceDesc.desc = nullptr;

    ret = audioAdapter_->CreateCapture(audioAdapter_, &deviceDesc, &param, &audioCapture_);
    if (audioCapture_ == nullptr || ret < 0) {
        MEDIA_ERR_LOG("Create capture failed");
        return ERR_NOT_STARTED;
    }

    capturerInited_ = true;

    return 0;
}

int32_t AudioCapturerSource::Init(AudioSourceAttr &attr)
{
    attr_ = attr;
    int32_t ret;
    int32_t index;
    int32_t size = 0;
    struct AudioAdapterDescriptor *descs = nullptr;
    struct AudioPort capturePort;

    if (InitAudioManager() != 0) {
        MEDIA_ERR_LOG("Init audio manager Fail");
        return ERR_INVALID_HANDLE;
    }

    ret = audioManager_->GetAllAdapters(audioManager_, &descs, &size);
    // adapters is 0~3
    if (size > MAX_AUDIO_ADAPTER_NUM || size == 0 || descs == nullptr || ret < 0) {
        MEDIA_ERR_LOG("Get adapters Fail");
        return ERR_NOT_STARTED;
    }

    // Get qualified sound card and port
    char adapterNameCase[PATH_LEN] = "usb";
    index = SwitchAdapterCapture(descs, adapterNameCase, PORT_IN, &capturePort, size);
    if (index < 0) {
        MEDIA_ERR_LOG("Switch Adapter Fail");
        return ERR_NOT_STARTED;
    }

    struct AudioAdapterDescriptor *desc = &descs[index];
    if (audioManager_->LoadAdapter(audioManager_, desc, &audioAdapter_) != 0) {
        MEDIA_ERR_LOG("Load Adapter Fail");
        return ERR_NOT_STARTED;
    }
    if (audioAdapter_ == nullptr) {
        MEDIA_ERR_LOG("Load audio device failed");
        return ERR_NOT_STARTED;
    }

    if (CreateCapture(capturePort) != 0) {
        MEDIA_ERR_LOG("Create capture failed");
        return ERR_NOT_STARTED;
    }
#ifdef CAPTURE_DUMP
    pfd = fopen(g_audioOutTestFilePath, "wb+");
    if (pfd == nullptr) {
        MEDIA_ERR_LOG("Error opening pcm test file!");
    }
#endif // CAPTURE_DUMP

    return SUCCESS;
}

int32_t AudioCapturerSource::CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    int32_t ret;
    if (audioCapture_ == nullptr) {
        MEDIA_ERR_LOG("Audio capture Handle is nullptr!");
        return ERR_INVALID_HANDLE;
    }

    ret = audioCapture_->CaptureFrame(audioCapture_, frame, requestBytes, &replyBytes);
    if (ret < 0) {
        MEDIA_ERR_LOG("Capture Frame Fail");
        return ERR_READ_FAILED;
    }

#ifdef CAPTURE_DUMP
    size_t writeResult = fwrite(frame, replyBytes, 1, pfd);
    if (writeResult != replyBytes) {
        MEDIA_ERR_LOG("Failed to write the file.");
    }
#endif // CAPTURE_DUMP

    return SUCCESS;
}

int32_t AudioCapturerSource::Start(void)
{
    int32_t ret;
    if (!started_ && audioCapture_ != nullptr) {
        ret = audioCapture_->control.Start((AudioHandle)audioCapture_);
        if (ret < 0) {
            return ERR_NOT_STARTED;
        }
        started_ = true;
    }

    return SUCCESS;
}

int32_t AudioCapturerSource::SetVolume(float left, float right)
{
    float volume;
    if (audioCapture_ == nullptr) {
        MEDIA_ERR_LOG("AudioCapturerSource::SetVolume failed audioCapture_ null");
        return ERR_INVALID_HANDLE;
    }

    leftVolume_ = left;
    rightVolume_ = right;
    if ((leftVolume_ == 0) && (rightVolume_ != 0)) {
        volume = rightVolume_;
    } else if ((leftVolume_ != 0) && (rightVolume_ == 0)) {
        volume = leftVolume_;
    } else {
        volume = (leftVolume_ + rightVolume_) / HALF_FACTOR;
    }

    audioCapture_->volume.SetVolume(reinterpret_cast<AudioHandle>(audioCapture_), volume);

    return SUCCESS;
}

int32_t AudioCapturerSource::GetVolume(float &left, float &right)
{
    float val = 0.0;
    audioCapture_->volume.GetVolume((AudioHandle)audioCapture_, &val);
    left = val;
    right = val;

    return SUCCESS;
}

int32_t AudioCapturerSource::SetMute(bool isMute)
{
    int32_t ret;
    if (audioCapture_ == nullptr) {
        MEDIA_ERR_LOG("AudioCapturerSource::SetMute failed audioCapture_ handle is null!");
        return ERR_INVALID_HANDLE;
    }

    ret = audioCapture_->volume.SetMute((AudioHandle)audioCapture_, isMute);
    if (ret != 0) {
        MEDIA_ERR_LOG("AudioCapturerSource::SetMute failed");
        return ERR_OPERATION_FAILED;
    }

    micMuteState_ = isMute;
    return SUCCESS;
}

int32_t AudioCapturerSource::GetMute(bool &isMute)
{
    int32_t ret;
    if (audioCapture_ == nullptr) {
        MEDIA_ERR_LOG("AudioCapturerSource::GetMute failed audioCapture_ handle is null!");
        return ERR_INVALID_HANDLE;
    }

    ret = audioCapture_->volume.GetMute((AudioHandle)audioCapture_, &isMute);
    if (ret != 0) {
        MEDIA_ERR_LOG("AudioCapturerSource::GetMute failed");
        return ERR_OPERATION_FAILED;
    }

    return SUCCESS;
}

int32_t AudioCapturerSource::Stop(void)
{
    int32_t ret;
    if (started_ && audioCapture_ != nullptr) {
        ret = audioCapture_->control.Stop(reinterpret_cast<AudioHandle>(audioCapture_));
        if (ret < 0) {
            MEDIA_ERR_LOG("Stop capture Failed");
            return ERR_OPERATION_FAILED;
        }
    }
    started_ = false;

    return SUCCESS;
}

int32_t AudioCapturerSource::Pause(void)
{
    int32_t ret;
    if (started_ && audioCapture_ != nullptr) {
        ret = audioCapture_->control.Pause(reinterpret_cast<AudioHandle>(audioCapture_));
        if (ret != 0) {
            MEDIA_ERR_LOG("pause capture Failed");
            return ERR_OPERATION_FAILED;
        }
    }
    paused_ = true;

    return SUCCESS;
}

int32_t AudioCapturerSource::Resume(void)
{
    int32_t ret;
    if (paused_ && audioCapture_ != nullptr) {
        ret = audioCapture_->control.Resume(reinterpret_cast<AudioHandle>(audioCapture_));
        if (ret != 0) {
            MEDIA_ERR_LOG("resume capture Failed");
            return ERR_OPERATION_FAILED;
        }
    }
    paused_ = false;

    return SUCCESS;
}

int32_t AudioCapturerSource::Reset(void)
{
    if (started_ && audioCapture_ != nullptr) {
        audioCapture_->control.Flush(reinterpret_cast<AudioHandle>(audioCapture_));
    }

    return SUCCESS;
}

int32_t AudioCapturerSource::Flush(void)
{
    if (started_ && audioCapture_ != nullptr) {
        audioCapture_->control.Flush(reinterpret_cast<AudioHandle>(audioCapture_));
    }

    return SUCCESS;
}
} // namespace AudioStandard
} // namesapce OHOS

#ifdef __cplusplus
extern "C" {
#endif

using namespace OHOS::AudioStandard;

AudioCapturerSource *g_audioCaptureSourceInstance = AudioCapturerSource::GetInstance();

int32_t AudioCapturerSourceInit(AudioSourceAttr *attr)
{
    int32_t ret;

    if (g_audioCaptureSourceInstance->capturerInited_)
        return SUCCESS;

    ret = g_audioCaptureSourceInstance->Init(*attr);

    return ret;
}

void AudioCapturerSourceDeInit()
{
    if (g_audioCaptureSourceInstance->capturerInited_)
        g_audioCaptureSourceInstance->DeInit();
}

int32_t AudioCapturerSourceStop()
{
    int32_t ret;

    if (!g_audioCaptureSourceInstance->capturerInited_)
        return SUCCESS;

    ret = g_audioCaptureSourceInstance->Stop();

    return ret;
}

int32_t AudioCapturerSourceStart()
{
    int32_t ret;

    if (!g_audioCaptureSourceInstance->capturerInited_) {
        MEDIA_ERR_LOG("audioCapturer Not Inited! Init the capturer first\n");
        return ERR_DEVICE_INIT;
    }

    ret = g_audioCaptureSourceInstance->Start();

    return ret;
}

int32_t AudioCapturerSourceFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes)
{
    int32_t ret;

    if (!g_audioCaptureSourceInstance->capturerInited_) {
        MEDIA_ERR_LOG("audioCapturer Not Inited! Init the capturer first\n");
        return ERR_DEVICE_INIT;
    }

    ret = g_audioCaptureSourceInstance->CaptureFrame(frame, requestBytes, replyBytes);

    return ret;
}

int32_t AudioCapturerSourceSetVolume(float left, float right)
{
    int32_t ret;

    if (!g_audioCaptureSourceInstance->capturerInited_) {
        MEDIA_ERR_LOG("audioCapturer Not Inited! Init the capturer first\n");
        return ERR_DEVICE_INIT;
    }

    ret = g_audioCaptureSourceInstance->SetVolume(left, right);

    return ret;
}

int32_t AudioCapturerSourceGetVolume(float *left, float *right)
{
    int32_t ret;

    if (!g_audioCaptureSourceInstance->capturerInited_) {
        MEDIA_ERR_LOG("audioCapturer Not Inited! Init the capturer first\n");
        return ERR_DEVICE_INIT;
    }
    ret = g_audioCaptureSourceInstance->GetVolume(*left, *right);

    return ret;
}

bool AudioCapturerSourceIsMuteRequired(void)
{
    return AudioCapturerSource::micMuteState_;
}

int32_t AudioCapturerSourceSetMute(bool isMute)
{
    int32_t ret;

    if (!g_audioCaptureSourceInstance->capturerInited_) {
        MEDIA_ERR_LOG("audioCapturer Not Inited! Init the capturer first\n");
        return ERR_DEVICE_INIT;
    }

    ret = g_audioCaptureSourceInstance->SetMute(isMute);

    return ret;
}

#ifdef __cplusplus
}
#endif
