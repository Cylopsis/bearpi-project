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

#include "ring.h"

#include <thread>

#include "audio_control_manager.h"

#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
bool Ring::keepRinging_ = false;
#ifdef ABILITY_AUDIO_SUPPORT
std::unique_ptr<AudioServiceClient> Ring::ringtonePlayer_ = nullptr;
#endif
std::unique_ptr<RingtoneStream> Ring::ringtoneStream_ = nullptr;

Ring::Ring()
    : isVibrating_(false), shouldRing_(false), shouldVibrate_(false), loopNumber_(DEFAULT_RING_LOOP_NUMBER),
      speed_(DEFAULT_RING_SPEED)
{
    Init(DelayedSingleton<AudioProxy>::GetInstance()->GetDefaultRingerPath());
}

Ring::Ring(const std::string &path)
    : isVibrating_(false), shouldRing_(false), shouldVibrate_(false), loopNumber_(DEFAULT_RING_LOOP_NUMBER),
      speed_(DEFAULT_RING_SPEED)
{
    Init(path);
}

Ring::~Ring()
{
    Release();
}

void Ring::Init(const std::string &ringtonePath)
{
#ifdef ABILITY_AUDIO_SUPPORT
    if (AudioRingerMode::RINGER_MODE_NORMAL == DelayedSingleton<AudioProxy>::GetInstance()->GetRingerMode()) {
        shouldRing_ = true;
        shouldVibrate_ = true;
    } else if (AudioRingerMode::RINGER_MODE_VIBRATE ==
        DelayedSingleton<AudioProxy>::GetInstance()->GetRingerMode()) {
        shouldRing_ = false;
        shouldVibrate_ = true;
    } else if (AudioRingerMode::RINGER_MODE_SILENT == DelayedSingleton<AudioProxy>::GetInstance()->GetRingerMode()) {
        shouldRing_ = false;
        shouldVibrate_ = false;
    }
    ringtoneStream_ = std::make_unique<RingtoneStream>();
    if (ringtoneStream_ == nullptr) {
        return;
    }
    ringtoneStream_->ringtonePath = ringtonePath;
    ringtoneStream_->playedPosition = 0;
    volume_ = DelayedSingleton<AudioProxy>::GetInstance()->GetVolume(
        AudioStandard::AudioSystemManager::AudioVolumeType::STREAM_RING);
    ringtonePlayer_ = std::make_unique<AudioServiceClient>();
    if (ringtonePlayer_ == nullptr || ringtonePlayer_->Initialize(AUDIO_SERVICE_CLIENT_PLAYBACK) < 0) {
        return;
    }
    AudioStreamParams audioParams;
    audioParams.format = DEFAULT_FORMAT;
    audioParams.channels = DEFAULT_CHANNELS;
    if (ringtonePlayer_->CreateStream(audioParams, STREAM_RING) < 0) {
        TELEPHONY_LOGE("create stream error");
    }
#endif
}

int32_t Ring::Start()
{
    if (!shouldRing_ || ringtoneStream_ == nullptr || ringtoneStream_->ringtonePath.empty()) {
        return TELEPHONY_FAIL;
    }
    int32_t result = TELEPHONY_FAIL;
    keepRinging_ = true;
    std::thread ringtoneThread(PlayRingtoneStream, ringtoneStream_->ringtonePath, ringtoneStream_->playedPosition);
    ringtoneThread.detach();
    if (shouldVibrate_) {
        result = StartVibrate();
    }
    return result;
}

