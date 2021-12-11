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
#include "incremental_pixel_map.h"
#include "log_tags.h"
#include "media_errors.h"
#include "pixel_map.h"

using namespace testing::ext;
using namespace OHOS::Media;
using namespace OHOS::HiviewDFX;

static constexpr OHOS::HiviewDFX::HiLogLabel LABEL_TEST = {
    LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "ImageSourceGifTest"
};
static constexpr uint32_t DEFAULT_DELAY_UTIME = 10000;  // 10 ms.

int64_t PackImage(const std::string &filePath, std::unique_ptr<PixelMap> pixelMap);
bool ReadFileToBuffer(const std::string &filePath, uint8_t *buffer, size_t bufferSize);

class ImageSourceGifTest : public testing::Test {
public:
    ImageSourceGifTest(){};
    ~ImageSourceGifTest(){};
};

/**
 * @tc.name: GifImageDecode001
 * @tc.desc: Decode gif image from file source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceGifTest, GifImageDecode001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct file path and format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/gif";
    std::unique_ptr<ImageSource> imageSource =
        ImageSource::CreateImageSource("/sdcard/multimedia/image/test.gif", opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. get the number of image.
     * @tc.expected: step2. check the number of image equals the actual number.
     */
    int32_t imageCount = imageSource->GetSourceInfo(errorCode).topLevelImageNum;
    HiLog::Debug(LABEL_TEST, "image frame number:%{public}d", imageCount);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_EQ(1, imageCount);
    /**
     * @tc.steps: step3. decode image source to bitmap by default decode options
     * @tc.expected: step3. decode image source to bitmap success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    HiLog::Debug(LABEL_TEST, "create bitmap code=%{public}d.", errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap.get(), nullptr);
    /**
     * @tc.steps: step4. get the image size.
     * @tc.expected: step4. check the image size equals the actual size.
     */
    EXPECT_EQ(198, pixelMap->GetWidth());
    EXPECT_EQ(202, pixelMap->GetHeight());
    /**
     * @tc.steps: step5. get the the RGBA of the point.
     * @tc.expected: step5. check the RGBA equals the actual RGBA.
     */
    uint32_t color = 0;
    uint32_t posX = 15;
    uint32_t posY = 15;
    pixelMap->GetARGB32Color(posX, posY, color);
    uint8_t red = pixelMap->GetARGB32ColorR(color);
    uint8_t green = pixelMap->GetARGB32ColorG(color);
    uint8_t blue = pixelMap->GetARGB32ColorB(color);
    uint8_t alpha = pixelMap->GetARGB32ColorA(color);
    HiLog::Debug(LABEL_TEST, "point:[%u, %u] RGBA:[%u, %u, %u, %u]", posX, posY, red, green, blue, alpha);

    EXPECT_EQ(244, red);
    EXPECT_EQ(63, green);
    EXPECT_EQ(19, blue);
    EXPECT_EQ(255, alpha);
}

/**
 * @tc.name: GifImageDecode002
 * @tc.desc: Create image source by correct gif file path and default format hit.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceGifTest, GifImageDecode002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct file path and wrong format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource =
        ImageSource::CreateImageSource("/sdcard/multimedia/image/test.gif", opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. get the number of image, compatibility test.
     * @tc.expected: step2. check the number of image equals the actual number.
     */
    int32_t imageCount = imageSource->GetSourceInfo(errorCode).topLevelImageNum;
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_EQ(1, imageCount);
}

