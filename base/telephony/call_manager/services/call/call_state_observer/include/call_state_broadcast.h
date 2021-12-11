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

#ifndef CALL_STATE_BROADCAST_H
#define CALL_STATE_BROADCAST_H

#include "call_state_listener_base.h"

namespace OHOS {
namespace Telephony {
class CallStateBroadcast : public CallStateListenerBase {
public:
    CallStateBroadcast() = default;
    ~CallStateBroadcast() = default;
    void NewCallCreated(sptr<CallBase> &callObjectPtr) override;
    void CallDestroyed(sptr<CallBase> &callObjectPtr) override;
    void IncomingCallActivated(sptr<CallBase> &callObjectPtr) override;
    void IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content) override;
    void CallStateUpdated(sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState) override;

private:
    int32_t eventCode_ = 0;
    const std::string EVENT_DATA = "call state update";
    const std::string CALL_STATE_UPDATE = "ohos.action.TELEPHONY_CALL_MANAGER_CALL_STATE_UPDATE";
    void PublishCommonEvent(sptr<CallBase> &callObjectPtr, int32_t code);
};
} // namespace Telephony
} // namespace OHOS
#endif // CALL_STATE_BROADCAST_H