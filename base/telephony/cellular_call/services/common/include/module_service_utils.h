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

#ifndef CELLULAR_CALL_MODULE_SERVICE_UTILS_H
#define CELLULAR_CALL_MODULE_SERVICE_UTILS_H

#include "core_manager.h"
#include "telephony_log_wrapper.h"
#include "iservice_registry.h"
#include "singleton.h"

namespace OHOS {
namespace Telephony {
class ModuleServiceUtils {
public:
    /**
     * Get Cs Radio state
     * @param slotId
     * @return Radio state
     */
    int32_t GetRadioStatus(int32_t slotId);

    /**
     * Get Network State
     * @param slotId
     * @return Network State
     */
    RadioTech GetNetworkStatus(int32_t slotId);

    /**
     * Get Iso Country Code
     * @param slotId
     * @return Iso Country Code
     */
    std::string GetIsoCountryCode(int32_t slotId);

    /**
     * Get net work Country Code
     * @param slotId
     * @return new work Country Code
     */
    std::string GetNetworkCountryCode(int32_t slotId);

    /**
     * Get slot info
     * @return slot id
     */
    std::vector<int32_t> GetSlotInfo();

    /**
     * Get core
     * @return slot id
     */
    std::shared_ptr<Core> GetCore(int32_t slotId);

private:
    static constexpr HiviewDFX::HiLogLabel LOG_LABEL = {LOG_CORE, LOG_DOMAIN, "ModuleServiceUtils"};
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_CALL_MODULE_SERVICE_UTILS_H
