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

#ifndef AUDIO_CAPTURER_PRIVATE_H
#define AUDIO_CAPTURER_PRIVATE_H

#include "audio_capturer.h"
#include "audio_stream.h"

namespace OHOS {
namespace AudioStandard {
class AudioCapturerPrivate : public AudioCapturer {
public:
    int32_t GetFrameCount(uint32_t &frameCount) const override;
    int32_t SetParams(const AudioCapturerParams params) const override;
    int32_t GetParams(AudioCapturerParams &params) const override;
    bool Start() const override;
    int32_t  Read(uint8_t &buffer, size_t userSize, bool isBlockingRead) const override;
    CapturerState GetStatus() const override;
    bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const override;
    bool Stop() const override;
    bool Flush() const override;
    bool Release() const override;
    int32_t GetBufferSize(size_t &bufferSize) const override;

    std::unique_ptr<AudioStream> audioCapturer;

    explicit AudioCapturerPrivate(AudioStreamType audioStreamType);
    virtual ~AudioCapturerPrivate();
};
}  // namespace AudioStandard
}  // namespace OHOS

#endif // AUDIO_CAPTURER_PRIVATE_H
