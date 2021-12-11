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

#ifndef AUDIO_SERVICE_CLIENT_H
#define AUDIO_SERVICE_CLIENT_H

#include <map>
#include <iostream>
#include <cstring>
#include <stdlib.h>
#include <unistd.h>
#include <pulse/pulseaudio.h>
#include <pulse/thread-mainloop.h>
#include <audio_info.h>
#include <audio_error.h>

#include "audio_system_manager.h"

namespace OHOS {
namespace AudioStandard {
enum ASClientType {
    AUDIO_SERVICE_CLIENT_PLAYBACK,
    AUDIO_SERVICE_CLIENT_RECORD,
    AUDIO_SERVICE_CLIENT_CONTROLLER
};

typedef pa_sink_input_info    SinkInputInfo;
typedef pa_source_output_info SourceOutputInfo;
typedef pa_sink_info          SinkDeviceInfo;
typedef pa_source_info        SourceDeviceInfo;
typedef pa_client_info        ClientInfo;

struct StreamBuffer {
    uint8_t *buffer; // the virtual address of stream
    uint32_t bufferLen; // stream length in bytes
};

struct AudioCache {
    std::unique_ptr<uint8_t[]> buffer;
    uint32_t readIndex;
    uint32_t writeIndex;
    uint32_t totalCacheSize;
    bool isFull;
};

class AudioRendererCallbacks {
public:
    virtual ~AudioRendererCallbacks();
    virtual void OnSinkDeviceUpdatedCb() const = 0;
    // Need to check required state changes to update applications
    virtual void OnStreamStateChangeCb() const = 0;
    virtual void OnStreamBufferUnderFlowCb() const = 0;
    virtual void OnStreamBufferOverFlowCb() const = 0;
    virtual void OnErrorCb(AudioServiceErrorCodes error) const = 0;
    virtual void OnEventCb(AudioServiceEventTypes error) const = 0;
};

class AudioCapturerCallbacks {
public:
    virtual ~AudioCapturerCallbacks();
    virtual void OnSourceDeviceUpdatedCb() const = 0;
    // Need to check required state changes to update applications
    virtual void OnStreamStateChangeCb() const = 0;
    virtual void OnStreamBufferUnderFlowCb() const = 0;
    virtual void OnStreamBufferOverFlowCb() const = 0;
    virtual void OnErrorCb(AudioServiceErrorCodes error) const = 0;
    virtual void OnEventCb(AudioServiceEventTypes error) const = 0;
};

class AudioServiceClient {
public:
    AudioServiceClient();
    virtual ~AudioServiceClient();

    /**
    * Initializes audio service client for the required client type
    *
    * @param eClientType indicates the client type like playback, record or controller.
    * @return Returns {@code 0} if success; returns {@code -1} otherwise.
    */
    int32_t Initialize(ASClientType eClientType);

    // Stream handling APIs

    /**
    * Creates & initializes resources based on the audioParams and audioType
    *
    * @param audioParams indicate format, sampling rate and number of channels
    * @param audioType indicate the stream type like music, system, ringtone etc
    * @return Returns {@code 0} if success; returns {@code -1} otherwise.
    */
    int32_t CreateStream(AudioStreamParams audioParams, AudioStreamType audioType);

    /**
    * Starts the stream created using CreateStream
    *
    * @return Returns {@code 0} if success; returns {@code -1} otherwise.
    */
    int32_t StartStream();

    /**
    * Stops the stream created using CreateStream
    *
    * @return Returns {@code 0} if success; returns {@code -1} otherwise.
    */
    int32_t StopStream();

    /**
    * Flushes the stream created using CreateStream. This is applicable for
    * playback only
    *
    * @return Returns {@code 0} if success; returns {@code -1} otherwise.
    */
    int32_t FlushStream();

    /**
    * Drains the stream created using CreateStream. This is applicable for
    * playback only
    *
    * @return Returns {@code 0} if success; returns {@code -1} otherwise.
    */
    int32_t DrainStream();

    /**
    * Pauses the stream
    *
    * @return Returns {@code 0} if success; returns {@code -1} otherwise.
    */
    int32_t PauseStream();

