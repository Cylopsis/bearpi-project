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

#include <gtest/gtest.h>
#include "image_pixel_map.h"
#include "pixel_map_manager.h"

using namespace testing::ext;
using namespace OHOS::Media;
namespace OHOS {
namespace Multimedia {
class PixelMapNdkTest : public testing::Test {
public:
    PixelMapNdkTest(){};
    ~PixelMapNdkTest(){};
};

/**
 * @tc.name: PixelMapNdkTest001
 * @tc.desc: override ndk interface,exception.
 * @tc.type: FUNC
 */
HWTEST_F(PixelMapNdkTest, PixelMapNdkTest001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. test AccessPixels input Illegal pixelmap.
     * @tc.expected: step1. expect ok.
     */
    void *pixels = nullptr;
    int err = AccessPixels(nullptr, nullptr, &pixels);
    ASSERT_EQ(err, OHOS_IMAGE_RESULT_BAD_PARAMETER);

    /**
     * @tc.steps: step2. test UnAccessPixels input Illegal pixelmap.
     * @tc.expected: step2. expect ok.
     */
    err = UnAccessPixels(nullptr, nullptr);
    ASSERT_EQ(err, OHOS_IMAGE_RESULT_BAD_PARAMETER);

    /**
     * @tc.steps: step2. test GetImageInfo input Illegal pixelmap.
     * @tc.expected: step2. expect ok.
     */
    OhosPixelMapInfo info;
    err = GetImageInfo(nullptr, nullptr, info);
    ASSERT_EQ(err, OHOS_IMAGE_RESULT_BAD_PARAMETER);
}
} // namespace Multimedia
} // namespace OHOS
