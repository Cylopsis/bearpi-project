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
#include "media_errors.h"
#include "pixel_convert.h"
#include "securec.h"

using namespace testing::ext;
using namespace OHOS::Media;
namespace OHOS {
namespace Multimedia {
#if __BYTE_ORDER == __LITTLE_ENDIAN
constexpr bool IS_LITTLE_ENDIAN = true;
#else
constexpr bool IS_LITTLE_ENDIAN = false;
#endif
class ColorConverterTest : public testing::Test {
public:
    ColorConverterTest(){};
    ~ColorConverterTest(){};
};

/**
 * @tc.name: ColorConverterTest001
 * @tc.desc: Create color space pointer, return nullptr.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    srcImageInfo.pixelFormat = PixelFormat::ARGB_8888;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    dstImageInfo.pixelFormat = PixelFormat::BGRA_8888;
    /**
     * @tc.steps: step2. build pixel convert object.
     * @tc.expected: step2. build success.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    EXPECT_NE(colorConverterPointer, nullptr);
}

/**
 * @tc.name: ColorConverterTest002
 * @tc.desc: RGB_565 to ARGB_8888.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    srcImageInfo.pixelFormat = PixelFormat::RGB_565;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    dstImageInfo.pixelFormat = PixelFormat::ARGB_8888;
    // 1010 0000 0110 0100
    uint8_t source[2] = { 0xA0, 0x64 };
    uint32_t destination[3] = { 0 };
    uint32_t result[3] = { 0 };
    if (IS_LITTLE_ENDIAN) {
        result[0] = 0x040314FF;
    } else {
        result[0] = 0xFF140304;
    }
    /**
     * @tc.steps: step2. convert pixel.
     * @tc.expected: step2. The return value is the same as the result.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    colorConverterPointer->Convert(destination, source, 1);
    int ret = memcmp(destination, result, 3 * sizeof(uint32_t));
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ColorConverterTest003
 * @tc.desc: RGB_565 to RGBA_8888.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    srcImageInfo.pixelFormat = PixelFormat::RGB_565;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL;
    dstImageInfo.pixelFormat = PixelFormat::RGBA_8888;
    uint8_t source[2] = { 0xA0, 0x64 };
    uint32_t destination[3] = { 0 };
    uint32_t result[3] = { 0 };
    if (IS_LITTLE_ENDIAN) {
        result[0] = 0xFF040314;
    } else {
        result[0] = 0x140304FF;
    }
    /**
     * @tc.steps: step2. build pixel convert object.
     * @tc.expected: step2. The return value is the same as the result.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    colorConverterPointer->Convert(destination, source, 1);
    int ret = memcmp(destination, result, 3 * sizeof(uint32_t));
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ColorConverterTest004
 * @tc.desc: RGB_565 to BGRA_8888.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest004, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    srcImageInfo.pixelFormat = PixelFormat::RGB_565;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    dstImageInfo.pixelFormat = PixelFormat::BGRA_8888;

    uint8_t source[2] = { 0xA0, 0x64 };
    uint32_t destination[3] = { 0 };
    uint32_t result[3] = { 0 };
    if (IS_LITTLE_ENDIAN) {
        result[0] = 0xFF140304;
    } else {
        result[0] = 0x040314FF;
    }
    /**
     * @tc.steps: step2. build pixel convert object.
     * @tc.expected: step2. The return value is the same as the result.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    colorConverterPointer->Convert(destination, source, 1);
    int ret = memcmp(destination, result, 3 * sizeof(uint32_t));
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ColorConverterTest005
 * @tc.desc: ARGB_8888 to BGRA_8888 UNPREMUL to PREMUL.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest005, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL;
    srcImageInfo.pixelFormat = PixelFormat::ARGB_8888;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    dstImageInfo.pixelFormat = PixelFormat::BGRA_8888;

    uint8_t source[8] = { 0x80, 0x02, 0x04, 0x08, 0x40, 0x02, 0x04, 0x08 };
    uint32_t destination[3] = { 0 };
    uint32_t result[3] = { 0 };
    if (IS_LITTLE_ENDIAN) {
        result[0] = 0x80010204;
        result[1] = 0x40010102;
    } else {
        result[0] = 0x04020180;
        result[1] = 0x02010140;
    }
    /**
     * @tc.steps: step2. build pixel convert object.
     * @tc.expected: step2. The return value is the same as the result.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    colorConverterPointer->Convert(destination, source, 2);
    int ret = memcmp(destination, result, 3 * sizeof(uint32_t));
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ColorConverterTest006
 * @tc.desc: ARGB_8888 to BGRA_8888 UNPREMUL to UNPREMUL.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest006, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL;
    srcImageInfo.pixelFormat = PixelFormat::ARGB_8888;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL;
    dstImageInfo.pixelFormat = PixelFormat::BGRA_8888;

    uint8_t source[8] = { 0x80, 0x02, 0x04, 0x08, 0x40, 0x02, 0x04, 0x08 };
    uint32_t destination[3] = { 0 };
    uint32_t result[3] = { 0 };
    if (IS_LITTLE_ENDIAN) {
        result[0] = 0x80020408;
        result[1] = 0x40020408;
    } else {
        result[0] = 0x08040280;
        result[1] = 0x08040240;
    }
    /**
     * @tc.steps: step2. build pixel convert object.
     * @tc.expected: step2. The return value is the same as the result.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    colorConverterPointer->Convert(destination, source, 2);
    int ret = memcmp(destination, result, 3 * sizeof(uint32_t));
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ColorConverterTest007
 * @tc.desc: ARGB_8888 to BGRA_8888 UNPREMUL to OPAQUE.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest007, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL;
    srcImageInfo.pixelFormat = PixelFormat::ARGB_8888;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    dstImageInfo.pixelFormat = PixelFormat::BGRA_8888;

    uint8_t source[8] = { 0x80, 0x02, 0x04, 0x08, 0x40, 0x02, 0x04, 0x08 };
    uint32_t destination[3] = { 0 };
    uint32_t result[3] = { 0 };
    if (IS_LITTLE_ENDIAN) {
        result[0] = 0xFF020408;
        result[1] = 0xFF020408;
    } else {
        result[0] = 0x080402FF;
        result[1] = 0x080402FF;
    }
    /**
     * @tc.steps: step2. build pixel convert object.
     * @tc.expected: step2. The return value is the same as the result.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    colorConverterPointer->Convert(destination, source, 2);
    int ret = memcmp(destination, result, 3 * sizeof(uint32_t));
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ColorConverterTest008
 * @tc.desc: ARGB_8888 to BGRA_8888 PREMUL to OPAQUE.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest008, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    srcImageInfo.pixelFormat = PixelFormat::ARGB_8888;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    dstImageInfo.pixelFormat = PixelFormat::BGRA_8888;

    uint8_t source[8] = { 0x80, 0x02, 0x04, 0x08, 0x40, 0x02, 0x04, 0x08 };
    uint32_t destination[3] = { 0 };
    uint32_t result[3] = { 0 };
    if (IS_LITTLE_ENDIAN) {
        result[0] = 0xFF040810;
        result[1] = 0xFF081020;
    } else {
        result[0] = 0x100804FF;
        result[1] = 0x201008FF;
    }
    /**
     * @tc.steps: step2. build pixel convert object.
     * @tc.expected: step2. The return value is the same as the result.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    colorConverterPointer->Convert(destination, source, 2);
    int ret = memcmp(destination, result, 3 * sizeof(uint32_t));
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ColorConverterTest009
 * @tc.desc: ARGB_8888 to BGRA_8888 PREMUL to UNPREMUL.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest009, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    srcImageInfo.pixelFormat = PixelFormat::ARGB_8888;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL;
    dstImageInfo.pixelFormat = PixelFormat::BGRA_8888;

    uint8_t source[8] = { 0x80, 0x02, 0x04, 0x08, 0x40, 0x02, 0x04, 0x08 };
    uint32_t destination[3] = { 0 };
    uint32_t result[3] = { 0 };
    if (IS_LITTLE_ENDIAN) {
        result[0] = 0x80040810;
        result[1] = 0x40081020;
    } else {
        result[0] = 0x10080480;
        result[1] = 0x20100840;
    }
    /**
     * @tc.steps: step2. build pixel convert object.
     * @tc.expected: step2. The return value is the same as the result.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    colorConverterPointer->Convert(destination, source, 2);
    int ret = memcmp(destination, result, 3 * sizeof(uint32_t));
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ColorConverterTest010
 * @tc.desc: ARGB_8888 to BGRA_8888 PREMUL to PREMUL.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest010, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    srcImageInfo.pixelFormat = PixelFormat::ARGB_8888;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    dstImageInfo.pixelFormat = PixelFormat::BGRA_8888;

    uint8_t source[8] = { 0x80, 0x02, 0x04, 0x08, 0x40, 0x02, 0x04, 0x08 };
    uint32_t destination[3] = { 0 };
    uint32_t result[3] = { 0 };
    if (IS_LITTLE_ENDIAN) {
        result[0] = 0x80020408;
        result[1] = 0x40020408;
    } else {
        result[0] = 0x08040280;
        result[1] = 0x08040240;
    }
    /**
     * @tc.steps: step2. build pixel convert object.
     * @tc.expected: step2. The return value is the same as the result.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    colorConverterPointer->Convert(destination, source, 2);
    int ret = memcmp(destination, result, 3 * sizeof(uint32_t));
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ColorConverterTest011
 * @tc.desc: ARGB_8888 to RGB565 UNPREMUL to OPAQUE.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest011, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL;
    srcImageInfo.pixelFormat = PixelFormat::ARGB_8888;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    dstImageInfo.pixelFormat = PixelFormat::RGB_565;

    uint8_t source[8] = { 0x80, 0x02, 0x04, 0x08, 0x40, 0x02, 0x04, 0x08 };
    uint16_t destination[3] = { 0 };
    uint16_t result[3] = { 0 };
    if (IS_LITTLE_ENDIAN) {
        result[0] = 0x0820;
        result[1] = 0x0820;
    } else {
        result[0] = 0x2008;
        result[1] = 0x2008;
    }
    /**
     * @tc.steps: step2. build pixel convert object.
     * @tc.expected: step2. The return value is the same as the result.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    colorConverterPointer->Convert(destination, source, 2);
    int ret = memcmp(destination, result, 3 * sizeof(uint16_t));
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ColorConverterTest012
 * @tc.desc: ARGB_8888 to RGB565 PREMUL to OPAQUE.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest012, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    srcImageInfo.pixelFormat = PixelFormat::ARGB_8888;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    dstImageInfo.pixelFormat = PixelFormat::RGB_565;

    uint8_t source[8] = { 0x80, 0x02, 0x04, 0x08, 0x40, 0x02, 0x04, 0x08 };
    uint16_t destination[3] = { 0 };
    uint16_t result[3] = { 0 };
    if (IS_LITTLE_ENDIAN) {
        result[0] = 0x1040;
        result[1] = 0x2081;
    } else {
        result[0] = 0x4010;
        result[1] = 0x8120;
    }
    /**
     * @tc.steps: step2. build pixel convert object.
     * @tc.expected: step2. The return value is the same as the result.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    colorConverterPointer->Convert(destination, source, 2);
    int ret = memcmp(destination, result, 3 * sizeof(uint16_t));
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ColorConverterTest013
 * @tc.desc: ARGB_8888 to BGRA_8888 UNPREMUL to PREMUL.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest013, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL;
    srcImageInfo.pixelFormat = PixelFormat::RGB_565;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL;
    dstImageInfo.pixelFormat = PixelFormat::RGB_565;
    /**
     * @tc.steps: step2. build pixel convert object.
     * @tc.expected: step2. The return value is the same as the result.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    EXPECT_EQ(colorConverterPointer, nullptr);
}

/**
 * @tc.name: ColorConverterTest014
 * @tc.desc: ARGB_8888 to RGBA_8888 PREMUL to OPAQUE.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest014, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    srcImageInfo.pixelFormat = PixelFormat::ARGB_8888;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    dstImageInfo.pixelFormat = PixelFormat::RGBA_8888;

    uint8_t source[8] = { 0x80, 0x02, 0x04, 0x08, 0x40, 0x02, 0x04, 0x08 };
    uint32_t destination[3] = { 0 };
    uint32_t result[3] = { 0 };
    if (IS_LITTLE_ENDIAN) {
        result[0] = 0xFF100804;
        result[1] = 0xFF201008;
    } else {
        result[0] = 0x040810FF;
        result[1] = 0x081020FF;
    }
    /**
     * @tc.steps: step2. build pixel convert object.
     * @tc.expected: step2. The return value is the same as the result.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    colorConverterPointer->Convert(destination, source, 2);
    int ret = memcmp(destination, result, 3 * sizeof(uint32_t));
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ColorConverterTest015
 * @tc.desc: ARGB_8888 to RGBA_8888 PREMUL to UNPREMUL.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest015, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    srcImageInfo.pixelFormat = PixelFormat::ARGB_8888;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL;
    dstImageInfo.pixelFormat = PixelFormat::RGBA_8888;

    uint8_t source[8] = { 0x80, 0x02, 0x04, 0x08, 0x40, 0x02, 0x04, 0x08 };
    uint32_t destination[3] = { 0 };
    uint32_t result[3] = { 0 };
    if (IS_LITTLE_ENDIAN) {
        result[0] = 0x80100804;
        result[1] = 0x40201008;
    } else {
        result[0] = 0x04081080;
        result[1] = 0x08102040;
    }
    /**
     * @tc.steps: step2. build pixel convert object.
     * @tc.expected: step2. The return value is the same as the result.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    colorConverterPointer->Convert(destination, source, 2);
    int ret = memcmp(destination, result, 3 * sizeof(uint32_t));
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ColorConverterTest016
 * @tc.desc: RGBA_8888 to ARGB_8888 PREMUL to OPAQUE.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest016, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    srcImageInfo.pixelFormat = PixelFormat::RGBA_8888;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    dstImageInfo.pixelFormat = PixelFormat::ARGB_8888;

    uint8_t source[8] = { 0x02, 0x04, 0x08, 0x80, 0x02, 0x04, 0x08, 0x40 };
    uint32_t destination[3] = { 0 };
    uint32_t result[3] = { 0 };
    if (IS_LITTLE_ENDIAN) {
        result[0] = 0x100804FF;
        result[1] = 0x201008FF;
    } else {
        result[0] = 0xFF040810;
        result[1] = 0xFF081020;
    }
    /**
     * @tc.steps: step2. build pixel convert object.
     * @tc.expected: step2. The return value is the same as the result.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    colorConverterPointer->Convert(destination, source, 2);
    int ret = memcmp(destination, result, 3 * sizeof(uint32_t));
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ColorConverterTest017
 * @tc.desc: RGBA_8888 to RGB_565 PREMUL to OPAQUE.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest017, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    srcImageInfo.pixelFormat = PixelFormat::RGBA_8888;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    dstImageInfo.pixelFormat = PixelFormat::RGB_565;

    uint8_t source[8] = { 0x02, 0x04, 0x08, 0x80, 0x02, 0x04, 0x08, 0x40 };
    uint16_t destination[3] = { 0 };
    uint16_t result[3] = { 0 };
    if (IS_LITTLE_ENDIAN) {
        result[0] = 0x1040;
        result[1] = 0x2081;
    } else {
        result[0] = 0x4010;
        result[1] = 0x8120;
    }
    /**
     * @tc.steps: step2. build pixel convert object.
     * @tc.expected: step2. The return value is the same as the result.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    colorConverterPointer->Convert(destination, source, 2);
    int ret = memcmp(destination, result, 3 * sizeof(uint16_t));
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ColorConverterTest018
 * @tc.desc: BGRA_8888 to ARGB_8888 PREMUL to OPAQUE.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest018, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    srcImageInfo.pixelFormat = PixelFormat::BGRA_8888;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    dstImageInfo.pixelFormat = PixelFormat::ARGB_8888;

    uint8_t source[8] = { 0x02, 0x04, 0x08, 0x80, 0x02, 0x04, 0x08, 0x40 };
    uint32_t destination[3] = { 0 };
    uint32_t result[3] = { 0 };
    if (IS_LITTLE_ENDIAN) {
        result[0] = 0x040810FF;
        result[1] = 0x081020FF;
    } else {
        result[0] = 0xFF100804;
        result[1] = 0xFF201008;
    }
    /**
     * @tc.steps: step2. build pixel convert object.
     * @tc.expected: step2. The return value is the same as the result.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    colorConverterPointer->Convert(destination, source, 2);
    int ret = memcmp(destination, result, 3 * sizeof(uint32_t));
    EXPECT_EQ(ret, 0);
}

/**
 * @tc.name: ColorConverterTest019
 * @tc.desc: BGRA_8888 to RGB_565 PREMUL to OPAQUE.
 * @tc.type: FUNC
 */
HWTEST_F(ColorConverterTest, ColorConverterTest019, TestSize.Level3)
{
    /**
     * @tc.steps: step1. set parameters to build object.
     * @tc.expected: step1. set parameters success.
     */
    ImageInfo srcImageInfo;
    srcImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    srcImageInfo.pixelFormat = PixelFormat::BGRA_8888;

    ImageInfo dstImageInfo;
    dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    dstImageInfo.pixelFormat = PixelFormat::RGB_565;

    uint8_t source[8] = { 0x02, 0x04, 0x08, 0x80, 0x02, 0x04, 0x08, 0x40 };
    uint16_t destination[3] = { 0 };
    uint16_t result[3] = { 0 };
    if (IS_LITTLE_ENDIAN) {
        result[0] = 0x0042;
        result[1] = 0x0884;
    } else {
        result[0] = 0x4200;
        result[1] = 0x8408;
    }
    /**
     * @tc.steps: step2. build pixel convert object.
     * @tc.expected: step2. The return value is the same as the result.
     */
    std::unique_ptr<PixelConvert> colorConverterPointer = PixelConvert::Create(srcImageInfo, dstImageInfo);
    colorConverterPointer->Convert(destination, source, 2);
    int ret = memcmp(destination, result, 3 * sizeof(uint16_t));
    EXPECT_EQ(ret, 0);
}
} // namespace Multimedia
} // namespace OHOS
