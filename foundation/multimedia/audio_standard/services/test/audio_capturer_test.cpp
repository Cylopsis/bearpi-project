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

#include "audio_capturer.h"
#include "media_log.h"

using namespace std;
using namespace OHOS;
using namespace OHOS::AudioStandard;

namespace AudioTestConstants {
    constexpr int32_t SECOND_ARG_IDX = 2;
    constexpr int32_t THIRD_ARG_IDX = 3;
    constexpr int32_t PAUSE_BUFFER_POSITION = 128;
    constexpr int32_t PAUSE_READ_TIME_SECONDS = 10;
    constexpr int32_t SUCCESS = 0;
}

class AudioCapturerTest {
public:
    void CheckSupportedParams() const
    {
        vector<AudioSampleFormat> supportedFormatList = AudioCapturer::GetSupportedFormats();
        MEDIA_INFO_LOG("Supported formats:");
        for (auto i = supportedFormatList.begin(); i != supportedFormatList.end(); ++i) {
            MEDIA_INFO_LOG("Format %{public}d", *i);
        }

        vector<AudioChannel> supportedChannelList = AudioCapturer::GetSupportedChannels();
        MEDIA_INFO_LOG("Supported channels:");
        for (auto i = supportedChannelList.begin(); i != supportedChannelList.end(); ++i) {
            MEDIA_INFO_LOG("channel %{public}d", *i);
        }

        vector<AudioEncodingType> supportedEncodingTypes
                                    = AudioCapturer::GetSupportedEncodingTypes();
        MEDIA_INFO_LOG("Supported encoding types:");
        for (auto i = supportedEncodingTypes.begin(); i != supportedEncodingTypes.end(); ++i) {
            MEDIA_INFO_LOG("encoding type %{public}d", *i);
        }

        vector<AudioSamplingRate> supportedSamplingRates = AudioCapturer::GetSupportedSamplingRates();
        MEDIA_INFO_LOG("Supported sampling rates:");
        for (auto i = supportedSamplingRates.begin(); i != supportedSamplingRates.end(); ++i) {
            MEDIA_INFO_LOG("sampling rate %{public}d", *i);
        }
    }

    bool InitCapture(const unique_ptr<AudioCapturer> &audioCapturer, const AudioCapturerParams &capturerParams) const
    {
        if (audioCapturer->SetParams(capturerParams) != AudioTestConstants::SUCCESS) {
            MEDIA_ERR_LOG("Set audio stream parameters failed");
            audioCapturer->Release();
            return false;
        }
        MEDIA_INFO_LOG("Capture stream created");

        MEDIA_INFO_LOG("Starting Stream");
        if (!audioCapturer->Start()) {
            MEDIA_ERR_LOG("Start stream failed");
            audioCapturer->Release();
            return false;
        }
        MEDIA_INFO_LOG("Capturing started");

        MEDIA_INFO_LOG("Get Audio parameters:");
        AudioCapturerParams getCapturerParams;
        if (audioCapturer->GetParams(getCapturerParams) == AudioTestConstants::SUCCESS) {
            MEDIA_INFO_LOG("Get Audio format: %{public}d", getCapturerParams.audioSampleFormat);
            MEDIA_INFO_LOG("Get Audio sampling rate: %{public}d", getCapturerParams.samplingRate);
            MEDIA_INFO_LOG("Get Audio channels: %{public}d", getCapturerParams.audioChannel);
        }

        return true;
    }

