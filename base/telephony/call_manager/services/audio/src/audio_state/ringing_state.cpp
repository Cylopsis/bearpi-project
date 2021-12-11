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

#include "ringing_state.h"

#include "telephony_log_wrapper.h"

#include "audio_control_manager.h"
#include "audio_proxy.h"

namespace OHOS {
namespace Telephony {
bool RingingState::ProcessEvent(int32_t event)
{
    bool result = false;
    std::lock_guard<std::mutex> lock(mutex_);
    switch (event) {
        case CallStateProcess::NO_MORE_INCOMING_CALL:
            result = DelayedSingleton<AudioControlManager>::GetInstance()->UpdateCallStateWhenNoMoreActiveCall();
            break;
        case CallStateProcess::NEW_ACTIVE_CS_CALL:
            // switch to cs call state anyway.
            if (DelayedSingleton<AudioControlManager>::GetInstance()->ExistOnlyOneActiveCall()) {
                result = DelayedSingleton<AudioControlManager>::GetInstance()->ProcessEvent(
                    CallStateProcess::SWITCH_CS_CALL_STATE);
            }
            break;
        case CallStateProcess::NEW_ACTIVE_IMS_CALL:
            if (DelayedSingleton<AudioControlManager>::GetInstance()->ExistOnlyOneActiveCall()) {
                // switch to ims call state anyway.
                result = DelayedSingleton<AudioControlManager>::GetInstance()->ProcessEvent(
                    CallStateProcess::SWITCH_IMS_CALL_STATE);
            }
            break;
        default:
            break;
    }
    return result;
}
} // namespace Telephony
} // namespace OHOS
