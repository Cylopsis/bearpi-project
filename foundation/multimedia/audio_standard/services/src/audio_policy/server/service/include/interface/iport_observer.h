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

#ifndef ST_AUDIO_PORT_OBSERVER_H
#define ST_AUDIO_PORT_OBSERVER_H

#include "audio_info.h"
#include "iaudio_policy_interface.h"

namespace OHOS {
namespace AudioStandard {
class IPortObserver {
public:
    virtual void OnAudioPortAvailable(std::unique_ptr<AudioPortInfo> portInfo) = 0;
    virtual void OnAudioPortPinAvailable(std::unique_ptr<AudioPortPinInfo> portInfo) = 0;
    virtual void OnDefaultOutputPortPin(InternalDeviceType device) = 0;
    virtual void OnDefaultInputPortPin(InternalDeviceType device) = 0;
};
} // namespace AudioStandard
} // namespace OHOS
#endif
