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

#ifndef AUDIO_CAPTURER_H
#define AUDIO_CAPTURER_H

#include <memory>

#include "audio_info.h"
#include "timestamp.h"

namespace OHOS {
namespace AudioStandard {
/**
 * @brief Defines information about audio capturer parameters
 */
struct AudioCapturerParams {
    /** Audio source type */
    AudioSourceType inputSource = AUDIO_MIC;
    /** Audio codec format */
    AudioEncodingType audioEncoding = ENCODING_PCM;
    /** Sampling rate */
    AudioSamplingRate samplingRate = SAMPLE_RATE_44100;
    /** Number of audio channels */
    AudioChannel audioChannel = MONO;
    /** Audio stream type */
    AudioStreamType streamType = STREAM_MEDIA;
    /** audioSampleFormat */
    AudioSampleFormat audioSampleFormat = SAMPLE_S16LE;
};

/**
 * @brief Enumerates the capturing states of the current device.
 */
enum CapturerState {
    /** Capturer INVALID state */
    CAPTURER_INVALID = -1,
    /** Create new capturer instance */
    CAPTURER_NEW,
    /** Capturer Prepared state */
    CAPTURER_PREPARED,
    /** Capturer Running state */
    CAPTURER_RUNNING,
    /** Capturer Stopped state */
    CAPTURER_STOPPED,
    /** Capturer Released state */
    CAPTURER_RELEASED
};

/**
 * @brief Provides functions for applications to implement audio capturing.
 */
class AudioCapturer {
public:
    /**
     * @brief creater capturer instance.
    */
    static std::unique_ptr<AudioCapturer> Create(AudioStreamType audioStreamType);

    /**
     * @brief Obtains the number of frames required in the current condition, in bytes per sample.
     *
     * @param frameCount Indicates the pointer in which framecount will be written
     * @return Returns {@link SUCCESS} if frameCount is successfully obtained; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t GetFrameCount(uint32_t &frameCount) const = 0;

    /**
     * @brief Sets audio capture parameters.
     *
     * @param params Indicates information about audio capture parameters to set. For details, see
     * {@link AudioCapturerParams}.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     */
    virtual int32_t SetParams(const AudioCapturerParams params) const = 0;

    /**
     * @brief Obtains audio capturer parameters.
     *
     * This function can be called after {@link SetParams} is successful.
     *
     * @param params Indicates information about audio capturer parameters.For details,see
     * {@link AudioCapturerParams}.
     * @return Returns {@link SUCCESS} if the parameter information is successfully obtained; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t GetParams(AudioCapturerParams &params) const = 0;

    /**
     * @brief Starts audio capturing.
     *
     * @return Returns <b>true</b> if the capturing is successfully started; returns <b>false</b> otherwise.
     */
    virtual bool Start() const = 0;

    /**
     * @brief capture audio data.
     *
     * @param buffer Indicates the pointer to the buffer into which the audio data is to be written.
     * @param userSize Indicates the size of the buffer into which the audio data is to be written, in bytes.
     * <b>userSize >= frameCount * channelCount * BytesPerSample</b> must evaluate to <b>true</b>. You can call
     * {@link GetFrameCount} to obtain the <b>frameCount</b> value.
     * @param isBlockingRead Specifies whether data reading will be blocked.
     * @return Returns the size of the audio data read from the device. The value ranges from <b>0</b> to
     * <b>userSize</b>. If the reading fails, one of the following error codes is returned.
     * <b>ERR_INVALID_PARAM</b>: The input parameter is incorrect.
     * <b>ERR_ILLEGAL_STATE</b>: The <b>AudioCapturer</b> instance is not initialized.
     * <b>ERR_INVALID_READ</b>: The read size < 0.
     */
    virtual int32_t Read(uint8_t &buffer, size_t userSize, bool isBlockingRead) const = 0;

    /**
     * @brief Obtains the audio capture state.
     *
     * @return Returns the audio capture state defined in {@link CapturerState}.
     */
    virtual CapturerState GetStatus() const = 0;

    /**
     * @brief Obtains the Timestamp.
     *
     * @param timestamp Indicates a {@link Timestamp} instance reference provided by the caller.
     * @param base Indicates the time base, which can be {@link Timestamp.Timestampbase#BOOTTIME} or
     * {@link Timestamp.Timestampbase#MONOTONIC}.
     * @return Returns <b>true</b> if the timestamp is successfully obtained; returns <b>false</b> otherwise.
     */
    virtual bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const = 0;

    /**
     * @brief Stops audio capturing.
     *
     * @return Returns <b>true</b> if the capturing is successfully stopped; returns <b>false</b> otherwise.
     */
    virtual bool Stop() const = 0;
    /**
     * @brief flush capture stream.
     *
     * @return Returns <b>true</b> if the object is successfully flushed; returns <b>false</b> otherwise.
     */
    virtual bool Flush() const = 0;

    /**
     * @brief Releases a local <b>AudioCapturer</b> object.
     *
     * @return Returns <b>true</b> if the object is successfully released; returns <b>false</b> otherwise.
     */
    virtual bool Release() const = 0;

    /**
     * @brief Obtains a reasonable minimum buffer size for capturer, however, the capturer can
     *        accept other read sizes as well.
     *
     * @param bufferSize Indicates a buffersize pointer value that wil be written.
     * @return Returns {@link SUCCESS} if bufferSize is successfully obtained; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t GetBufferSize(size_t &bufferSize) const = 0;

    /**
     * @brief Obtains the capturer supported formats.
     *
     * @return vector with capturer supported formats.
     */
    static std::vector<AudioSampleFormat> GetSupportedFormats();

    /**
     * @brief Obtains the capturer supported channels.
     *
     * @return vector with capturer supported channels.
     */
    static std::vector<AudioChannel> GetSupportedChannels();

    /**
     * @brief Obtains the capturer supported encoding types.
     *
     * @return vector with capturer supported encoding types.
     */
    static std::vector<AudioEncodingType> GetSupportedEncodingTypes();

    /**
     * @brief Obtains the capturer supported SupportedSamplingRates.
     *
     * @return vector with capturer supported SupportedSamplingRates.
     */
    static std::vector<AudioSamplingRate> GetSupportedSamplingRates();

    virtual ~AudioCapturer();
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_CAPTURER_H
