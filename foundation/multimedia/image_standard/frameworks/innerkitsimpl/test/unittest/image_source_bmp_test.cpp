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
#include <fstream>
#include "directory_ex.h"
#include "hilog/log.h"
#include "image_packer.h"
#include "image_source.h"
#include "image_type.h"
#include "image_utils.h"
#include "media_errors.h"
#include "pixel_map.h"

using namespace testing::ext;
using namespace OHOS::Media;
using namespace OHOS::HiviewDFX;

static const std::string IMAGE_INPUT_BMP_PATH = "/sdcard/multimedia/image/test.bmp";
static const std::string IMAGE_OUTPUT_BMP_FILE_PATH = "/data/test/test_bmp_file.jpg";

int64_t PackImage(const std::string &filePath, std::unique_ptr<PixelMap> pixelMap);
bool ReadFileToBuffer(const std::string &filePath, uint8_t *buffer, size_t bufferSize);

class ImageSourceBmpTest : public testing::Test {
public:
    ImageSourceBmpTest(){};
    ~ImageSourceBmpTest(){};
};

/**
 * @tc.name: BmpImageDecode001
 * @tc.desc: Decode bmp image from file source stream(default:RGBA_8888)
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceBmpTest, BmpImageDecode001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct bmp file path and bmp format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/bmp";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_BMP_PATH, opts, errorCode);
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
    /**
     * @tc.steps: step3. compress the pixel map to jpeg file.
     * @tc.expected: step3. pack pixel map success and compare the jpeg compress file size.
     */
    int64_t packSize = PackImage(IMAGE_OUTPUT_BMP_FILE_PATH, std::move(pixelMap));
    ASSERT_NE(packSize, 0);
}

/**
 * @tc.name: BmpImageDecode002
 * @tc.desc: Decode bmp image from file source stream(BGRA_8888)
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceBmpTest, BmpImageDecode002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct bmp file path and bmp format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/bmp";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_BMP_PATH, opts, errorCode);
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
    /**
     * @tc.steps: step3. compress the pixel map to jpeg file.
     * @tc.expected: step3. pack pixel map success and compare the jpeg compress file size.
     */
    int64_t packSize = PackImage(IMAGE_OUTPUT_BMP_FILE_PATH, std::move(pixelMap));
    ASSERT_NE(packSize, 0);
}

/**
 * @tc.name: BmpImageDecode003
 * @tc.desc: Decode bmp image from file source stream(RGB_565)
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceBmpTest, BmpImageDecode003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct bmp file path and bmp format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/bmp";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_BMP_PATH, opts, errorCode);
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
 * @tc.name: BmpImageDecode004
 * @tc.desc: Decode bmp image from file source stream(ARGB_8888)
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceBmpTest, BmpImageDecode004, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct bmp file path and bmp format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/bmp";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_BMP_PATH, opts, errorCode);
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
    /**
     * @tc.steps: step3. compress the pixel map to jpeg file.
     * @tc.expected: step3. pack pixel map success and compare the jpeg compress file size.
     */
    int64_t packSize = PackImage(IMAGE_OUTPUT_BMP_FILE_PATH, std::move(pixelMap));
    ASSERT_NE(packSize, 0);
}

/**
 * @tc.name: BmpImageDecode005
 * @tc.desc: Create image source by correct bmp file path and default format hit.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceBmpTest, BmpImageDecode005, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct bmp file path and default format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_BMP_PATH, opts, errorCode);
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
 * @tc.name: BmpImageDecode006
 * @tc.desc: Create image source by correct bmp file path and wrong format hit.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceBmpTest, BmpImageDecode006, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct bmp file path and wrong format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/png";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_BMP_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
}

/**
 * @tc.name: BmpImageDecode007
 * @tc.desc: Create image source by wrong bmp file path and default format hit.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceBmpTest, BmpImageDecode007, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by wrong bmp file path and default format hit.
     * @tc.expected: step1. create image source error.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource("/data/jpeg/test.bmp", opts, errorCode);
    ASSERT_EQ(errorCode, ERR_IMAGE_SOURCE_DATA);
    ASSERT_EQ(imageSource.get(), nullptr);
}

/**
 * @tc.name: BmpImageDecode008
 * @tc.desc: Decode bmp image from buffer source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceBmpTest, BmpImageDecode008, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by buffer source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool ret = ImageUtils::GetFileSize(IMAGE_INPUT_BMP_PATH, bufferSize);
    ASSERT_EQ(ret, true);
    uint8_t *buffer = (uint8_t *)malloc(bufferSize);
    ASSERT_NE(buffer, nullptr);
    ret = ReadFileToBuffer(IMAGE_INPUT_BMP_PATH, buffer, bufferSize);
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
    /**
     * @tc.steps: step3. compress the pixel map to jpeg file.
     * @tc.expected: step3. pack pixel map success and compare the jpeg compress file size.
     */
    ImagePacker imagePacker;
    int64_t packSize = PackImage(IMAGE_OUTPUT_BMP_FILE_PATH, std::move(pixelMap));
    ASSERT_NE(packSize, 0);
    free(buffer);
}

/**
 * @tc.name: BmpImageDecode009
 * @tc.desc: Decode bmp image from istream source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceBmpTest, BmpImageDecode009, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by istream source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    std::unique_ptr<std::fstream> fs = std::make_unique<std::fstream>();
    fs->open(IMAGE_INPUT_BMP_PATH, std::fstream::binary | std::fstream::in);
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
    /**
     * @tc.steps: step3. compress the pixel map to jpeg file.
     * @tc.expected: step3. pack pixel map success and the jpeg compress file size.
     */
    int64_t packSize = PackImage(IMAGE_OUTPUT_BMP_FILE_PATH, std::move(pixelMap));
    ASSERT_NE(packSize, 0);
}

/**
 * @tc.name: BmpImageDecode010
 * @tc.desc: Decode bmp image multiple times from one imageSource
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceBmpTest, BmpImageDecode010, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by bmp file path.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_BMP_PATH, opts, errorCode);
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
    /**
     * @tc.steps: step4. compress the pixel map to jpeg file.
     * @tc.expected: step4. pack pixel map success and compare the jpeg compress file size.
     */
    int64_t packSize = PackImage(IMAGE_OUTPUT_BMP_FILE_PATH, std::move(pixelMap1));
    ASSERT_NE(packSize, 0);
    packSize = PackImage(IMAGE_OUTPUT_BMP_FILE_PATH, std::move(pixelMap2));
    ASSERT_NE(packSize, 0);
}

/**
 * @tc.name: BmpImageDecode011
 * @tc.desc: Decode wrong bmp image from one imageSource
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceBmpTest, BmpImageDecode011, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by buffer source stream and default format hit, modify data buffer to wrong
     * bmp format.
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool ret = ImageUtils::GetFileSize(IMAGE_INPUT_BMP_PATH, bufferSize);
    ASSERT_EQ(ret, true);
    uint8_t *buffer = (uint8_t *)malloc(bufferSize);
    ASSERT_NE(buffer, nullptr);
    ret = ReadFileToBuffer(IMAGE_INPUT_BMP_PATH, buffer, bufferSize);
    ASSERT_EQ(ret, true);
    buffer[0] = 43; // change one bit in buffer to wrong value.
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