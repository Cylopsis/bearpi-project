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

#ifndef WIRED_HEADSET_H
#define WIRED_HEADSET_H
#include <memory>
#include <mutex>

#include "event_handler.h"
#include "event_runner.h"
#include "singleton.h"
#include "call_state_listener_base.h"

namespace OHOS {
namespace Telephony {
class WiredHeadsetHandler : public AppExecFwk::EventHandler {
public:
    WiredHeadsetHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);
    virtual ~WiredHeadsetHandler();
    void Init();
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event);
};

class WiredHeadset : public CallStateListenerBase {
    DECLARE_DELAYED_SINGLETON(WiredHeadset)
public:
    void Init();
    void NewCallCreated(sptr<CallBase> &callObjectPtr) override;
    void CallDestroyed(sptr<CallBase> &callObjectPtr) override;
    void CallStateUpdated(sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState) override;
    void IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content) override;
    void IncomingCallActivated(sptr<CallBase> &callObjectPtr) override;
    enum WiredHeadsetInterfaceType {
        MEDIA_INITIALIZE_MSG = 0,
        MEDIA_SET_ACTIVE_MSG,
        MEDIA_SET_LEAVE_MSG,
    };
    enum Buttontype {
        SHORT_PRESS = 0,
        LONG_PRESS,
    };

private:
    std::shared_ptr<AppExecFwk::EventRunner> eventLoop_;
    std::shared_ptr<WiredHeadsetHandler> handler_;
    std::mutex mutex_;
};
} // namespace Telephony
} // namespace OHOS
#endif // WIRED_HEADSET_H