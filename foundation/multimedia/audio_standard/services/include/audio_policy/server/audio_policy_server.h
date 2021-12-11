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

#ifndef ST_AUDIO_POLICY_SERVER_H
#define ST_AUDIO_POLICY_SERVER_H

#include <mutex>
#include <pthread.h>

#include "audio_policy_manager_stub.h"
#include "audio_policy_service.h"
#include "iremote_stub.h"
#include "system_ability.h"

namespace OHOS {
namespace AudioStandard {
class AudioPolicyServer : public SystemAbility, public AudioPolicyManagerStub {
    DECLARE_SYSTEM_ABILITY(AudioPolicyServer);
public:
    DISALLOW_COPY_AND_MOVE(AudioPolicyServer);

    explicit AudioPolicyServer(int32_t systemAbilityId, bool runOnCreate = true);

    virtual ~AudioPolicyServer() = default;

    void OnDump() override;
    void OnStart() override;
    void OnStop() override;

    int32_t SetStreamVolume(AudioStreamType streamType, float volume) override;

    float GetStreamVolume(AudioStreamType streamType) override;

    int32_t SetStreamMute(AudioStreamType streamType, bool mute) override;

    bool GetStreamMute(AudioStreamType streamType) override;

    bool IsStreamActive(AudioStreamType streamType) override;

    int32_t SetDeviceActive(InternalDeviceType deviceType, bool active) override;

    bool IsDeviceActive(InternalDeviceType deviceType) override;

    int32_t SetRingerMode(AudioRingerMode ringMode) override;

    AudioRingerMode GetRingerMode() override;

private:
    AudioPolicyService& mPolicyService;
};
} // namespace AudioStandard
} // namespace OHOS
#endif // ST_AUDIO_POLICY_SERVER_H
