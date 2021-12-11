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

#include "call_setting_manager.h"

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

#include "cellular_call_ipc_interface_proxy.h"

namespace OHOS {
namespace Telephony {
CallSettingManager::CallSettingManager() {}

CallSettingManager::~CallSettingManager() {}

bool CallSettingManager::Init()
{
    return true;
}

int32_t CallSettingManager::GetCallWaiting(int32_t slotId)
{
    return DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->InquireCallWaiting(slotId);
}

int32_t CallSettingManager::SetCallWaiting(int32_t slotId, bool activate)
{
    return DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->SetCallWaiting(activate, slotId);
}

int32_t CallSettingManager::GetCallRestriction(int32_t slotId, CallRestrictionType type)
{
    TELEPHONY_LOGD("===========slotId = %d, type = %d", slotId, type);
    return DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->InquireCallRestriction(type, slotId);
}

int32_t CallSettingManager::SetCallRestriction(int32_t slotId, CallRestrictionInfo &info)
{
    return DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->SetCallRestriction(info, slotId);
}

int32_t CallSettingManager::GetCallTransferInfo(int32_t slotId, CallTransferType type)
{
    return DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->InquireCallTransfer(type, slotId);
}

int32_t CallSettingManager::SetCallTransferInfo(int32_t slotId, CallTransferInfo &info)
{
    return DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->SetCallTransfer(info, slotId);
}
} // namespace Telephony
} // namespace OHOS