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
#ifndef AUDIO_STREAM_H
#define AUDIO_STREAM_H

#include "audio_session.h"
#include "timestamp.h"

namespace OHOS {
namespace AudioStandard {
enum AudioMode {
    AUDIO_MODE_PLAYBACK,
    AUDIO_MODE_RECORD
};

/**
 * @brief Enumerates the stream states of the current device.
 *
 * @since 1.0
 * @version 1.0
 */
enum State {
    /** INVALID */
    INVALID = -1,
    /** New */
    NEW,
    /** Prepared */
    PREPARED,
    /** Running */
    RUNNING,
    /** Stopped */
    STOPPED,
    /** Released */
    RELEASED,
    /** Paused */
    PAUSED
};

class AudioStream : public AudioSession {
public:
    AudioStream(AudioStreamType eStreamType, AudioMode eMode);
    virtual ~AudioStream();

    int32_t SetAudioStreamInfo(const AudioStreamParams info);
    int32_t GetAudioStreamInfo(AudioStreamParams &info);

    uint32_t GetAudioSessionID();
    State GetState();
    bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base);
    int32_t GetBufferSize(size_t &bufferSize);
    int32_t GetFrameCount(uint32_t &frameCount);
    int32_t GetLatency(uint64_t &latency);
    int32_t SetVolume(float volume);
    float GetVolume();

    std::vector<AudioSampleFormat> GetSupportedFormats();
    std::vector<AudioChannel> GetSupportedChannels();
    std::vector<AudioEncodingType> GetSupportedEncodingTypes();
    std::vector<AudioSamplingRate> GetSupportedSamplingRates();

    // Common APIs
    bool StartAudioStream();
    bool PauseAudioStream();
    bool StopAudioStream();
    bool ReleaseAudioStream();
    bool FlushAudioStream();

    // Playback related APIs
    bool DrainAudioStream();
    size_t Write(uint8_t *buffer, size_t buffer_size);

    // Recording related APIs
    int32_t Read(uint8_t &buffer, size_t userSize, bool isBlockingRead);
private:
    AudioStreamType eStreamType_;
    AudioMode eMode_;
    State state_;
    std::atomic<bool> isReadInProgress_;
    std::atomic<bool> isWriteInProgress_;
    uint64_t resetTimestamp_;
    struct timespec baseTimestamp_;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_STREAM_H