int32_t Ring::PlayRingtoneStream(const std::string &path, int32_t offset)
{
#ifdef ABILITY_AUDIO_SUPPORT
    FILE *file = fopen(path.c_str(), "rb");
    if (file == nullptr) {
        return TELEPHONY_FAIL;
    }
    if (fseek(file, offset, SEEK_SET) < 0 || ringtonePlayer_ == nullptr) {
        return TELEPHONY_FAIL;
    }
    size_t bufferLen;
    if (ringtonePlayer_->GetMinimumBufferSize(bufferLen) < 0 || ringtonePlayer_->StartStream() < 0) {
        return TELEPHONY_FAIL;
    }
    uint8_t *buffer = nullptr;
    buffer = (uint8_t *)malloc(bufferLen + bufferLen);
    StreamBuffer stream;
    size_t bytesToWrite = 0;
    size_t bytesWritten = 0;
    size_t minBytes = MIN_BYTES;
    int32_t writeError;
    while (!feof(file) && keepRinging_) {
        bytesToWrite = fread(buffer, ELEMENT_SIZE, bufferLen, file);
        bytesWritten = 0;
        while ((bytesWritten < bytesToWrite) && ((bytesToWrite - bytesWritten) > minBytes)) {
            stream.buffer = buffer + bytesWritten;
            stream.bufferLen = bytesToWrite - bytesWritten;
            bytesWritten += ringtonePlayer_->WriteStream(stream, writeError);
        }
    }
    ringtonePlayer_->FlushStream();
    ringtonePlayer_->StopStream();
    ringtonePlayer_->ReleaseStream();
    free(buffer);
    int32_t ret = fclose(file);
    if (ret != TELEPHONY_SUCCESS) {
        return TELEPHONY_FAIL;
    }
    ringtoneStream_->playedPosition = bytesWritten;
    return TELEPHONY_SUCCESS;
#endif
    return TELEPHONY_SUCCESS;
}

int32_t Ring::Stop()
{
    int32_t result = TELEPHONY_FAIL;
    if (keepRinging_) {
        keepRinging_ = false;
    }
    if (isVibrating_) {
        result = CancelVibrate();
    }
    return result;
}

int32_t Ring::Release()
{
    return TELEPHONY_SUCCESS;
}

int32_t Ring::Resume()
{
    if (keepRinging_) {
        return TELEPHONY_SUCCESS;
    }
    if (ringtoneStream_ == nullptr || ringtoneStream_->ringtonePath.empty()) {
        return TELEPHONY_FAIL;
    }
    std::thread ringtoneThread(PlayRingtoneStream, ringtoneStream_->ringtonePath, ringtoneStream_->playedPosition);
    ringtoneThread.detach();
    return TELEPHONY_SUCCESS;
}

int32_t Ring::Pause()
{
    if (keepRinging_) {
        keepRinging_ = false;
    }
    return TELEPHONY_SUCCESS;
}

int32_t Ring::StartVibrate()
{
    if (DelayedSingleton<AudioProxy>::GetInstance()->StartVibrate() == TELEPHONY_SUCCESS) {
        isVibrating_ = true;
        return TELEPHONY_SUCCESS;
    }
    return TELEPHONY_FAIL;
}

int32_t Ring::CancelVibrate()
{
    if (DelayedSingleton<AudioProxy>::GetInstance()->CancelVibrate() == TELEPHONY_SUCCESS) {
        isVibrating_ = false;
        return TELEPHONY_SUCCESS;
    }
    return TELEPHONY_FAIL;
}

void Ring::SetLoop(uint32_t number)
{
    loopNumber_ = number;
}

uint32_t Ring::GetLoop()
{
    return loopNumber_;
}

void Ring::SetVolume(float volume)
{
    volume_ = volume;
}

float Ring::GetVolume()
{
    return volume_;
}

void Ring::SetSpeed(double speed)
{
    speed_ = speed;
}

double Ring::GetSpeed()
{
    return speed_;
}

bool Ring::ShouldVibrate()
{
#ifdef ABILITY_AUDIO_SUPPORT
    return DelayedSingleton<AudioProxy>::GetInstance()->GetRingerMode() != AudioRingerMode::RINGER_MODE_SILENT;
#endif
    return false;
}
} // namespace Telephony
} // namespace OHOS