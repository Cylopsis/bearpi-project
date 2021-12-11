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

#ifndef AUDIO_RENDERER_H
#define AUDIO_RENDERER_H

#include<memory>

#include "audio_info.h"
#include "timestamp.h"

namespace OHOS {
namespace AudioStandard {
/**
 * @brief Defines information about audio renderer parameters.
 */

struct AudioRendererParams {
    /** Sample Format */
    AudioSampleFormat sampleFormat = SAMPLE_S16LE;
    /** Sampling rate */
    AudioSamplingRate sampleRate = SAMPLE_RATE_8000;
    /** Number of channels */
    AudioChannel channelCount = MONO;
    /** Encoding Type */
    AudioEncodingType encodingType = ENCODING_PCM;
};

/**
 * @brief Enumerates the rendering states of the current device.
 */
enum RendererState {
    /** INVALID state */
    RENDERER_INVALID = -1,
    /** Create New Renderer instance */
    RENDERER_NEW,
    /** Reneder Prepared state */
    RENDERER_PREPARED,
    /** Rendere Running state */
    RENDERER_RUNNING,
    /** Renderer Stopped state */
    RENDERER_STOPPED,
    /** Renderer Released state */
    RENDERER_RELEASED,
    /** Renderer Paused state */
    RENDERER_PAUSED
};

/**
 * @brief Provides functions for applications to implement audio rendering.
 */
class AudioRenderer {
public:
    /**
     * @brief creater renderer instance.
     *
     * @param audioStreamType The audio streamtype to be created.
     * refer AudioStreamType in audio_info.h.
     * @return Returns unique pointer to the AudioRenderer object
    */
    static std::unique_ptr<AudioRenderer> Create(AudioStreamType audioStreamType);
    /**
     * @brief Obtains the number of frames required in the current condition, in bytes per sample.
     *
     * @param frameCount Indicates the reference variable in which framecount will be written
     * @return Returns {@link SUCCESS} if frameCount is successfully obtained; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t GetFrameCount(uint32_t &frameCount) const = 0;

    /**
     * @brief Sets audio renderer parameters.
     *
     * @param params Indicates information about audio renderer parameters to set. For details, see
     * {@link AudioRendererParams}.
     * @return Returns {@link SUCCESS} if the setting is successful; returns an error code defined
     * in {@link audio_errors.h} otherwise.
     */
    virtual int32_t SetParams(const AudioRendererParams params) const = 0;

    /**
     * @brief Obtains audio renderer parameters.
     *
     * This function can be called after {@link SetParams} is successful.
     *
     * @param params Indicates information about audio renderer parameters. For details, see
     * {@link AudioRendererParams}.
     * @return Returns {@link SUCCESS} if the parameter information is successfully obtained; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t GetParams(AudioRendererParams &params) const = 0;

    /**
     * @brief Starts audio rendering.
     *
     * @return Returns <b>true</b> if the rendering is successfully started; returns <b>false</b> otherwise.
     */
    virtual bool Start() const = 0;

    /**
     * @brief Writes audio data.
     *
     * @param buffer Indicates the pointer to the buffer which contains the audio data to be written.
     * @param bufferSize Indicates the size of the buffer which contains audio data to be written, in bytes.
     * @return Returns the size of the audio data written to the device. The value ranges from <b>0</b> to
     * <b>bufferSize</b>. If the write fails, one of the following error codes is returned.
     * <b>ERR_INVALID_PARAM</b>: The input parameter is incorrect.
     * <b>ERR_ILLEGAL_STATE</b>: The <b>AudioRenderer</b> instance is not initialized.
     * <b>ERR_INVALID_WRITE</b>: The written audio data size is < 0.
     * <b>ERR_WRITE_FAILED</b>: The audio data write failed .
     */
    virtual int32_t Write(uint8_t *buffer, size_t bufferSize) = 0;

    /**
     * @brief Obtains the audio renderer state.
     *
     * @return Returns the audio renderer state defined in {@link RendererState}.
     */
    virtual RendererState GetStatus() const = 0;

    /**
     * @brief Obtains the timestamp.
     *
     * @param timestamp Indicates a {@link Timestamp} instance reference provided by the caller.
     * @param base Indicates the time base, which can be {@link Timestamp.Timestampbase#BOOTTIME} or
     * {@link Timestamp.Timestampbase#MONOTONIC}.
     * @return Returns <b>true</b> if the timestamp is successfully obtained; returns <b>false</b> otherwise.
     */
    virtual bool GetAudioTime(Timestamp &timestamp, Timestamp::Timestampbase base) const = 0;

    /**
     * @brief Obtains the latency in microseconds.
     *
     * @param latency Indicates the reference variable into which latency value will be written.
     * @return Returns {@link SUCCESS} if latency is successfully obtained, returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t GetLatency(uint64_t &latency) const = 0;

    /**
     * @brief drain renderer buffer.
     *
     * @return Returns <b>true</b> if the buffer is successfully drained; returns <b>false</b> otherwise.
     */
    virtual bool Drain() const = 0;

    /**
     * @brief flush renderer stream.
     *
     * @return Returns <b>true</b> if the object is successfully flushed; returns <b>false</b> otherwise.
     */
    virtual bool Flush() const = 0;

    /**
     * @brief Pauses audio rendering.
     *
     * @return Returns <b>true</b> if the rendering is successfully Paused; returns <b>false</b> otherwise.
     */
    virtual bool Pause() const = 0;

    /**
     * @brief Stops audio rendering.
     *
     * @return Returns <b>true</b> if the rendering is successfully stopped; returns <b>false</b> otherwise.
     */
    virtual bool Stop() const = 0;

    /**
     * @brief Releases a local <b>AudioRenderer</b> object.
     *
     * @return Returns <b>true</b> if the object is successfully released; returns <b>false</b> otherwise.
     */
    virtual bool Release() const = 0;

    /**
     * @brief Obtains a reasonable minimum buffer size for rendering, however, the renderer can
     *        accept other write sizes as well.
     *
     * @param bufferSize Indicates the reference variable into which buffer size value wil be written.
     * @return Returns {@link SUCCESS} if bufferSize is successfully obtained; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t GetBufferSize(size_t &bufferSize) const = 0;

    /**
     * @brief Set the track volume
     *
     * @param volume The volume to be set for the current track.
     * @return Returns {@link SUCCESS} if volume is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    virtual int32_t SetVolume(float volume) const = 0;

    /**
     * @brief Obtains the current track volume
     *
     * @return Returns current track volume
     */
    virtual float GetVolume() const = 0;

        /**
     * @brief Obtains the foramts supported by renderer.
     *
     * @return Returns vector with supported formats.
     */
    static std::vector<AudioSampleFormat> GetSupportedFormats();

    /**
     * @brief Obtains the SupportedSamplingRates supported by renderer.
     *
     * @return Returns vector with supported SupportedSamplingRates.
     */
    static std::vector<AudioSamplingRate> GetSupportedSamplingRates();

    /**
     * @brief Obtains the channels supported by renderer.
     *
     * @return Returns vector with supported channels.
     */
    static std::vector<AudioChannel> GetSupportedChannels();

    /**
     * @brief Obtains the encoding types supported by renderer.
     *
     * @return Returns vector with supported encoding types.
     */
    static std::vector<AudioEncodingType> GetSupportedEncodingTypes();

    virtual ~AudioRenderer();
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_RENDERER_H
