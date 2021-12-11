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

#ifndef PROXIMITY_SENSOR_H
#define PROXIMITY_SENSOR_H

#include "singleton.h"
#include "call_state_listener_base.h"
#include "power_mgr_client.h"

namespace OHOS {
namespace Telephony {
class ProximitySensor : public CallStateListenerBase {
    DECLARE_DELAYED_SINGLETON(ProximitySensor)
public:
    void ShutDown(PowerMgr::SuspendDeviceType operationType, bool suspendImmed);
    void StartUp(PowerMgr::WakeupDeviceType operationType, std::string wakeupReason);
    void NewCallCreated(sptr<CallBase> &callObjectPtr) override;
    void CallDestroyed(sptr<CallBase> &callObjectPtr) override;
    void CallStateUpdated(sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState) override;
    void IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content) override;
    void IncomingCallActivated(sptr<CallBase> &callObjectPtr) override;
};
} // namespace Telephony
} // namespace OHOS
#endif // PROXIMITY_SENSOR_H