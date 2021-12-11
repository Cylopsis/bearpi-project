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

#ifndef COMMON_TYPE_H
#define COMMON_TYPE_H

#include <cstdio>
#include <list>
#include <string>

#include "call_manager_inner_type.h"
#include "telephony_log_wrapper.h"

#define RETURN_VALUE(...) return __VA_ARGS__

#define RETURN_FAILURE_IF_NULLPTR(ptr, errorInfo, ...) \
    if (ptr == nullptr) {                              \
        TELEPHONY_LOGE(errorInfo);                     \
        RETURN_VALUE(__VA_ARGS__);                     \
    }

#define RETURN_VALUE_IF_NOT_WITHOUT_PROMT(condition, ...) \
    if (!condition) {                                     \
        RETURN_VALUE(__VA_ARGS__);                        \
    }

#define RETURN_VALUE_IF_NOT_CONDITION_WITHOUT_PROMT(value, condition, ...) \
    if (value != condition) {                                              \
        RETURN_VALUE(__VA_ARGS__);                                         \
    }

const int32_t kMaxRingingCallNumberLen = 30;
const int32_t kMaxDialingCallNumberLen = 30;
const int32_t kMaxManeLen = 10;
const int32_t kMaxPathKeyLen = 60;
const int32_t kCallStartId = 1000;
namespace OHOS {
namespace Telephony {
enum PolicyFlag {
    // Denote playing ring tone
    POLICY_FLAG_PLAY_RINGTONE = 0x00000001,
    // Denote vibration prompt
    POLICY_FLAG_VIBRATING_ALERT = 0x00000002,
    // Represents the play prompt tone
    POLICY_FLAG_PLAY_SOUND = 0x00000004,
    // Represents an audio routing handset
    POLICY_FLAG_AUDIO_DEVICE_MIC = 0x00000008,
    // Represents an audio routing speaker
    POLICY_FLAG_AUDIO_DEVICE_SPEAKER = 0x00000010,
    // Represents audio routing wired headset
    POLICY_FLAG_AUDIO_DEVICE_WIRED_HEADSET = 0x00000020,
    // Represents an audio routing Bluetooth headset
    POLICY_FLAG_AUDIO_DEVICE_BLUETOOTH = 0x00000040,
    // Indicates that no audio device is active
    POLICY_FLAG_AUDIO_DEVICE_DISABLE = 0x00000080,
};

struct CallInfo {
    char phoneNum[kMaxNumberLen];
    char scheme[kMaxSchemeNumberLen];
    int32_t dialScene;
    bool speakerphoneOn;
    int32_t accountId;
    int32_t videoState;
    int32_t startTime; // Call start time
    bool isEcc;
    bool isDefault; // Whether to dial by default
    CallType callType;
    int32_t callId;
    int64_t policyFlag;
    TelCallState state;
};

struct ContactInfo {
    char name[kMaxManeLen];
    char number[kMaxNumberLen];
    bool isContacterExists;
    char ringtonePath[kMaxPathKeyLen];
    bool isSendToVoicemail;
    bool isEcc;
    bool isVoiceMail;
};
} // namespace Telephony
} // namespace OHOS

#endif // COMMON_TYPE_H
