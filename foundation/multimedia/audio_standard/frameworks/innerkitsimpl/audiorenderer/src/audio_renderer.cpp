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
#include "audio_renderer.h"
#include "audio_renderer_private.h"
#include "audio_stream.h"

namespace OHOS {
namespace AudioStandard {
AudioRenderer::~AudioRenderer() = default;
AudioRendererPrivate::~AudioRendererPrivate() = default;

std::unique_ptr<AudioRenderer> AudioRenderer::Create(AudioStreamType audioStreamType)
{
    return std::make_unique<AudioRendererPrivate>(audioStreamType);
}

AudioRendererPrivate::AudioRendererPrivate(AudioStreamType audioStreamType)
{
    audioRenderer = std::make_unique<AudioStream>(audioStreamType, AUDIO_MODE_PLAYBACK);
}

int32_t AudioRendererPrivate::GetFrameCount(uint32_t &frameCount) const
{
    return audioRenderer->GetFrameCount(frameCount);
}

int32_t AudioRendererPrivate::GetLatency(uint64_t &latency) const
{
    return audioRenderer->GetLatency(latency);
}

int32_t AudioRendererPrivate::SetParams(const AudioRendererParams params) const
{
    AudioStreamParams audioStreamParams;
    audioStreamParams.format = params.sampleFormat;
    audioStreamParams.samplingRate = params.sampleRate;
    audioStreamParams.channels = params.channelCount;
    audioStreamParams.encoding = params.encodingType;

    return audioRenderer->SetAudioStreamInfo(audioStreamParams);
}

int32_t AudioRendererPrivate::GetParams(AudioRendererParams &params) const
{
    AudioStreamParams audioStreamParams;
    int32_t result = audioRenderer->GetAudioStreamInfo(audioStreamParams);
    if (!result) {
        params.sampleFormat = static_cast<AudioSampleFormat>(audioStreamParams.format);
        params.sampleRate = static_cast<AudioSamplingRate>(audioStreamParams.samplingRate);
        params.channelCount = static_cast<AudioChannel>(audioStreamParams.channels);
        params.encodingType = static_cast<AudioEncodingType>(audioStreamParams.encoding);
    }

    return result;
}

bool AudioRendererPrivate::Start() const
{
    return audioRenderer->StartAudioStream();
}

int32_t AudioRendererPrivate::Write(uint8_t *buffer, size_t bufferSize)
{
    return audioRenderer->Write(buffer, bufferSize);
}

RendererState AudioRendererPrivate::GetStatus() const
{
    return static_cast<RendererState>(audioRenderer->GetState());
}

bool AudioRendererPrivate::GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const
{
    return audioRenderer->GetAudioTime(timestamp, base);
}

bool AudioRendererPrivate::Drain() const
{
    return audioRenderer->DrainAudioStream();
}

bool AudioRendererPrivate::Flush() const
{
    return audioRenderer->FlushAudioStream();
}

bool AudioRendererPrivate::Pause() const
{
    return audioRenderer->PauseAudioStream();
}

bool AudioRendererPrivate::Stop() const
{
    return audioRenderer->StopAudioStream();
}

bool AudioRendererPrivate::Release() const
{
    return audioRenderer->ReleaseAudioStream();
}

int32_t AudioRendererPrivate::GetBufferSize(size_t &bufferSize) const
{
    return audioRenderer->GetBufferSize(bufferSize);
}

int32_t AudioRendererPrivate::SetVolume(float volume) const
{
    return audioRenderer->SetVolume(volume);
}

float AudioRendererPrivate::GetVolume() const
{
    return audioRenderer->GetVolume();
}

std::vector<AudioSampleFormat> AudioRenderer::GetSupportedFormats()
{
    return AUDIO_SUPPORTED_FORMATS;
}

std::vector<AudioSamplingRate> AudioRenderer::GetSupportedSamplingRates()
{
    return AUDIO_SUPPORTED_SAMPLING_RATES;
}

std::vector<AudioChannel> AudioRenderer::GetSupportedChannels()
{
    return AUDIO_SUPPORTED_CHANNELS;
}

std::vector<AudioEncodingType> AudioRenderer::GetSupportedEncodingTypes()
{
    return AUDIO_SUPPORTED_ENCODING_TYPES;
}
}  // namespace AudioStandard
}  // namespace OHOS
