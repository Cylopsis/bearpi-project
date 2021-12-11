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

#include "telephony_state_manager.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "telephony_log_wrapper.h"

#include "telephony_errors.h"

namespace OHOS {
namespace Telephony {
TelephonyStateManager::TelephonyStateManager()
{
    ConnectService();
}

TelephonyStateManager::~TelephonyStateManager() {}

int32_t TelephonyStateManager::AddStateObserver(const sptr<TelephonyObserverBroker> &telephonyObserver,
    int32_t subId, uint32_t mask, const std::u16string &callingPackage, bool notifyNow)
{
    int32_t result = TELEPHONY_SUCCESS;
    if (telephonyStateNotify_ != nullptr) {
        result =
            telephonyStateNotify_->RegisterStateChange(telephonyObserver, subId, mask, callingPackage, notifyNow);
    }
    return result;
}

int32_t TelephonyStateManager::RemoveStateObserver(int32_t subId, uint32_t mask)
{
    int32_t result = TELEPHONY_SUCCESS;
    if (telephonyStateNotify_ != nullptr) {
        result = telephonyStateNotify_->UnregisterStateChange(subId, mask);
    }
    return result;
}

int TelephonyStateManager::ConnectService()
{
    auto systemManager = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemManager == nullptr) {
        TELEPHONY_LOGE("TelephonyStateManager::ConnectService() GetSystemAbilityManager() null\n");
        return TELEPHONY_FAIL;
    }

    sptr<IRemoteObject> object = systemManager->GetSystemAbility(TELEPHONY_STATE_REGISTRY_SYS_ABILITY_ID);

    if (object != nullptr) {
        TELEPHONY_LOGD("TelephonyStateManager::ConnectService() IRemoteObject not null\n");
        telephonyStateNotify_ = iface_cast<ITelephonyStateNotify>(object);
    }

    if (telephonyStateNotify_ == nullptr) {
        TELEPHONY_LOGE("TelephonyStateManager::ConnectService() telephonyStateNotify_ null\n");
        return TELEPHONY_FAIL;
    }
    TELEPHONY_LOGD("TelephonyStateManager::ConnectService() success\n");
    return TELEPHONY_SUCCESS;
}
} // namespace Telephony
} // namespace OHOS