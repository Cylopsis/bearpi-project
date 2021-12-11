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
#include "basic_transformer.h"
#include "securec.h"

using namespace testing::ext;
using namespace OHOS::Media;
namespace OHOS {
namespace Multimedia {
class ImageTransformTest : public testing::Test {
public:
    ImageTransformTest(){};
    ~ImageTransformTest(){};
};

/*
|255,255,0,0  0,0,0,0  255,0,255,0|
|0,0,0,0      0,0,0,0  0,0,0,0|
|0,0,0,0      0,0,0,0  0,0,0,0|
|255,0,0,255  0,0,0,0  255,163,213,234|
 */
void ConstructPixmapInfo(PixmapInfo &pixmapInfo)
{
    pixmapInfo.imageInfo.size.width = 3;
    pixmapInfo.imageInfo.size.height = 4;
    pixmapInfo.imageInfo.pixelFormat = PixelFormat::ARGB_8888;
    pixmapInfo.imageInfo.colorSpace = ColorSpace::SRGB;
    int32_t width = 3;
    int32_t height = 4;
    pixmapInfo.data = new uint8_t[width * height * 4];

    if (pixmapInfo.data == nullptr) {
        return;
    }
    pixmapInfo.bufferSize = width * height * 4;
    if (memset_s(pixmapInfo.data, sizeof(width * height * 4), 0, sizeof(width * height * 4)) != EOK) {
        ASSERT_NE(*pixmapInfo.data, 0);
    }
    for (int32_t i = 0; i < width * height; ++i) {
        int rb = i * 4;
        // the 0th item set red
        if (i == 0) {
            *(pixmapInfo.data + rb) = 255;
            *(pixmapInfo.data + rb + 1) = 255;
        }
        // the 2th item set green
        if (i == 2) {
            *(pixmapInfo.data + rb) = 255;
            *(pixmapInfo.data + rb + 2) = 255;
        }

        // the 9th item set blue
        if (i == 9) {
            *(pixmapInfo.data + rb) = 255;
            *(pixmapInfo.data + rb + 3) = 255;
        }

        // the 11th item rand
        if (i == 11) {
            *(pixmapInfo.data + rb) = 255;
            *(pixmapInfo.data + rb + 1) = 163;
            *(pixmapInfo.data + rb + 2) = 213;
            *(pixmapInfo.data + rb + 3) = 234;
        }
    }
}

/**
 * @tc.name: ImageTransformTest001
 * @tc.desc: the pixmap info scale 2.0f.
 * @tc.type: FUNC
 */
HWTEST_F(ImageTransformTest, ImageTransformTest001, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "ImageTransformTest: ImageTransform001_Scale2 start";

    PixmapInfo inPutInfo;
    ConstructPixmapInfo(inPutInfo);

    /**
     * @tc.steps: step1. construct pixel map info.
     * @tc.expected: step1. expect the pixel map width and height.
     */
    PixmapInfo outPutInfo(inPutInfo);
    BasicTransformer trans;
    trans.SetScaleParam(2.0f, 2.0f);
    trans.TransformPixmap(inPutInfo, outPutInfo);

    ASSERT_NE(outPutInfo.data, nullptr);
    EXPECT_EQ(outPutInfo.imageInfo.size.width, 6);
    EXPECT_EQ(outPutInfo.imageInfo.size.height, 8);
    EXPECT_EQ(outPutInfo.bufferSize, (uint32_t)(6 * 8 * 4));

    /**
     * @tc.steps: step2. scale 2 times.
     * @tc.expected: step2. expect four corner values.
     */
    for (int32_t i = 0; i < 6 * 8; ++i) {
        int rb = i * 4;
        // after scale 2.0, the 0th item change to 0th item
        if (i == 0) {
            EXPECT_EQ((int32_t)(*(outPutInfo.data + rb + 1)), 255);
        }

        // after scale 2.0, the 2th item change to 5th item
        if (i == 5) {
            EXPECT_EQ((int32_t)(*(outPutInfo.data + rb + 2)), 255);
        }

        // after scale 2.0, the 9th item change to 42th item
        if (i == 42) {
            EXPECT_EQ((int32_t)(*(outPutInfo.data + rb + 3)), 255);
        }

        // after scale 2.0, the 11th item change to 47th item
        if (i == 47) {
            EXPECT_EQ((int32_t)(*(outPutInfo.data + rb + 1)), 163);
            EXPECT_EQ((int32_t)(*(outPutInfo.data + rb + 2)), 213);
            EXPECT_EQ((int32_t)(*(outPutInfo.data + rb + 3)), 234);
        }
    }
    if (inPutInfo.data != nullptr) {
        free(inPutInfo.data);
        inPutInfo.data = nullptr;
    }
    if (outPutInfo.data != nullptr) {
        free(outPutInfo.data);
        outPutInfo.data = nullptr;
    }
    GTEST_LOG_(INFO) << "ImageTransformTest: ImageTransform001_Scale2 end";
}

/**
 * @tc.name: ImageTransformTest002
 * @tc.desc: the pixmap info rotate 90 at the center point.
 * @tc.type: FUNC
 */
HWTEST_F(ImageTransformTest, ImageTransformTest002, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "ImageTransformTest: ImageTransform002_Rotate90 start";

    PixmapInfo inPutInfo;
    ConstructPixmapInfo(inPutInfo);

    /**
     * @tc.steps: step1. construct pixel map info.
     * @tc.expected: step1. expect the pixel map width and height.
     */
    PixmapInfo outPutInfo(inPutInfo);
    BasicTransformer trans;
    trans.SetRotateParam(90, (float)(inPutInfo.imageInfo.size.width / 2), (float)(inPutInfo.imageInfo.size.height / 2));
    trans.TransformPixmap(inPutInfo, outPutInfo);

    ASSERT_NE(outPutInfo.data, nullptr);
    EXPECT_EQ(outPutInfo.imageInfo.size.width, 4);
    EXPECT_EQ(outPutInfo.imageInfo.size.height, 3);
    EXPECT_EQ(outPutInfo.bufferSize, (uint32_t)(4 * 3 * 4));

    /**
     * @tc.steps: step2. rotate 90.
     * @tc.expected: step2. expect four corner values.
     */
    for (int32_t i = 0; i < 4 * 3; ++i) {
        int rb = i * 4;
        // after rotate 90, the 0th change to 9th
        if (i == 0) {
            EXPECT_EQ((int32_t)(*(outPutInfo.data + rb + 3)), 255);
        }

        // after rotate 90, the 3th item change to 0th item
        if (i == 3) {
            EXPECT_EQ((int32_t)(*(outPutInfo.data + rb + 1)), 255);
        }

        // after rotate 90, the 11th item change to 2th item
        if (i == 11) {
            EXPECT_EQ((int32_t)(*(outPutInfo.data + rb + 2)), 255);
        }
    }
    if (inPutInfo.data != nullptr) {
        free(inPutInfo.data);
        inPutInfo.data = nullptr;
    }
    if (outPutInfo.data != nullptr) {
        free(outPutInfo.data);
        outPutInfo.data = nullptr;
    }
    GTEST_LOG_(INFO) << "ImageTransforTest: ImageTransfor002_Rotate90 end";
}

/**
 * @tc.name: ImageTransformTest003
 * @tc.desc: the pixmap info rotate 180 at the center point.
 * @tc.type: FUNC
 */
HWTEST_F(ImageTransformTest, ImageTransformTest003, TestSize.Level3)
{
    GTEST_LOG_(INFO) << "ImageTransformTest: ImageTransform003_Rotate180 start";

    /**
     * @tc.steps: step1. construct pixel map info.
     * @tc.expected: step1. expect the pixel map width and height.
     */
    PixmapInfo inPutInfo;
    ConstructPixmapInfo(inPutInfo);

    PixmapInfo outPutInfo(inPutInfo);
    BasicTransformer trans;
    trans.SetRotateParam(180, (float)(inPutInfo.imageInfo.size.width / 2),
                         (float)(inPutInfo.imageInfo.size.height / 2));
    trans.TransformPixmap(inPutInfo, outPutInfo);

    ASSERT_NE(outPutInfo.data, nullptr);
    EXPECT_EQ(outPutInfo.imageInfo.size.width, 3);
    EXPECT_EQ(outPutInfo.imageInfo.size.height, 4);
    EXPECT_EQ(outPutInfo.bufferSize, (uint32_t)(3 * 4 * 4));

    /**
     * @tc.steps: step2. rotate 180.
     * @tc.expected: step2. expect four corner values.
     */
    for (int32_t i = 0; i < 3 * 4; ++i) {
        int rb = i * 4;
        // after rotate 180, the 0th change to 11th
        if (i == 0) {
            EXPECT_EQ((int32_t)(*(outPutInfo.data + rb + 1)), 163);
            EXPECT_EQ((int32_t)(*(outPutInfo.data + rb + 2)), 213);
            EXPECT_EQ((int32_t)(*(outPutInfo.data + rb + 3)), 234);
        }

        // after rotate 180, the 2th item change to 9th item
        if (i == 2) {
            EXPECT_EQ((int32_t)(*(outPutInfo.data + rb + 3)), 255);
        }

        // after rotate 180, the 9th item change to 2th item
        if (i == 9) {
            EXPECT_EQ((int32_t)(*(outPutInfo.data + rb + 2)), 255);
        }

        // after rotate 180, the 11th item change to 0th item
        if (i == 11) {
            EXPECT_EQ((int32_t)(*(outPutInfo.data + rb + 1)), 255);
        }
    }
    if (inPutInfo.data != nullptr) {
        free(inPutInfo.data);
        inPutInfo.data = nullptr;
    }
    if (outPutInfo.data != nullptr) {
        free(outPutInfo.data);
        outPutInfo.data = nullptr;
    }
    GTEST_LOG_(INFO) << "ImageTransformTest: ImageTransform003_Rotate180 end";
}
} // namespace Multimedia
} // namespace OHOS
