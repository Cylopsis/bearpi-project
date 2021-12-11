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

#include "proximity_sensor.h"

#include "call_object_manager.h"

namespace OHOS {
namespace Telephony {
ProximitySensor::ProximitySensor() {}

ProximitySensor::~ProximitySensor() {}

void ProximitySensor::ShutDown(PowerMgr::SuspendDeviceType operationType, bool suspendImmed)
{
    PowerMgr::PowerMgrClient::GetInstance().SuspendDevice(operationType, suspendImmed);
}

void ProximitySensor::StartUp(PowerMgr::WakeupDeviceType operationType, std::string wakeupReason)
{
    PowerMgr::PowerMgrClient::GetInstance().WakeupDevice(operationType, wakeupReason);
}

void ProximitySensor::CallDestroyed(sptr<CallBase> &callObjectPtr)
{
    if (CallObjectManager::HasCallExist()) {
        ShutDown(PowerMgr::SuspendDeviceType::SUSPEND_DEVICE_REASON_APPLICATION, true);
    }
}

void ProximitySensor::NewCallCreated(sptr<CallBase> &callObjectPtr) {}

void ProximitySensor::IncomingCallActivated(sptr<CallBase> &callObjectPtr) {}

void ProximitySensor::IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content) {}

void ProximitySensor::CallStateUpdated(
    sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState)
{}
} // namespace Telephony
} // namespace OHOS