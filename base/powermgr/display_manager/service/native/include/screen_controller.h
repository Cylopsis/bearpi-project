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

#ifndef DISPLAYMGR_SCREEN_CONTROLLER_H
#define DISPLAYMGR_SCREEN_CONTROLLER_H

#include <mutex>

#include "display_info.h"
#include "screen_action.h"

namespace OHOS {
namespace DisplayMgr {
class ScreenController {
public:
    ScreenController();
    ~ScreenController() = default;

    bool UpdateState(ScreenState state);
    bool UpdateBrightness(int32_t value);
    bool IsScreenOn();

private:
    inline bool IsScreenStateLocked(ScreenState state)
    {
        return state_ == state;
    }

    std::mutex mutex_;
    ScreenState state_{ScreenState::SCREEN_STATE_ON};
    int32_t brightness_{0};
    ScreenAction action_;
};
} // namespace DisplayMgr
} // namespace OHOS
#endif // DISPLAYMGR_SCREEN_CONTROLLER_H
