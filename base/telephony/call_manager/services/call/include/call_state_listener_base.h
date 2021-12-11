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

#ifndef CALL_STATE_LISTENER_BASE_H
#define CALL_STATE_LISTENER_BASE_H

#include <cstdio>
#include <memory>
#include <string>

#include "refbase.h"

#include "call_base.h"

namespace OHOS {
namespace Telephony {
class CallStateListenerBase : public RefBase {
public:
    virtual ~CallStateListenerBase() {}
    virtual void NewCallCreated(sptr<CallBase> &callObjectPtr) {}
    virtual void CallDestroyed(sptr<CallBase> &callObjectPtr) {}
    virtual void CallStateUpdated(sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState)
    {}
    virtual void IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content) {}
    virtual void IncomingCallActivated(sptr<CallBase> &callObjectPtr) {}
    virtual void CallEventUpdated(CallEventInfo &info) {}
};
} // namespace Telephony
} // namespace OHOS

#endif // CALL_STATE_LISTENER_BASE_H
