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

#ifndef CALL_STATE_LISTENER_H
#define CALL_STATE_LISTENER_H

#include <memory>
#include <mutex>
#include <set>
#include <vector>

#include "call_state_listener_base.h"

namespace OHOS {
namespace Telephony {
class CallStateListener {
public:
    CallStateListener();
    virtual ~CallStateListener();
    bool AddOneObserver(const sptr<CallStateListenerBase> &observer);
    bool RemoveOneObserver(const sptr<CallStateListenerBase> &observer);
    bool RemoveAllObserver();
    void NewCallCreated(sptr<CallBase> &callObjectPtr);
    void CallDestroyed(sptr<CallBase> &callObjectPtr);
    void CallStateUpdated(sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState);
    void IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content);
    void IncomingCallActivated(sptr<CallBase> &callObjectPtr);
    void CallEventUpdated(CallEventInfo &info);

private:
    std::set<sptr<CallStateListenerBase>> listenerSet_;
    std::mutex mutex_;
};
} // namespace Telephony
} // namespace OHOS

#endif // CALL_STATE_LISTENER_H
