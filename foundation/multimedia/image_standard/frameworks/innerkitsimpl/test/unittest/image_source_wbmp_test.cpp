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


#include <fstream>
#include <gtest/gtest.h>
#include "directory_ex.h"
#include "hilog/log.h"
#include "image_packer.h"
#include "image_source.h"
#include "image_source_util.h"
#include "image_type.h"
#include "image_utils.h"
#include "media_errors.h"
#include "pixel_map.h"

using namespace testing::ext;
using namespace OHOS::Media;
using namespace OHOS::HiviewDFX;

static const std::string IMAGE_INPUT_WBMP_PATH = "/sdcard/multimedia/image/test.wbmp";

class ImageSourceWbmpTest : public testing::Test {
public:
    ImageSourceWbmpTest(){};
    ~ImageSourceWbmpTest(){};
};

/**
 * @tc.name: WbmpImageDecode001
 * @tc.desc: Decode wbmp image from file source stream(default:RGBA_8888)
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWbmpTest, WbmpImageDecode001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct file path
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/vnd.wap.wbmp";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_WBMP_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. decode image source to pixel map by default decode options(RGBA_8888).
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap.get(), nullptr);
    ASSERT_EQ(pixelMap->GetAlphaType(), AlphaType::IMAGE_ALPHA_TYPE_OPAQUE);
}

/**
 * @tc.name: WbmpImageDecode002
 * @tc.desc: Decode wbmp image from file source stream(BGRA_8888)
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWbmpTest, WbmpImageDecode002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct bmp file path and bmp format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/vnd.wap.wbmp";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_WBMP_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. decode image source to pixel map using pixel format BGRA_8888.
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    decodeOpts.desiredPixelFormat = PixelFormat::BGRA_8888;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap.get(), nullptr);
    ASSERT_EQ(pixelMap->GetAlphaType(), AlphaType::IMAGE_ALPHA_TYPE_OPAQUE);
}

/**
 * @tc.name: WbmpImageDecode003
 * @tc.desc: Decode wbmp image from file source stream(RGB_565)
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWbmpTest, WbmpImageDecode003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct bmp file path and bmp format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/vnd.wap.wbmp";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_WBMP_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. decode image source to pixel map using pixel format RGB_565.
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    decodeOpts.desiredPixelFormat = PixelFormat::RGB_565;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap.get(), nullptr);
    ASSERT_EQ(pixelMap->GetAlphaType(), AlphaType::IMAGE_ALPHA_TYPE_OPAQUE);
}

/**
 * @tc.name: WbmpImageDecode004
 * @tc.desc: Decode wbmp image from file source stream(ARGB_8888)
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWbmpTest, WbmpImageDecode004, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct bmp file path and bmp format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/vnd.wap.wbmp";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_WBMP_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. decode image source to pixel map using pixel format BGRA_8888.
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    decodeOpts.desiredPixelFormat = PixelFormat::BGRA_8888;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap.get(), nullptr);
    ASSERT_EQ(pixelMap->GetAlphaType(), AlphaType::IMAGE_ALPHA_TYPE_OPAQUE);
}

/**
 * @tc.name: WbmpImageDecode005
 * @tc.desc: Create source by correct wbmp file path and default format hit.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWbmpTest, WbmpImageDecode005, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct bmp file path and default format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_WBMP_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. get image info from input image.
     * @tc.expected: step2. get image info success.
     */
    ImageInfo imageInfo;
    uint32_t ret = imageSource->GetImageInfo(0, imageInfo);
    ASSERT_EQ(ret, SUCCESS);
    ret = imageSource->GetImageInfo(imageInfo);
    ASSERT_EQ(imageInfo.size.width, 472);
    ASSERT_EQ(imageInfo.size.height, 75);
}

/**
 * @tc.name: WbmpImageDecode006
 * @tc.desc: Create image source by correct wbmp file path and wrong format hit.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWbmpTest, WbmpImageDecode006, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct bmp file path and wrong format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/png";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_WBMP_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
}

/**
 * @tc.name: WbmpImageDecode007
 * @tc.desc: Decode wbmp image from buffer source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWbmpTest, WbmpImageDecode007, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by buffer source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool ret = ImageUtils::GetFileSize(IMAGE_INPUT_WBMP_PATH, bufferSize);
    ASSERT_EQ(ret, true);
    auto *buffer = (uint8_t *)malloc(bufferSize);
    ASSERT_NE(buffer, nullptr);
    ret = ReadFileToBuffer(IMAGE_INPUT_WBMP_PATH, buffer, bufferSize);
    ASSERT_EQ(ret, true);
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(buffer, bufferSize, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. decode image source to pixel map by default decode options
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap.get(), nullptr);

    ImageInfo imageInfo;
    pixelMap->GetImageInfo(imageInfo);
    decodeOpts.CropRect = { imageInfo.size.width - 1, imageInfo.size.height - 1, 1, 1 };
    std::unique_ptr<PixelMap> cropPixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_NE(pixelMap.get(), nullptr);
    cropPixelMap->GetImageInfo(imageInfo);
    ASSERT_EQ(imageInfo.size.width, 1);
    ASSERT_EQ(imageInfo.size.height, 1);
}

/**
 * @tc.name: WbmpImageDecode008
 * @tc.desc: Decode wbmp image from istream source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWbmpTest, WbmpImageDecode008, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by istream source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    std::unique_ptr<std::fstream> fs = std::make_unique<std::fstream>();
    fs->open(IMAGE_INPUT_WBMP_PATH, std::fstream::binary | std::fstream::in);
    bool isOpen = fs->is_open();
    ASSERT_EQ(isOpen, true);
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(std::move(fs), opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. decode image source to pixel map by default decode options
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
}

/**
 * @tc.name: WbmpImageDecode009
 * @tc.desc: Decode wbmp image multiple times from one imageSource
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWbmpTest, WbmpImageDecode009, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by bmp file path.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_WBMP_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. decode image source to pixel map by default decode options.
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap1 = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap1.get(), nullptr);
    /**
     * @tc.steps: step3. decode image source to pixel map by default decode options again.
     * @tc.expected: step3. decode image source to pixel map success.
     */
    std::unique_ptr<PixelMap> pixelMap2 = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap1.get(), nullptr);
}

/**
 * @tc.name: WbmpImageDecode010
 * @tc.desc: Decode wrong wbmp image from one imageSource
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWbmpTest, WbmpImageDecode010, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by buffer source stream and default format hit, modify data buffer to wrong
     * format.
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool ret = ImageUtils::GetFileSize(IMAGE_INPUT_WBMP_PATH, bufferSize);
    ASSERT_EQ(ret, true);
    auto *buffer = (uint8_t *)malloc(bufferSize);
    ASSERT_NE(buffer, nullptr);
    ret = ReadFileToBuffer(IMAGE_INPUT_WBMP_PATH, buffer, bufferSize);
    ASSERT_EQ(ret, true);
    buffer[0] = 43;
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(buffer, bufferSize, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. decode image source to pixel map by default decode options
     * @tc.expected: step2. decode image source to pixel map failed, because bmp format error.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_NE(errorCode, SUCCESS);
    ASSERT_EQ(pixelMap.get(), nullptr);
}