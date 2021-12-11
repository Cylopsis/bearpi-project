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

#ifndef AUDIO_CAPTURER_SINK_INTF_H
#define AUDIO_CAPTURER_SINK_INTF_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    enum AudioFormat format;
    uint32_t sampleFmt;
    uint32_t sampleRate;
    uint32_t channel;
    float volume;
} AudioSourceAttr;

int32_t AudioCapturerSourceInit(AudioSourceAttr *attr);
void AudioCapturerSourceDeInit(void);
int32_t AudioCapturerSourceStart(void);
int32_t AudioCapturerSourceStop(void);
int32_t AudioCapturerSourceFrame(char *frame, uint64_t requestBytes, uint64_t *replyBytes);
int32_t AudioCapturerSourceSetVolume(float left, float right);
bool AudioCapturerSourceIsMuteRequired(void);
int32_t AudioCapturerSourceSetMute(bool);
int32_t AudioCapturerSourceGetVolume(float *left, float *right);
#ifdef __cplusplus
}
#endif

#endif  // AUDIO_CAPTURER_SINK_INTF_H