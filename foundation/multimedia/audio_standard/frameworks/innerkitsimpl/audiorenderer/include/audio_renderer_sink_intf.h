/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#ifndef AUDIO_RENDERER_SINK_INTF_H
#define AUDIO_RENDERER_SINK_INTF_H

#ifdef __cplusplus
extern "C" {
#endif
typedef struct {
    enum AudioFormat format;
    uint32_t sampleFmt;
    uint32_t sampleRate;
    uint32_t channel;
    float volume;
} AudioSinkAttr;

int32_t AudioRendererSinkInit(AudioSinkAttr *attr);
void AudioRendererSinkDeInit(void);
int32_t AudioRendererSinkStart(void);
int32_t AudioRendererSinkStop(void);
int32_t AudioRendererRenderFrame(char *data, uint64_t len, uint64_t *writeLen);
int32_t AudioRendererSinkSetVolume(float left, float right);
int32_t AudioRendererSinkGetLatency(uint32_t *latency);
#ifdef __cplusplus
}
#endif
#endif // AUDIO_RENDERER_SINK_INTF_H
