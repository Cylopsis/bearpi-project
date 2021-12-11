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

#ifndef AUDIO_RENDERER_SINK_H
#define AUDIO_RENDERER_SINK_H

#include "audio_proxy_manager.h"

#include <cstdio>

namespace OHOS {
namespace AudioStandard {
typedef struct {
    AudioFormat format;
    uint32_t sampleFmt;
    uint32_t sampleRate;
    uint32_t channel;
    float volume;
} AudioSinkAttr;

class AudioRendererSink {
public:
    int32_t Init(AudioSinkAttr &atrr);
    void DeInit(void);
    int32_t Start(void);
    int32_t Stop(void);
    int32_t Flush(void);
    int32_t Reset(void);
    int32_t Pause(void);
    int32_t Resume(void);
    int32_t RenderFrame(char &frame, uint64_t len, uint64_t &writeLen);
    int32_t SetVolume(float left, float right);
    int32_t GetVolume(float &left, float &right);
    int32_t GetLatency(uint32_t *latency);
    static AudioRendererSink *GetInstance(void);
    bool rendererInited_;
private:
    AudioRendererSink();
    ~AudioRendererSink();
    AudioSinkAttr attr_;
    bool started_;
    bool paused_;
    float leftVolume_;
    float rightVolume_;
    struct AudioProxyManager *audioManager_;
    struct AudioAdapter *audioAdapter_;
    struct AudioRender *audioRender_;
    void *handle_;

    int32_t CreateRender(struct AudioPort &renderPort);
    int32_t InitAudioManager();
#ifdef DUMPFILE
    FILE *pfd;
#endif // DUMPFILE
};
}  // namespace AudioStandard
}  // namespace OHOS
#endif // AUDIO_RENDERER_SINK_H
