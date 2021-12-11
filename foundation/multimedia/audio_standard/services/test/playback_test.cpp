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
#include <cstdio>
#include <iostream>

#include "audio_service_client.h"
#include "audio_system_manager.h"
#include "media_log.h"
#include "pcm2wav.h"

using namespace OHOS::AudioStandard;

namespace {
constexpr uint8_t DEFAULT_FORMAT = SAMPLE_S16LE;
constexpr uint8_t DEFAULT_CHANNELS = 2;
constexpr int32_t ARGS_INDEX_TWO = 2;
constexpr int32_t ARGS_INDEX_THREE = 3;
} // namespace

class PlaybackTest : public AudioRendererCallbacks {
public:
    void OnSinkDeviceUpdatedCb() const
    {
        MEDIA_INFO_LOG("My custom callback OnSinkDeviceUpdatedCb");
    }
    virtual void OnStreamStateChangeCb() const
    {
        MEDIA_INFO_LOG("My custom callback OnStreamStateChangeCb");
    }

    virtual void OnStreamBufferUnderFlowCb() const{}
    virtual void OnStreamBufferOverFlowCb() const{}
    virtual void OnErrorCb(AudioServiceErrorCodes error) const{}
    virtual void OnEventCb(AudioServiceEventTypes error) const{}
};

static int32_t InitPlayback(std::unique_ptr<AudioServiceClient> &client, AudioStreamParams &audioParams)
{
    if (client == nullptr) {
        MEDIA_ERR_LOG("Create AudioServiceClient instance failed");
        return -1;
    }

    MEDIA_INFO_LOG("Initializing of AudioServiceClient");
    if (client->Initialize(AUDIO_SERVICE_CLIENT_PLAYBACK) < 0)
        return -1;

    PlaybackTest customCb;
    client->RegisterAudioRendererCallbacks(customCb);

    MEDIA_INFO_LOG("Creating Stream");
    if (client->CreateStream(audioParams, STREAM_MUSIC) < 0)
        return -1;

    MEDIA_INFO_LOG("Starting Stream");
    if (client->StartStream() < 0)
        return -1;

    return 0;
}

int32_t StartPlayback(std::unique_ptr<AudioServiceClient> &client, FILE *wavFile)
{
    uint8_t* buffer = nullptr;
    int32_t n = 2;
    size_t bytesToWrite = 0;
    size_t bytesWritten = 0;
    size_t minBytes = 4;
    int32_t writeError;
    uint64_t timeStamp = 0;
    size_t bufferLen = 0;
    StreamBuffer stream;

    if (client->GetMinimumBufferSize(bufferLen) < 0) {
        MEDIA_ERR_LOG(" GetMinimumBufferSize failed");
        return -1;
    }

    MEDIA_DEBUG_LOG("minimum buffer length: %{public}zu", bufferLen);

    buffer = (uint8_t *) malloc(n * bufferLen);
    if (buffer == nullptr) {
        MEDIA_ERR_LOG("Failed to allocate buffer");
        return -1;
    }

    while (!feof(wavFile)) {
        bytesToWrite = fread(buffer, 1, bufferLen, wavFile);
        bytesWritten = 0;

        while ((bytesWritten < bytesToWrite) && ((bytesToWrite - bytesWritten) > minBytes)) {
            stream.buffer = buffer + bytesWritten;
            stream.bufferLen = bytesToWrite - bytesWritten;
            bytesWritten += client->WriteStream(stream, writeError);
            if (client->GetCurrentTimeStamp(timeStamp) >= 0)
                MEDIA_DEBUG_LOG("current timestamp: %{public}" PRIu64, timeStamp);
        }
    }

    free(buffer);

    return 0;
}

int main(int argc, char* argv[])
{
    wav_hdr wavHeader;
    size_t headerSize = sizeof(wav_hdr);
    char *inputPath = argv[1];
    char path[PATH_MAX + 1] = {0x00};
    if ((strlen(inputPath) > PATH_MAX) || (realpath(inputPath, path) == nullptr)) {
        MEDIA_ERR_LOG("Invalid path");
        return -1;
    }
    MEDIA_INFO_LOG("path = %{public}s", path);
    FILE* wavFile = fopen(path, "rb");
    if (wavFile == nullptr) {
        MEDIA_ERR_LOG("Unable to open wave file");
        return -1;
    }

    float volume = 0.5;
    if (argc >= ARGS_INDEX_THREE) {
        volume = strtof(argv[ARGS_INDEX_TWO], nullptr);
    }

    size_t bytesRead = fread(&wavHeader, 1, headerSize, wavFile);
    MEDIA_INFO_LOG("Header Read in bytes %{public}zu", bytesRead);
    AudioStreamParams audioParams;
    audioParams.format = DEFAULT_FORMAT;
    audioParams.samplingRate = wavHeader.SamplesPerSec;
    audioParams.channels = DEFAULT_CHANNELS;

    std::unique_ptr client = std::make_unique<AudioServiceClient>();
    if (InitPlayback(client, audioParams) < 0) {
        MEDIA_INFO_LOG("Initialize playback failed");
        fclose(wavFile);
        return -1;
    }

    AudioSystemManager *audioSystemMgr = AudioSystemManager::GetInstance();
    audioSystemMgr->SetVolume(AudioSystemManager::AudioVolumeType::STREAM_MUSIC, volume);

    if (StartPlayback(client, wavFile) < 0) {
        MEDIA_INFO_LOG("Start playback failed");
        fclose(wavFile);
        return -1;
    }

    client->FlushStream();
    client->StopStream();
    client->ReleaseStream();
    fclose(wavFile);
    MEDIA_INFO_LOG("Exit from test app");
    return 0;
}
