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

#ifndef STATE_REGISTER_TEST_H
#define STATE_REGISTER_TEST_H

#include <iostream>
#include <securec.h>

#include "gtest/gtest.h"

#include "if_system_ability_manager.h"

#include "iservice_registry.h"

#include "system_ability_definition.h"

#include "string_ex.h"

#include "i_telephony_state_notify.h"
#include "telephony_observer.h"
#include "state_registry_errors.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
class StateRegistryTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    using RequestFuncType = int (StateRegistryTest::*)(const sptr<ITelephonyStateNotify> &telephonyService) const;
    std::map<char, RequestFuncType> requestFuncMap_;

    int UpdateCallState(const sptr<ITelephonyStateNotify> &telephonyService) const;
    int UpdateCallStateForSimId(const sptr<ITelephonyStateNotify> &telephonyService) const;
    int UpdateSignalInfo(const sptr<ITelephonyStateNotify> &telephonyService) const;
    int UpdateCellularDataConnectState(const sptr<ITelephonyStateNotify> &telephonyService) const;
    int UpdateSimState(const sptr<ITelephonyStateNotify> &telephonyService) const;
    int UpdateNetworkState(const sptr<ITelephonyStateNotify> &telephonyService) const;
    int InputNumForInterface(const sptr<ITelephonyStateNotify> &telephonyService) const;
};
} // namespace Telephony
} // namespace OHOS
#endif // STATE_REGISTER_TEST_H