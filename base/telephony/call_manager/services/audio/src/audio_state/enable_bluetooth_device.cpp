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

#include "enable_bluetooth_device.h"

#include "telephony_log_wrapper.h"

#include "audio_control_manager.h"

namespace OHOS {
namespace Telephony {
bool EnableBluetoothDevice::ProcessEvent(int32_t event)
{
    bool result = false;
    std::lock_guard<std::mutex> lock(mutex_);
    switch (event) {
        case AudioDeviceManager::WIRED_HEADSET_AVAILABLE:
            // should switch to wired headset route when wired headset available
            result = DelayedSingleton<AudioControlManager>::GetInstance()->ProcessEvent(
                AudioDeviceManager::ENABLE_DEVICE_WIRED_HEADSET);
            break;
        case AudioDeviceManager::BLUETOOTH_SCO_UNAVAILABLE:
            // should reinitialize audio device in order to switch to a proper audio route
            result = DelayedSingleton<AudioControlManager>::GetInstance()->ProcessEvent(
                AudioDeviceManager::INIT_AUDIO_DEVICE);
            break;
        default:
            break;
    }
    return result;
}
} // namespace Telephony
} // namespace OHOS