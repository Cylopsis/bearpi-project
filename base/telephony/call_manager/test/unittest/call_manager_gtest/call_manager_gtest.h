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

#ifndef CALL_MANAGER_GTEST
#define CALL_MANAGER_GTEST

#include <gtest/gtest.h>
#include <iostream>
#include <string_ex.h>

#include "call_manager_connect.h"

namespace OHOS {
namespace Telephony {
class CallManagerGtest : public testing::Test {
public:
    // execute before first testcase
    static void SetUpTestCase()
    {
        std::cout << "---------- gtest start ------------" << std::endl;
        clientPtr_ = std::make_unique<CallManagerConnect>();
        if (clientPtr_ == nullptr) {
            std::cout << "make_unique CallManagerConnect failed!";
            return;
        }
        if (clientPtr_->Init(TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID) != TELEPHONY_SUCCESS) {
            std::cout << "connect callManager server failed!" << std::endl;
            return;
        }
        std::cout << "connect callManager  server success!!!" << std::endl;
    }

    // execute before each testcase
    void SetUp() {}

    // execute after each testcase
    void TearDown() {}

    // execute after last testcase
    static void TearDownTestCase()
    {
        if (clientPtr_ != nullptr) {
            clientPtr_->UnInit();
        }
        std::cout << "---------- gtest end ------------" << std::endl;
    }

public:
    static std::unique_ptr<CallManagerConnect> clientPtr_;
};
std::unique_ptr<CallManagerConnect> CallManagerGtest::clientPtr_ = nullptr;
} // namespace Telephony
} // namespace OHOS

#endif