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

#include <vector>

#include "audio_errors.h"
#include "audio_capturer.h"
#include "audio_capturer_private.h"
#include "audio_stream.h"

namespace OHOS {
namespace AudioStandard {
AudioCapturer::~AudioCapturer() = default;
AudioCapturerPrivate::~AudioCapturerPrivate() = default;

std::unique_ptr<AudioCapturer> AudioCapturer::Create(AudioStreamType audioStreamType)
{
    return std::make_unique<AudioCapturerPrivate>(audioStreamType);
}

AudioCapturerPrivate::AudioCapturerPrivate(AudioStreamType audioStreamType)
{
    audioCapturer = std::make_unique<AudioStream>(audioStreamType, AUDIO_MODE_RECORD);
}

int32_t AudioCapturerPrivate::GetFrameCount(uint32_t &frameCount) const
{
    return audioCapturer->GetFrameCount(frameCount);
}

int32_t AudioCapturerPrivate::SetParams(const AudioCapturerParams params) const
{
    AudioStreamParams audioStreamParams;
    audioStreamParams.format = params.audioSampleFormat;
    audioStreamParams.samplingRate = params.samplingRate;
    audioStreamParams.channels = params.audioChannel;
    audioStreamParams.encoding = params.audioEncoding;

    return audioCapturer->SetAudioStreamInfo(audioStreamParams);
}

int32_t AudioCapturerPrivate::GetParams(AudioCapturerParams &params) const
{
    AudioStreamParams audioStreamParams;
    int32_t result = audioCapturer->GetAudioStreamInfo(audioStreamParams);
    if (SUCCESS == result) {
        params.audioSampleFormat = static_cast<AudioSampleFormat>(audioStreamParams.format);
        params.samplingRate =  static_cast<AudioSamplingRate>(audioStreamParams.samplingRate);
        params.audioChannel = static_cast<AudioChannel>(audioStreamParams.channels);
        params.audioEncoding = static_cast<AudioEncodingType>(audioStreamParams.encoding);
    }

    return result;
}

bool AudioCapturerPrivate::Start() const
{
    return audioCapturer->StartAudioStream();
}

int32_t AudioCapturerPrivate::Read(uint8_t &buffer, size_t userSize, bool isBlockingRead) const
{
    return audioCapturer->Read(buffer, userSize, isBlockingRead);
}

CapturerState AudioCapturerPrivate::GetStatus() const
{
    return (CapturerState)audioCapturer->GetState();
}

bool AudioCapturerPrivate::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    return audioCapturer->GetAudioTime(timestamp, base);
}

bool AudioCapturerPrivate::Stop() const
{
    return audioCapturer->StopAudioStream();
}

bool AudioCapturerPrivate::Flush() const
{
    return audioCapturer->FlushAudioStream();
}

bool AudioCapturerPrivate::Release() const
{
    return audioCapturer->ReleaseAudioStream();
}

int32_t AudioCapturerPrivate::GetBufferSize(size_t &bufferSize) const
{
    return audioCapturer->GetBufferSize(bufferSize);
}

std::vector<AudioSampleFormat> AudioCapturer::GetSupportedFormats()
{
    return AUDIO_SUPPORTED_FORMATS;
}

std::vector<AudioChannel> AudioCapturer::GetSupportedChannels()
{
    return AUDIO_SUPPORTED_CHANNELS;
}

std::vector<AudioEncodingType> AudioCapturer::GetSupportedEncodingTypes()
{
    return AUDIO_SUPPORTED_ENCODING_TYPES;
}

std::vector<AudioSamplingRate> AudioCapturer::GetSupportedSamplingRates()
{
    return AUDIO_SUPPORTED_SAMPLING_RATES;
}
}  // namespace AudioStandard
}  // namespace OHOS