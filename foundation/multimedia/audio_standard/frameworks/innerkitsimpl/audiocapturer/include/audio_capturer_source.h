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

#ifndef AUDIO_CAPTURER_SOURCE_H
#define AUDIO_CAPTURER_SOURCE_H

#include "audio_proxy_manager.h"

#include <cstdio>

namespace OHOS {
namespace AudioStandard {
#define AUDIO_CHANNELCOUNT 2
#define AUDIO_SAMPLE_RATE_48K 48000
#define DEEP_BUFFER_CAPTURE_PERIOD_SIZE 4096
#define INT_32_MAX 0x7fffffff
#define PERIOD_SIZE 1024
#define PATH_LEN 256
#define AUDIO_BUFF_SIZE (16 * 1024)
#define PCM_8_BIT 8
#define PCM_16_BIT 16

typedef struct {
    AudioFormat format;
    uint32_t sampleFmt;
    uint32_t sampleRate;
    uint32_t channel;
    float volume;
} AudioSourceAttr;

class AudioCapturerSource {
public:
    int32_t Init(AudioSourceAttr &atrr);
    void DeInit(void);

    int32_t Start(void);
    int32_t Stop(void);
    int32_t Flush(void);
    int32_t Reset(void);
    int32_t Pause(void);
    int32_t Resume(void);
    int32_t CaptureFrame(char *frame, uint64_t requestBytes, uint64_t &replyBytes);
    int32_t SetVolume(float left, float right);
    int32_t GetVolume(float &left, float &right);
    int32_t SetMute(bool isMute);
    int32_t GetMute(bool &isMute);

    static AudioCapturerSource *GetInstance(void);
    bool capturerInited_;
    static bool micMuteState_;

private:
    const int32_t HALF_FACTOR = 2;
    const int32_t MAX_AUDIO_ADAPTER_NUM = 3;
    const float MAX_VOLUME_LEVEL = 15.0f;

    AudioSourceAttr attr_;
    bool started_;
    bool paused_;
    float leftVolume_;
    float rightVolume_;

    struct AudioProxyManager *audioManager_;
    struct AudioAdapter *audioAdapter_;
    struct AudioCapture *audioCapture_;

    void *handle_;

    int32_t CreateCapture(struct AudioPort &capturePort);
    int32_t InitAudioManager();

#ifdef CAPTURE_DUMP
    FILE *pfd;
#endif

    AudioCapturerSource();
    ~AudioCapturerSource();
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif  // AUDIO_CAPTURER_SOURCE_H
