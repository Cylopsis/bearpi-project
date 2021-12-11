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

#ifndef TONE_H
#define TONE_H

#include <memory>

#include "audio_proxy.h"

namespace OHOS {
namespace Telephony {
enum ToneDescriptor {
    TONE_UNKNOWN = 0,
    TONE_ENGAGED,
    TONE_FINISHED,
    TONE_WAITING,
    TONE_RING_BACK,
    TONE_NO_SERVICE,
    TONE_INVALID_NUMBER,
    TONE_CALL_RECORDING,
    TONE_DTMF_CHARACTER_0,
    TONE_DTMF_CHARACTER_1,
    TONE_DTMF_CHARACTER_2,
    TONE_DTMF_CHARACTER_3,
    TONE_DTMF_CHARACTER_4,
    TONE_DTMF_CHARACTER_5,
    TONE_DTMF_CHARACTER_6,
    TONE_DTMF_CHARACTER_7,
    TONE_DTMF_CHARACTER_8,
    TONE_DTMF_CHARACTER_9,
    TONE_DTMF_CHARACTER_P,
    TONE_DTMF_CHARACTER_W
};
struct ToneStream {
    ToneDescriptor toneDescriptor;
    int32_t duration;
    int32_t volume;
};

/**
 * @class Tone
 * plays the specific tone.
 */
class Tone {
public:
    Tone();
    explicit Tone(ToneDescriptor type);
    virtual ~Tone();
    void Init();
    int32_t Start();
    int32_t Stop();
    int32_t Release();
    static void SetDurationMs(int32_t duration);
    static int32_t GetDurationMs();
    static ToneDescriptor ConvertDigitToTone(char digit);

private:
    const static int32_t TONE_DURATION = 3000; // default tone play duration , in millisecond
    ToneDescriptor currentToneDescriptor_;
    static int32_t toneDuration_;
    bool isTonePlaying_;
    bool isCreateComplete_; // whether audio resource create complete or not
};
} // namespace Telephony
} // namespace OHOS
#endif // TONE_H