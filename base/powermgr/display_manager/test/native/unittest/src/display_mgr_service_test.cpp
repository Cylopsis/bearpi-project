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

#include "display_mgr_service_test.h"

#include <iservice_registry.h>
#include <system_ability_definition.h>

#include "display_manager.h"
#include "display_mgr_service.h"

using namespace testing::ext;
using namespace OHOS;
using namespace OHOS::DisplayMgr;

/**
 * @tc.name: DisplayMgrService01
 * @tc.desc: Test DisplayMgrService service ready.
 * @tc.type: FUNC
 */
HWTEST_F(DisplayMgrServiceTest, DisplayMgrService01, TestSize.Level0)
{
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    EXPECT_TRUE(sam != nullptr) << "DisplayMgrService01 fail to get GetSystemAbilityManager";
    sptr<IRemoteObject> remoteObject_ = sam->CheckSystemAbility(DISPLAY_MANAGER_SERVICE_ID);
    EXPECT_TRUE(remoteObject_ != nullptr) << "GetSystemAbility failed.";
}

/**
 * @tc.name: DisplayMgrService002
 * @tc.desc: Test set screen state off
 * @tc.type: FUNC
 */
HWTEST_F(DisplayMgrServiceTest, DisplayMgrService002, TestSize.Level0)
{
    auto ret = DisplayManager::SetScreenState(ScreenState::SCREEN_STATE_OFF);
    sleep(5);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: DisplayMgrService003
 * @tc.desc: Test set screen state on
 * @tc.type: FUNC
 */
HWTEST_F(DisplayMgrServiceTest, DisplayMgrService003, TestSize.Level0)
{
    auto ret = DisplayManager::SetScreenState(ScreenState::SCREEN_STATE_ON);
    sleep(5);
    EXPECT_TRUE(ret);
}

/**
 * @tc.name: DisplayMgrService004
 * @tc.desc: Test set brightness
 * @tc.type: FUNC
 */
HWTEST_F(DisplayMgrServiceTest, DisplayMgrService004, TestSize.Level0)
{
    auto ret = DisplayManager::SetBrightness(5);
    sleep(5);
    EXPECT_TRUE(ret);
    ret = DisplayManager::SetBrightness(255);
    sleep(5);
    EXPECT_TRUE(ret);
}
