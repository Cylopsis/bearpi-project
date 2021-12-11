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

#ifndef AUDIO_RENDERER_PRIVATE_H
#define AUDIO_RENDERER_PRIVATE_H

#include "audio_renderer.h"
#include "audio_stream.h"

namespace OHOS {
namespace AudioStandard {
class AudioRendererPrivate : public AudioRenderer {
public:
    int32_t GetFrameCount(uint32_t &frameCount) const override;
    int32_t GetLatency(uint64_t &latency) const override;
    int32_t SetParams(const AudioRendererParams params) const  override;
    int32_t GetParams(AudioRendererParams &params) const override;
    bool Start() const override;
    int32_t Write(uint8_t *buffer, size_t bufferSize) override;
    RendererState GetStatus() const override;
    bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const override;
    bool Drain() const override;
    bool Pause() const override;
    bool Stop() const override;
    bool Flush() const override;
    bool Release() const override;
    int32_t GetBufferSize(size_t &bufferSize) const override;
    int32_t SetVolume(float volume) const override;
    float GetVolume() const override;

    std::unique_ptr<AudioStream> audioRenderer;

    explicit AudioRendererPrivate(AudioStreamType audioStreamType);
    virtual ~AudioRendererPrivate();
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_RENDERER_PRIVATE_H
