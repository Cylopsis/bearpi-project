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

#include <audio_system_manager.h>
#include <gtest/gtest.h>
#include <memory>
#include <tuple>
#include <media_log.h>

namespace OHOS {
namespace AudioStandard {
namespace V1_0 {
using namespace std;
using namespace testing;

struct PolicyParam {
    float volume;
    AudioStreamType streamType;
    AudioRingerMode ringerMode;
    ActiveDeviceType actDeviceType;
    AudioDeviceDescriptor::DeviceType deviceType;
    AudioDeviceDescriptor::DeviceFlag deviceFlag;
    AudioDeviceDescriptor::DeviceRole deviceRole;
    bool active;
    bool mute;
    string key;
    string value;
};

class AudioPolicyTest : public TestWithParam<PolicyParam> {
public:
    AudioPolicyTest() {}

    virtual ~AudioPolicyTest() {}

    // SetUpTestCase: Called before all test cases
    static void SetUpTestCase(void);
    // TearDownTestCase: Called after all test case
    static void TearDownTestCase(void);
    // SetUp: Called before each test cases
    void SetUp(void) override;
    // TearDown: Called after each test cases
    void TearDown(void) override;
};
}
}
}
