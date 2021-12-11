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

#include "screen_action.h"

#include "display_common.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace DisplayMgr {
void ScreenAction::Init()
{
    DeviceFuncs *f = NULL;

    int32_t ret = DeviceInitialize(&f);
    if (ret != DISPLAY_SUCCESS) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Failed to init device");
        return;
    }
    hdiFuncs_ = DeviceFuncPtr(f);
    DISPLAY_HILOGI(MODULE_SERVICE, "Succeed to init");
}

bool ScreenAction::SetPowerState(ScreenState state __attribute__((__unused__)))
{
    return true;
}

bool ScreenAction::SetBrightness(int32_t value)
{
    if (!hdiFuncs_) {
        DISPLAY_HILOGE(MODULE_SERVICE, "Invalid device functions");
        return false;
    }
    return hdiFuncs_->SetDisplayBacklight(0, GetValidBrightness(value)) == DISPLAY_SUCCESS;
}
} // namespace DisplayMgr
} // namespace OHOS
