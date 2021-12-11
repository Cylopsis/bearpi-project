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

#ifndef AUDIO_SESSION_H
#define AUDIO_SESSION_H

#include "audio_service_client.h"

namespace OHOS {
namespace AudioStandard {
enum SessionType {
    SESSION_CONTROL,
    SESSION_PLAYBACK,
    SESSION_RECORD
};

struct AudioDevDescriptor {
};

class AudioSession : public AudioServiceClient {
public:
    uint32_t GetSessionID();

    AudioDevDescriptor *GetActiveAudioSinkDevice(uint32_t sessionID);
    AudioDevDescriptor *GetActiveAudioSourceDevice(uint32_t sessionID);

    bool SetActiveAudioSinkDevice(uint32_t sessionID, const AudioDevDescriptor &audioDesc);
    bool SetActiveAudioSourceDevice(uint32_t sessionID, const AudioDevDescriptor &audioDesc);
    float GetAudioStreamVolume(uint32_t sessionID);
    float GetAudioDeviceVolume(uint32_t sessionID);
    bool SetAudioStreamVolume(uint32_t sessionID, float volume);
    bool SetAudioDeviceVolume(uint32_t sessionID, float volume);

private:
    AudioSession *CreateSession(SessionType eSession);
};
} // namespace AudioStandard
} // namespace OHOS
#endif // AUDIO_SESSION_H
