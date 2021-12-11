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

#include "module_service_utils.h"
#include "cellular_call_types.h"
#include "ipc_skeleton.h"
#include "string_ex.h"

namespace OHOS {
namespace Telephony {
int32_t ModuleServiceUtils::GetRadioStatus(int32_t slotId)
{
    if (GetCore(slotId) == nullptr) {
        TELEPHONY_LOGE("ModuleServiceUtils::GetRadioStatus error, return.");
        return ERR_RESOURCE_UNAVAILABLE;
    }

    auto networkSearchManager_ = GetCore(slotId)->GetNetworkSearchManager();
    if (networkSearchManager_ == nullptr) {
        TELEPHONY_LOGE("ModuleServiceUtils::GetRadioStatus error.");
        return ERR_GET_RADIO_STATE;
    }
    return networkSearchManager_->GetRadioState();
}

RadioTech ModuleServiceUtils::GetNetworkStatus(int32_t slotId)
{
    if (GetCore(slotId) == nullptr) {
        TELEPHONY_LOGE("ModuleServiceUtils::GetNetworkStatus error, return.");
        return RadioTech::RADIO_TECHNOLOGY_UNKNOWN;
    }
    auto networkSearchManager_ = GetCore(slotId)->GetNetworkSearchManager();
    if (networkSearchManager_ == nullptr) {
        TELEPHONY_LOGE("ModuleServiceUtils::GetNetworkStatus ERROR");
        return RadioTech::RADIO_TECHNOLOGY_UNKNOWN;
    }
    if (networkSearchManager_->GetNetworkStatus(slotId) == nullptr) {
        TELEPHONY_LOGE("ModuleServiceUtils::GetNetworkStatus GetNetworkStatus() is nullptr");
        return RadioTech::RADIO_TECHNOLOGY_UNKNOWN;
    }
    auto type = static_cast<RadioTech>(networkSearchManager_->GetCsRadioTech(slotId));
    TELEPHONY_LOGE("ModuleServiceUtils::GetNetworkStatus network type is %{public}d", type);
    return RadioTech::RADIO_TECHNOLOGY_GSM;
}

std::string ModuleServiceUtils::GetIsoCountryCode(int32_t slotId)
{
    if (GetCore(slotId) == nullptr) {
        TELEPHONY_LOGE("ModuleServiceUtils::GetIsoCountryCode error, return.");
        return std::string();
    }
    auto simFileManager = GetCore(slotId)->GetSimFileManager();
    if (simFileManager == nullptr) {
        TELEPHONY_LOGE("simFileManager::GetIsoCountryCode ERROR");
        return std::string();
    }
    return Str16ToStr8(simFileManager->GetIsoCountryCodeForSim(slotId));
}

std::string ModuleServiceUtils::GetNetworkCountryCode(int32_t slotId)
{
    if (GetCore(slotId) == nullptr) {
        TELEPHONY_LOGE("ModuleServiceUtils::GetNetworkCountryCode error, return.");
        return std::string();
    }
    auto networkSearchManager_ = GetCore(slotId)->GetNetworkSearchManager();
    if (networkSearchManager_ == nullptr) {
        TELEPHONY_LOGE("ModuleServiceUtils::GetNetworkCountryCode error");
        return std::string();
    }
    return Str16ToStr8(networkSearchManager_->GetIsoCountryCodeForNetwork(slotId));
}

std::vector<int32_t> ModuleServiceUtils::GetSlotInfo()
{
    std::vector<int32_t> slotVector;
    int32_t slot = CoreManager::DEFAULT_SLOT_ID;
    slotVector.push_back(slot);
    return slotVector;
}

std::shared_ptr<Core> ModuleServiceUtils::GetCore(int32_t slotId)
{
    return CoreManager::GetInstance().getCore(slotId);
}
} // namespace Telephony
} // namespace OHOS