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

#ifndef DISPLAYMGR_DISPLAY_MGR_SERVICE_H
#define DISPLAYMGR_DISPLAY_MGR_SERVICE_H

#include <mutex>

#include "display_mgr_stub.h"
#include "screen_controller.h"

namespace OHOS {
namespace DisplayMgr {
class DisplayMgrService : public DisplayMgrStub {
public:
    bool SetScreenState(ScreenState state) override;
    bool SetBrightness(int32_t value) override;
    int32_t Dump(int32_t fd, const std::vector<std::u16string>& args) override;

private:
    ScreenController screenController_;
};
} // namespace DisplayMgr
} // namespace OHOS
#endif // DISPLAYMGR_DISPLAY_MGR_SERVICE_H