    bool StartCapture(const unique_ptr<AudioCapturer> &audioCapturer, bool isBlocking, FILE *pFile) const
    {
        size_t bufferLen;
        if (audioCapturer->GetBufferSize(bufferLen) < 0) {
            MEDIA_ERR_LOG(" GetMinimumBufferSize failed");
            return false;
        }

        auto buffer = std::make_unique<uint8_t[]>(bufferLen);
        if (buffer == nullptr) {
            MEDIA_ERR_LOG("AudioCapturerTest: Failed to allocate buffer");
            return false;
        }

        size_t size = 1;
        size_t numBuffersToCapture = 256;
        while (numBuffersToCapture) {
            size_t bytesRead = 0;
            while (bytesRead < bufferLen) {
                int32_t len = audioCapturer->Read(*(buffer.get() + bytesRead), bufferLen - bytesRead, isBlocking);
                if (len >= 0) {
                    bytesRead += len;
                } else {
                    bytesRead = len;
                    break;
                }
            }
            if (bytesRead < 0) {
                MEDIA_ERR_LOG("Bytes read failed. error code %{public}zu", bytesRead);
                break;
            } else if (bytesRead == 0) {
                continue;
            }

            if (fwrite(buffer.get(), size, bytesRead, pFile) != bytesRead) {
                MEDIA_ERR_LOG("error occured in fwrite");
            }
            numBuffersToCapture--;
            if ((numBuffersToCapture == AudioTestConstants::PAUSE_BUFFER_POSITION)
                && (audioCapturer->Stop())) {
                MEDIA_INFO_LOG("Audio capture stopped for %{public}d seconds",
                               AudioTestConstants::PAUSE_READ_TIME_SECONDS);
                sleep(AudioTestConstants::PAUSE_READ_TIME_SECONDS);
                if (!audioCapturer->Start()) {
                    MEDIA_ERR_LOG("resume stream failed");
                    audioCapturer->Release();
                    break;
                }
            }
        }

        return true;
    }

    bool TestRecording(int argc, char *argv[]) const
    {
        MEDIA_INFO_LOG("TestCapture start ");

        unique_ptr<AudioCapturer> audioCapturer = AudioCapturer::Create(AudioStreamType::STREAM_MUSIC);

        CheckSupportedParams();

        AudioCapturerParams capturerParams;
        capturerParams.audioSampleFormat = SAMPLE_S16LE;
        capturerParams.samplingRate =  static_cast<AudioSamplingRate>(atoi(argv[AudioTestConstants::SECOND_ARG_IDX]));
        capturerParams.audioChannel = AudioChannel::STEREO;
        capturerParams.audioEncoding = ENCODING_PCM;
        if (!InitCapture(audioCapturer, capturerParams)) {
            MEDIA_ERR_LOG("Initialize capturer failed");
            return false;
        }

        bool isBlocking = (atoi(argv[AudioTestConstants::THIRD_ARG_IDX]) == 1);
        MEDIA_INFO_LOG("Is blocking read: %{public}s", isBlocking ? "true" : "false");
        FILE *pFile = fopen(argv[AudioTestConstants::SECOND_ARG_IDX - 1], "wb");
        if (pFile == nullptr) {
            MEDIA_INFO_LOG("AudioCapturerTest: Unable to open file");
            return false;
        }

        if (!StartCapture(audioCapturer, isBlocking, pFile)) {
            MEDIA_ERR_LOG("Start capturer failed");
            fclose(pFile);
            return false;
        }

        Timestamp timestamp;
        if (audioCapturer->GetAudioTime(timestamp, Timestamp::Timestampbase::MONOTONIC)) {
            MEDIA_INFO_LOG("Timestamp seconds: %{public}ld", timestamp.time.tv_sec);
            MEDIA_INFO_LOG("Timestamp nanoseconds: %{public}ld", timestamp.time.tv_nsec);
        }

        fflush(pFile);
        if (!audioCapturer->Flush()) {
            MEDIA_ERR_LOG("AudioCapturerTest: flush failed");
        }

        if (!audioCapturer->Stop()) {
            MEDIA_ERR_LOG("AudioCapturerTest: Stop failed");
        }

        if (!audioCapturer->Release()) {
            MEDIA_ERR_LOG("AudioCapturerTest: Release failed");
        }
        fclose(pFile);
        MEDIA_INFO_LOG("TestCapture end");

        return true;
    }
};

int main(int argc, char *argv[])
{
    MEDIA_INFO_LOG("capture test in");

    if ((argv == nullptr) || (argc <= AudioTestConstants::THIRD_ARG_IDX)) {
        MEDIA_ERR_LOG("argv is null");
        return 0;
    }

    MEDIA_INFO_LOG("argc=%d", argc);
    MEDIA_INFO_LOG("argv[1]=%{public}s", argv[1]);
    MEDIA_INFO_LOG("argv[2]=%{public}s", argv[AudioTestConstants::SECOND_ARG_IDX]);
    MEDIA_INFO_LOG("argv[3]=%{public}s", argv[AudioTestConstants::THIRD_ARG_IDX]);

    AudioCapturerTest testObj;
    bool ret = testObj.TestRecording(argc, argv);

    return ret;
}