/**
 * @tc.name: GifImageDecode003
 * @tc.desc: Create image source by wrong file path and correct format hit.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceGifTest, GifImageDecode003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct png file path and wrong format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/gif";
    std::unique_ptr<ImageSource> imageSource =
        ImageSource::CreateImageSource("/sdcard/multimedia/image/gif/test.gif", opts, errorCode);
    ASSERT_EQ(errorCode, ERR_IMAGE_SOURCE_DATA);
    ASSERT_EQ(imageSource.get(), nullptr);
}

/**
 * @tc.name: GifImageDecode004
 * @tc.desc: Decode gif image from buffer source stream and pixel format ARGB.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceGifTest, GifImageDecode004, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by buffer source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool ret = ImageUtils::GetFileSize("/sdcard/multimedia/image/test.gif", bufferSize);
    ASSERT_EQ(ret, true);
    uint8_t *buffer = (uint8_t *)malloc(bufferSize);
    ASSERT_NE(buffer, nullptr);
    ret = ReadFileToBuffer("/sdcard/multimedia/image/test.gif", buffer, bufferSize);
    ASSERT_EQ(ret, true);
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(buffer, bufferSize, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. decode image source to pixel map by pixel format BGRA.
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    decodeOpts.desiredPixelFormat = PixelFormat::BGRA_8888;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    HiLog::Debug(LABEL_TEST, "create bitmap code=%{public}d.", errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap.get(), nullptr);
    /**
     * @tc.steps: step3. get the the ARGB of the point.
     * @tc.expected: step3. check the ARGB equals the actual ARGB.
     */
    uint32_t color = 0;
    uint32_t posX = 15;
    uint32_t posY = 15;
    pixelMap->GetARGB32Color(posX, posY, color);
    uint8_t alpha = pixelMap->GetARGB32ColorA(color);
    uint8_t red = pixelMap->GetARGB32ColorR(color);
    uint8_t green = pixelMap->GetARGB32ColorG(color);
    uint8_t blue = pixelMap->GetARGB32ColorB(color);
    HiLog::Debug(LABEL_TEST, "point:[%u, %u] ARGB:[%u, %u, %u, %u]", posX, posY, alpha, red, green, blue);
    EXPECT_EQ(255, alpha);
    EXPECT_EQ(244, red);
    EXPECT_EQ(63, green);
    EXPECT_EQ(19, blue);
    free(buffer);
}

/**
 * @tc.name: GifImageDecode005
 * @tc.desc: Decode gif image from istream source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceGifTest, GifImageDecode005, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by istream source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    std::unique_ptr<std::fstream> fs = std::make_unique<std::fstream>();
    fs->open("/sdcard/multimedia/image/test.gif", std::fstream::binary | std::fstream::in);
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
    ASSERT_NE(pixelMap.get(), nullptr);
    /**
     * @tc.steps: step3. get the the RGBA of the point.
     * @tc.expected: step3. check the RGBA equals the actual RGBA.
     */
    uint32_t color = 0;
    uint32_t posX = 15;
    uint32_t posY = 15;
    pixelMap->GetARGB32Color(posX, posY, color);
    uint8_t red = pixelMap->GetARGB32ColorR(color);
    uint8_t green = pixelMap->GetARGB32ColorG(color);
    uint8_t blue = pixelMap->GetARGB32ColorB(color);
    uint8_t alpha = pixelMap->GetARGB32ColorA(color);
    HiLog::Debug(LABEL_TEST, "point:[%u, %u] RGBA:[%u, %u, %u, %u]", posX, posY, red, green, blue, alpha);
    EXPECT_EQ(255, alpha);
    EXPECT_EQ(244, red);
    EXPECT_EQ(63, green);
    EXPECT_EQ(19, blue);
}