    /**
    * Sets the volume of the stream associated with session ID
    *
    * @param sessionID indicates the ID for the active stream to be controlled
    * @param volume indicates volume level between 0 to 65536
    * @return Returns {@code 0} if success; returns {@code -1} otherwise.
    */
    int32_t SetStreamVolume(uint32_t sessionID, uint32_t volume);

    /**
    * Get the volume of the stream associated with session ID
    *
    * @param sessionID indicates the ID for the active stream to be controlled
    * @return returns volume level between 0 to 65536
    */
    uint32_t GetStreamVolume(uint32_t sessionID);

    /**
    * Writes audio data of the stream created using CreateStream to active sink device
    *
    * @param buffer contains audio data to write
    * @param bufferSize indicates the size of audio data in bytes to write from the buffer
    * @param pError indicates pointer to error which will be filled in case of internal errors
    * @return returns size of audio data written in bytes.
    */
    size_t WriteStream(const StreamBuffer &stream, int32_t &pError);

    /**
    * Reads audio data of the stream created using CreateStream from active source device
    *
    * @param StreamBuffer including buffer to be filled with audio data
    * and bufferSize indicating the size of audio data to read into buffer
    * @param isBlocking indicates if the read is blocking or not
    * @return Returns size read if success; returns {@code -1} failure.
    */
    int32_t ReadStream(StreamBuffer &stream, bool isBlocking);

    /**
    * Release the resources allocated using CreateStream
    *
    * @return Returns {@code 0} if success; returns {@code -1} otherwise.
    */
    int32_t ReleaseStream();

    /**
    * Provides the current timestamp for playback/record stream created using CreateStream
    *
    * @param timeStamp will be filled up with current timestamp
    * @return Returns {@code 0} if success; returns {@code -1} otherwise.
    */
    int32_t GetCurrentTimeStamp(uint64_t &timeStamp);

    /**
    * Provides the current latency for playback/record stream created using CreateStream
    *
    * @param latency will be filled up with the current latency in microseconds
    * @return Returns {@code 0} if success; returns {@code -1} otherwise.
    */
    int32_t GetAudioLatency(uint64_t &latency);

    /**
    * Provides the playback/record stream parameters created using CreateStream
    *
    * @param audioParams will be filled up with stream audio parameters
    * @return Returns {@code 0} if success; returns {@code -1} otherwise.
    */
    int32_t GetAudioStreamParams(AudioStreamParams &audioParams);

    /**
    * Provides the minimum buffer size required for this audio stream
    * created using CreateStream
    * @param minBufferSize will be set to minimum buffer size in bytes
    * @return Returns {@code 0} if success; returns {@code -1} otherwise.
    */
    int32_t GetMinimumBufferSize(size_t &minBufferSize);

    /**
    * Provides the minimum frame count required for this audio stream
    * created using CreateStream
    * @param frameCount will be set to minimum number of frames
    * @return Returns {@code 0} if success; returns {@code -1} otherwise.
    */
    int32_t GetMinimumFrameCount(uint32_t &frameCount);

    /**
    * Provides the sampling rate for the active audio stream
    * created using CreateStream
    *
    * @return Returns sampling rate in Hz
    */
    uint32_t GetSamplingRate();

    /**
    * Provides the channel count for the active audio stream
    * created using CreateStream
    *
    * @return Returns number of channels
    */
    uint8_t GetChannelCount();

    /**
    * Provides the sample size for the active audio stream
    * created using CreateStream
    *
    * @return Returns sample size in number of bits
    */
    uint8_t GetSampleSize();

    // Device volume & route handling APIs

    // Audio stream callbacks

    /**
    * Register for callbacks associated with the playback stream created using CreateStream
    *
    * @param cb indicates pointer for registered callbacks
    * @return none
    */
    void RegisterAudioRendererCallbacks(const AudioRendererCallbacks &cb);

    /**
    * Register for callbacks associated with the record stream created using CreateStream
    *
    * @param cb indicates pointer for registered callbacks
    * @return none
    */
    void RegisterAudioCapturerCallbacks(const AudioCapturerCallbacks &cb);

    /**
     * @brief Set the track volume
     *
     * @param volume The volume to be set for the current track.
     * @return Returns {@link SUCCESS} if volume is successfully set; returns an error code
     * defined in {@link audio_errors.h} otherwise.
     */
    int32_t SetStreamVolume(float volume);

