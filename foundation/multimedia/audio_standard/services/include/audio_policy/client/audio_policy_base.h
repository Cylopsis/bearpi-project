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

#ifndef I_AUDIO_POLICY_BASE_H
#define I_AUDIO_POLICY_BASE_H

#include "audio_policy_types.h"
#include "audio_policy_manager.h"
#include "ipc_types.h"
#include "iremote_broker.h"
#include "iremote_proxy.h"
#include "iremote_stub.h"

namespace OHOS {
namespace AudioStandard {
class IAudioPolicy : public IRemoteBroker {
public:

    virtual int32_t SetStreamVolume(AudioStreamType streamType, float volume) = 0;

    virtual float GetStreamVolume(AudioStreamType streamType) = 0;

    virtual int32_t SetStreamMute(AudioStreamType streamType, bool mute) = 0;

    virtual bool GetStreamMute(AudioStreamType streamType) = 0;

    virtual bool IsStreamActive(AudioStreamType streamType) = 0;

    virtual int32_t SetDeviceActive(InternalDeviceType deviceType, bool active) = 0;

    virtual bool IsDeviceActive(InternalDeviceType deviceType) = 0;

    virtual int32_t SetRingerMode(AudioRingerMode ringMode) = 0;

    virtual AudioRingerMode GetRingerMode() = 0;
public:
    DECLARE_INTERFACE_DESCRIPTOR(u"IAudioPolicy");
};
} // AudioStandard
} // namespace OHOS
#endif // I_AUDIO_POLICY_BASE_H
