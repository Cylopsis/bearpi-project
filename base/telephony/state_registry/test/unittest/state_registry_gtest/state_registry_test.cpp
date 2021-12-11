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

#include "state_registry_test.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
void StateRegistryTest::SetUpTestCase(void)
{
    // step 3: Set Up Test Case
}

void StateRegistryTest::TearDownTestCase(void)
{
    // step 3: Tear Down Test Case
}

void StateRegistryTest::SetUp(void)
{
    // step 3: input testcase setup step
    requestFuncMap_['A'] = &StateRegistryTest::UpdateCallState;
    requestFuncMap_['B'] = &StateRegistryTest::UpdateCallStateForSimId;
    requestFuncMap_['C'] = &StateRegistryTest::UpdateSignalInfo;
    requestFuncMap_['D'] = &StateRegistryTest::UpdateCellularDataConnectState;
    requestFuncMap_['E'] = &StateRegistryTest::UpdateSimState;
    requestFuncMap_['F'] = &StateRegistryTest::UpdateNetworkState;
}

void StateRegistryTest::TearDown(void)
{
    // step 3: input testcase teardown step
}

int StateRegistryTest::UpdateCallState(const sptr<ITelephonyStateNotify> &telephonyService) const
{
    int32_t callState = 16;
    std::string phoneNumber("137xxxxxxxx");
    std::u16string number = Str8ToStr16(phoneNumber);
    return telephonyService->UpdateCallState(callState, number);
}

int StateRegistryTest::UpdateCallStateForSimId(const sptr<ITelephonyStateNotify> &telephonyService) const
{
    int32_t callState = 16;
    int32_t simId = 1;
    int32_t callId = 0;
    std::string phoneNumber("137xxxxxxxx");
    std::u16string number = Str8ToStr16(phoneNumber);
    return telephonyService->UpdateCallStateForSimId(simId, callId, callState, number);
}

int StateRegistryTest::UpdateSignalInfo(const sptr<ITelephonyStateNotify> &telephonyService) const
{
    int32_t simId = 1;
    std::vector<sptr<SignalInformation>> vec;
    std::unique_ptr<SignalInformation> signal = std::make_unique<GsmSignalInformation>();
    vec.push_back(signal.release());
    return telephonyService->UpdateSignalInfo(simId, vec);
}

int StateRegistryTest::UpdateCellularDataConnectState(const sptr<ITelephonyStateNotify> &telephonyService) const
{
    int32_t simId = 1;
    int32_t dataState = 1;
    int32_t networkState = 1;
    return telephonyService->UpdateCellularDataConnectState(simId, dataState, networkState);
}

int StateRegistryTest::UpdateSimState(const sptr<ITelephonyStateNotify> &telephonyService) const
{
    int32_t simId = 1;
    int32_t state = 0;
    std::u16string reason;
    return telephonyService->UpdateSimState(simId, state, reason);
}

int StateRegistryTest::UpdateNetworkState(const sptr<ITelephonyStateNotify> &telephonyService) const
{
    int32_t simId = 1;
    std::unique_ptr<NetworkState> networkState = std::make_unique<NetworkState>();
    return telephonyService->UpdateNetworkState(simId, networkState.release());
}

int StateRegistryTest::InputNumForInterface(const sptr<ITelephonyStateNotify> &telephonyService) const
{
    char interfaceNum = '0';
    bool loopFlag = true;
    int ret = -1;
    while (loopFlag) {
        printf(
            "\n**********Unit Test Start**********\n"
            "Usage: please input a cmd num:\n"
            "A:UpdateCallState\n"
            "B:UpdateCallStateForSimId\n"
            "C:UpdateSignalInfo\n"
            "D:UpdateCellularDataConnectState\n"
            "E:UpdateSimState\n"
            "F:UpdateNetworkState\n"
            "W:Exit\n"
            "***********************************\n"
            "Your choice: ");
        std::cin >> interfaceNum;
        if (interfaceNum == 'W') {
            break;
        }
        auto itFunc = requestFuncMap_.find(interfaceNum);
        if (itFunc != requestFuncMap_.end()) {
            auto requestFunc = itFunc->second;
            if (requestFunc != nullptr) {
                ret = (this->*requestFunc)(telephonyService);
            }
        }
        if (ret != TELEPHONY_SUCCESS) {
            return ret;
        }
    }
    return 0;
}

HWTEST_F(StateRegistryTest, i_telephony_state_notify_001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> systemAbilityMgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        printf("StateRegisterService Get ISystemAbilityManager failed.\n");
        return;
    }
    sptr<IRemoteObject> remote = systemAbilityMgr->CheckSystemAbility(TELEPHONY_STATE_REGISTRY_SYS_ABILITY_ID);
    if (remote == nullptr) {
        printf("StateRegisterService Remote service not exists.\n");
        return;
    }
    sptr<ITelephonyStateNotify> telephonyService = iface_cast<ITelephonyStateNotify>(remote);
    printf("HWTEST_F i_telephony_state_notify_001");
}
} // namespace Telephony
} // namespace OHOS