/**
 * @tc.name: GifImageDecode006
 * @tc.desc: Decode moving gif image from file source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceGifTest, GifImageDecode006, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct file path and correct format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/gif";
    std::unique_ptr<ImageSource> imageSource =
        ImageSource::CreateImageSource("/sdcard/multimedia/image/moving_test.gif", opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. get the number of image.
     * @tc.expected: step2. check the number of image equals the actual number.
     */
    int32_t imageCount = imageSource->GetSourceInfo(errorCode).topLevelImageNum;
    HiLog::Debug(LABEL_TEST, "image frame number:%{public}d", imageCount);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_EQ(3, imageCount);
    /**
     * @tc.steps: step3. decode image source to bitmap by default decode options
     * @tc.expected: step3. decode image source to bitmap success.
     */
    DecodeOptions decodeOpts;
    uint32_t color = 0;
    uint32_t posX = 15;
    uint32_t posY = 15;

    std::unique_ptr<PixelMap> pixelMap;
    for (int32_t i = 0; i < imageCount; i++) {
        pixelMap = imageSource->CreatePixelMap(i, decodeOpts, errorCode);
        ASSERT_EQ(errorCode, SUCCESS);
        ASSERT_NE(pixelMap.get(), nullptr);
        if (i == 0) {
            /**
             * @tc.steps: step4. get the the RGBA of the point.
             * @tc.expected: step4. check the RGBA equals the actual RGBA.
             */
            pixelMap->GetARGB32Color(posX, posY, color);
            uint8_t red = pixelMap->GetARGB32ColorR(color);
            uint8_t green = pixelMap->GetARGB32ColorG(color);
            uint8_t blue = pixelMap->GetARGB32ColorB(color);
            uint8_t alpha = pixelMap->GetARGB32ColorA(color);
            HiLog::Debug(LABEL_TEST, "point:[%u, %u] RGBA:[%u, %u, %u, %u]", posX, posY, red, green, blue, alpha);
            EXPECT_EQ(255, alpha);
            EXPECT_EQ(244, red);
            EXPECT_EQ(63, green);
            EXPECT_EQ(19, blue);
        }
        string fileName = "/data/test/test_moving_gif" + std::to_string(i) + ".jpg";
        int64_t packSize = PackImage(fileName, std::move(pixelMap));
        ASSERT_NE(0, packSize);
    }
}

/**
 * @tc.name: GifImageDecode007
 * @tc.desc: Decode gif image from incremental source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceGifTest, GifImageDecode007, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by incremental source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool fileRet = ImageUtils::GetFileSize("/sdcard/multimedia/image/moving_test.gif", bufferSize);
    ASSERT_EQ(fileRet, true);
    uint8_t *buffer = (uint8_t *)malloc(bufferSize);
    ASSERT_NE(buffer, nullptr);
    fileRet = ReadFileToBuffer("/sdcard/multimedia/image/moving_test.gif", buffer, bufferSize);
    ASSERT_EQ(fileRet, true);
    uint32_t errorCode = 0;
    IncrementalSourceOptions incOpts;
    incOpts.incrementalMode = IncrementalMode::INCREMENTAL_DATA;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateIncrementalImageSource(incOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. update incremental stream every 10 ms with random data size and promote decode
     * image to pixel map by default decode options
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<IncrementalPixelMap> incPixelMap = imageSource->CreateIncrementalPixelMap(0, decodeOpts, errorCode);
    uint32_t updateSize = 0;
    bool isCompleted = false;
    while (updateSize < bufferSize) {
        uint32_t updateOnceSize = 10240;
        if (updateSize + updateOnceSize > bufferSize) {
            updateOnceSize = bufferSize - updateSize;
            isCompleted = true;
        }
        uint32_t ret = imageSource->UpdateData(buffer + updateSize, updateOnceSize, isCompleted);
        ASSERT_EQ(ret, SUCCESS);
        uint8_t decodeProgress = 0;
        HiLog::Debug(LABEL_TEST, "updateOnceSize:%{public}u,updateSize:%{public}u,bufferSize:%{public}zu",
                     updateOnceSize, updateSize, bufferSize);
        incPixelMap->PromoteDecoding(decodeProgress);
        updateSize += updateOnceSize;
        usleep(DEFAULT_DELAY_UTIME);
    }
    /**
     * @tc.steps: step3. get the number of image.
     * @tc.expected: step3. check the number of image equals the actual number.
     */
    int32_t imageCount = imageSource->GetSourceInfo(errorCode).topLevelImageNum;
    HiLog::Debug(LABEL_TEST, "image frame number:%{public}d", imageCount);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_EQ(3, imageCount);
}
