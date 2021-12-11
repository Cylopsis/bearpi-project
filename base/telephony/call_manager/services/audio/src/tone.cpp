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

#include "tone.h"

#include "telephony_log_wrapper.h"

#include "audio_control_manager.h"

namespace OHOS {
namespace Telephony {
int32_t Tone::toneDuration_ = TONE_DURATION;

Tone::Tone()
    : currentToneDescriptor_(ToneDescriptor::TONE_UNKNOWN), isTonePlaying_(false), isCreateComplete_(false)
{}

Tone::Tone(ToneDescriptor type) : currentToneDescriptor_(type), isTonePlaying_(false), isCreateComplete_(false) {}

Tone::~Tone()
{
    Release();
}

void Tone::Init() {}

int32_t Tone::Start()
{
    if (currentToneDescriptor_ == TONE_UNKNOWN) {
        TELEPHONY_LOGE("tone type unknown");
        return TELEPHONY_FAIL;
    }
    if (DelayedSingleton<AudioProxy>::GetInstance()->Play(currentToneDescriptor_, toneDuration_) ==
        TELEPHONY_SUCCESS) {
        isTonePlaying_ = true;
        return TELEPHONY_SUCCESS;
    }
    return TELEPHONY_FAIL;
}

int32_t Tone::Stop()
{
    if (isTonePlaying_ == false) {
        return TELEPHONY_SUCCESS;
    }
    if (DelayedSingleton<AudioProxy>::GetInstance()->StopCallTone() == TELEPHONY_SUCCESS) {
        isTonePlaying_ = false;
        isCreateComplete_ = false;
        return TELEPHONY_SUCCESS;
    }
    return TELEPHONY_FAIL;
}

int32_t Tone::Release()
{
    TELEPHONY_LOGI("tone release");
    if (DelayedSingleton<AudioProxy>::GetInstance()->Release() == TELEPHONY_SUCCESS) {
        isTonePlaying_ = false;
        isCreateComplete_ = false;
        return TELEPHONY_SUCCESS;
    }
    return TELEPHONY_FAIL;
}

void Tone::SetDurationMs(int32_t duration)
{
    toneDuration_ = duration;
}

int32_t Tone::GetDurationMs()
{
    return toneDuration_;
}

ToneDescriptor Tone::ConvertDigitToTone(char digit)
{
    ToneDescriptor tone = ToneDescriptor::TONE_UNKNOWN;
    switch (digit) {
        case '0':
            tone = ToneDescriptor::TONE_DTMF_CHARACTER_0;
            break;
        case '1':
            tone = ToneDescriptor::TONE_DTMF_CHARACTER_1;
            break;
        case '2':
            tone = ToneDescriptor::TONE_DTMF_CHARACTER_2;
            break;
        case '3':
            tone = ToneDescriptor::TONE_DTMF_CHARACTER_3;
            break;
        case '4':
            tone = ToneDescriptor::TONE_DTMF_CHARACTER_4;
            break;
        case '5':
            tone = ToneDescriptor::TONE_DTMF_CHARACTER_5;
            break;
        case '6':
            tone = ToneDescriptor::TONE_DTMF_CHARACTER_6;
            break;
        case '7':
            tone = ToneDescriptor::TONE_DTMF_CHARACTER_7;
            break;
        case '8':
            tone = ToneDescriptor::TONE_DTMF_CHARACTER_8;
            break;
        case '9':
            tone = ToneDescriptor::TONE_DTMF_CHARACTER_9;
            break;
        case 'p':
        case 'P':
            tone = ToneDescriptor::TONE_DTMF_CHARACTER_P;
            break;
        case 'w':
        case 'W':
            tone = ToneDescriptor::TONE_DTMF_CHARACTER_W;
            break;
        default:
            break;
    }
    return tone;
}
} // namespace Telephony
} // namespace OHOS