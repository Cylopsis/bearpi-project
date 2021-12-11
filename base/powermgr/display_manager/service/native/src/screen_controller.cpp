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

#include "screen_controller.h"

#include "display_common.h"
#include "hilog_wrapper.h"

namespace OHOS {
namespace DisplayMgr {
ScreenController::ScreenController()
{
    action_.Init();
}

bool ScreenController::UpdateState(ScreenState state)
{
    std::lock_guard lock(mutex_);
    if (IsScreenStateLocked(state)) {
        return true;
    }
    state_ = state;
    action_.SetPowerState(state);
    DISPLAY_HILOGI(MODULE_SERVICE, "Update screen state to %{public}u", ToUnderlying(state));
    return true;
}

bool ScreenController::UpdateBrightness(int32_t value)
{
    std::lock_guard lock(mutex_);
    brightness_ = value;
    action_.SetBrightness(value);
    DISPLAY_HILOGI(MODULE_SERVICE, "Update brightness to %{public}d", value);
    return true;
}

bool ScreenController::IsScreenOn()
{
    std::lock_guard lock(mutex_);
    return IsScreenStateLocked(ScreenState::SCREEN_STATE_ON);
}
} // namespace DisplayMgr
} // namespace OHOS
