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

#ifndef INCOMING_CALL_NOTIFICATION_H
#define INCOMING_CALL_NOTIFICATION_H

#include <set>
#include <cstdint>
#include <mutex>

#include "call_state_listener_base.h"

namespace OHOS {
namespace Telephony {
class IncomingCallNotification : public CallStateListenerBase {
public:
    IncomingCallNotification();
    virtual ~IncomingCallNotification();
    void Init();
    void NewCallCreated(sptr<CallBase> &callObjectPtr) override;
    void CallDestroyed(sptr<CallBase> &callObjectPtr) override;
    void IncomingCallActivated(sptr<CallBase> &callObjectPtr) override;
    void IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content) override;
    void CallStateUpdated(sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState) override;

private:
    std::mutex mutex_;
    const std::string title_ = "incoming call";
    void PublishNotification(const std::string &title, const std::string &text); // publish a notification
    void CancelNotification(); // cancel a notification
    bool IsAnsAbilityExist();
    bool IsFullScreen();
#ifdef ABILITY_NOTIFICATION_SUPPORT
    uint32_t notificationId_ = 0;
    const uint32_t ACTION_ANSWER = 1;
    const uint32_t ACTION_REJECT = 2;
    sptr<AdvancedNotificationService> ansAbility_;
    sptr<IRemoteObject> remoteObject_;
    std::unique_ptr<AnsManagerProxy> ansManagerProxy_;
#endif
};
} // namespace Telephony
} // namespace OHOS
#endif