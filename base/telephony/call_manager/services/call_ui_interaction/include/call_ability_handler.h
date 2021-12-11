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

#ifndef CALL_ABILITY_HANDLER_H
#define CALL_ABILITY_HANDLER_H

#include <memory>
#include <mutex>

#include "event_handler.h"
#include "event_runner.h"
#include "singleton.h"

#include "call_state_listener_base.h"
#include "call_ability_report_ipc_proxy.h"

namespace OHOS {
namespace Telephony {
class CallAbilityHandler : public AppExecFwk::EventHandler {
public:
    CallAbilityHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);
    virtual ~CallAbilityHandler();
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event);
};

class CallAbilityHandlerService : public CallStateListenerBase,
                                  public std::enable_shared_from_this<CallAbilityHandlerService> {
public:
    CallAbilityHandlerService();
    ~CallAbilityHandlerService();
    void Start();

    void CallStateUpdated(sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState) override;
    void CallEventUpdated(CallEventInfo &info) override;
    int32_t ReportCallStateInfo(CallAttributeInfo &info);
    int32_t ReportCallEvent(CallEventInfo &info);

    enum CallAbilityInterfaceType {
        HANDLER_REPORT_CALL_STATE_INFO = 0,
        HANDLER_REPORT_CALL_EVENT,
    };

private:
    std::shared_ptr<AppExecFwk::EventRunner> eventLoop_;
    std::shared_ptr<CallAbilityHandler> handler_;
    std::mutex mutex_;
};
} // namespace Telephony
} // namespace OHOS

#endif // CALL_ABILITY_HANDLER_H