    /**
     * @brief Obtains the current track volume
     *
     * @return Returns current track volume
     */
    float GetStreamVolume();

private:
    pa_threaded_mainloop *mainLoop;
    pa_mainloop_api *api;
    pa_context *context;
    pa_stream *paStream;
    pa_sample_spec sampleSpec;

    std::mutex mtx;

    AudioCache acache;
    const void *internalReadBuffer;
    size_t internalRdBufLen;
    size_t internalRdBufIndex;
    int32_t streamCmdStatus;
    bool isMainLoopStarted;
    bool isContextConnected;
    bool isStreamConnected;

    float mVolumeFactor;
    AudioStreamType mStreamType;
    AudioSystemManager *mAudioSystemMgr;

    // To be set while using audio stream
    // functionality for callbacks
    AudioRendererCallbacks *mAudioRendererCallbacks;
    AudioCapturerCallbacks *mAudioCapturerCallbacks;

    std::map<uint32_t, SinkDeviceInfo*> sinkDevices;
    std::map<uint32_t, SourceDeviceInfo*> sourceDevices;
    std::map<uint32_t, SinkInputInfo*> sinkInputs;
    std::map<uint32_t, SourceOutputInfo*> sourceOutputs;
    std::map<uint32_t, ClientInfo*> clientInfo;

    ASClientType eAudioClientType;

    uint32_t underFlowCount;
    int32_t ConnectStreamToPA();

    // Audio cache related functions. These APIs are applicable only for playback scenarios
    int32_t InitializeAudioCache();
    size_t WriteToAudioCache(const StreamBuffer &stream);
    int32_t DrainAudioCache();

    int32_t UpdateReadBuffer(uint8_t *buffer, size_t &length, size_t &readSize);
    int32_t PaWriteStream(const uint8_t *buffer, size_t &length);

    // Error code used
    static const uint32_t AUDIO_CLIENT_SUCCESS = 0;
    static const uint32_t AUDIO_CLIENT_ERR = -1;
    static const uint32_t AUDIO_CLIENT_INVALID_PARAMS_ERR = -2;
    static const uint32_t AUDIO_CLIENT_INIT_ERR = -3;
    static const uint32_t AUDIO_CLIENT_CREATE_STREAM_ERR = -4;
    static const uint32_t AUDIO_CLIENT_START_STREAM_ERR = -5;
    static const uint32_t AUDIO_CLIENT_READ_STREAM_ERR = -6;
    static const uint32_t AUDIO_CLIENT_WRITE_STREAM_ERR = -7;
    static const uint32_t AUDIO_CLIENT_PA_ERR = -8;

    // Default values
    static const uint32_t MINIMUM_BUFFER_SIZE = 1024;
    static const uint32_t DEFAULT_SAMPLING_RATE = 44100;
    static const uint8_t DEFAULT_CHANNEL_COUNT = 2;
    static const uint8_t DEFAULT_SAMPLE_SIZE = 2;
    static const uint32_t DEFAULT_STREAM_VOLUME = 65536;
    static const std::string GetStreamName(AudioStreamType audioType);
    static pa_sample_spec ConvertToPAAudioParams(AudioStreamParams audioParams);
    static AudioStreamParams ConvertFromPAAudioParams(pa_sample_spec paSampleSpec);

    static constexpr float MAX_STREAM_VOLUME_LEVEL = 1.0f;
    static constexpr float MIN_STREAM_VOLUME_LEVEL = 0.0f;

    // Resets PA audio client and free up resources if any with this API
    void ResetPAAudioClient();

    // Callbacks to be implemented
    static void PAStreamStateCb(pa_stream *stream, void *userdata);
    static void PAStreamUnderFlowCb(pa_stream *stream, void *userdata);
    static void PAContextStateCb(pa_context *context, void *userdata);
    static void PAStreamRequestCb(pa_stream *stream, size_t length, void *userdata);
    static void PAStreamCmdSuccessCb(pa_stream *stream, int32_t success, void *userdata);
    static void PAStreamLatencyUpdateCb(pa_stream *stream, void *userdata);

    static void GetSinkInputInfoVolumeCb(pa_context *c, const pa_sink_input_info *i, int eol, void *userdata);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SERVICE_CLIENT_H
