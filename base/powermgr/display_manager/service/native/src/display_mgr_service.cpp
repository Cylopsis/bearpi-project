/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#include "display_mgr_service.h"

#include <file_ex.h>
#include <string_ex.h>
#include <system_ability.h>
#include <system_ability_definition.h>

#include "display_common.h"

namespace OHOS {
namespace DisplayMgr {
namespace {
class DisplaySystemAbility : public SystemAbility {
    DECLARE_SYSTEM_ABILITY(DisplaySystemAbility);

public:
    DisplaySystemAbility(int32_t id, bool runOnCreate) : SystemAbility(id, runOnCreate) {}
    ~DisplaySystemAbility() override = default;

    void OnStart() override
    {
        DISPLAY_HILOGI(MODULE_SERVICE, "Start service");
        service_ = new DisplayMgrService();
        if (!Publish(service_)) {
            DISPLAY_HILOGE(MODULE_SERVICE, "Failed to publish service");
        }
    }

    void OnStop() override
    {
        DISPLAY_HILOGI(MODULE_SERVICE, "Stop service");
    }

private:
    sptr<DisplayMgrService> service_;
};
REGISTER_SYSTEM_ABILITY_BY_ID(DisplaySystemAbility, DISPLAY_MANAGER_SERVICE_ID, true);
}

bool DisplayMgrService::SetScreenState(ScreenState state)
{
    return screenController_.UpdateState(state);
}

bool DisplayMgrService::SetBrightness(int32_t value)
{
    return screenController_.UpdateBrightness(value);
}

int32_t DisplayMgrService::Dump(int32_t fd, const std::vector<std::u16string>& args)
{
    std::string result("Empty dump info");
    if (!SaveStringToFd(fd, result)) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to save dump info to fd");
    }
    return ERR_OK;
}
} // namespace DisplayMgr
} // namespace OHOS
