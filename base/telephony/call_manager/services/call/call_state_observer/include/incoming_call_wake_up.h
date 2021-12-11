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

#ifndef INCOMING_CALL_WAKE_UP_H
#define INCOMING_CALL_WAKE_UP_H

#include <mutex>

#include "call_state_listener_base.h"

namespace OHOS {
namespace Telephony {
class IncomingCallWakeup : public CallStateListenerBase {
public:
    IncomingCallWakeup() = default;
    ~IncomingCallWakeup() = default;
    void NewCallCreated(sptr<CallBase> &callObjectPtr) override;
    void CallDestroyed(sptr<CallBase> &callObjectPtr) override;
    void IncomingCallActivated(sptr<CallBase> &callObjectPtr) override;
    void IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content) override;
    void CallStateUpdated(sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState) override;

private:
    std::mutex mutex_;
    const std::string wakeupReason_ = "incoming call";
    /**
     * Wake up the device and set the screen on.
     */
    void WakeupDevice();
    /**
     * Check whether the device screen is on or not.
     */
    bool IsScreenOn();
    bool IsPowerAbilityExist();
};
} // namespace Telephony
} // namespace OHOS
#endif