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
    LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "ImageSourceWebpTest"
};
static constexpr uint32_t DEFAULT_DELAY_UTIME = 10000;  // 10 ms.
static const std::string IMAGE_INPUT_WEBP_PATH = "/sdcard/multimedia/image/test_large.webp";
static const std::string IMAGE_INPUT_HW_JPEG_PATH = "/sdcard/multimedia/image/test_hw.jpg";
static const std::string IMAGE_OUTPUT_JPEG_FILE_PATH = "/data/test/test_webp_file.jpg";
static const std::string IMAGE_OUTPUT_JPEG_BUFFER_PATH = "/data/test/test_webp_buffer.jpg";
static const std::string IMAGE_OUTPUT_JPEG_ISTREAM_PATH = "/data/test/test_webp_istream.jpg";
static const std::string IMAGE_OUTPUT_JPEG_INC_PATH = "/data/test/test_webp_inc.jpg";
static const std::string IMAGE_OUTPUT_JPEG_MULTI_FILE1_PATH = "/data/test/test_webp_file1.jpg";
static const std::string IMAGE_OUTPUT_JPEG_MULTI_FILE2_PATH = "/data/test/test_webp_file2.jpg";
static const std::string IMAGE_OUTPUT_JPEG_MULTI_INC1_PATH = "/data/test/test_webp_inc1.jpg";
static const std::string IMAGE_OUTPUT_JPEG_MULTI_ONETIME1_PATH = "/data/test/test_webp_onetime1.jpg";
static const std::string IMAGE_OUTPUT_JPEG_MULTI_INC2_PATH = "/data/test/test_webp_inc2.jpg";
static const std::string IMAGE_OUTPUT_JPEG_MULTI_ONETIME2_PATH = "/data/test/test_webp_onetime2.jpg";

int64_t PackImage(const std::string &filePath, std::unique_ptr<PixelMap> pixelMap);
bool ReadFileToBuffer(const std::string &filePath, uint8_t *buffer, size_t bufferSize);

class ImageSourceWebpTest : public testing::Test {
public:
    ImageSourceWebpTest(){};
    ~ImageSourceWebpTest(){};
};

/**
 * @tc.name: WebpImageDecode001
 * @tc.desc: Decode webp image from file source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWebpTest, WebpImageDecode001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct webp file path and jpeg format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/webp";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_WEBP_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. get support decode image format.
     * @tc.expected: step2. get support format info success.
     */
    std::set<string> formats;
    uint32_t ret = imageSource->GetSupportedFormats(formats);
    ASSERT_EQ(ret, SUCCESS);
    /**
     * @tc.steps: step3. decode image source to pixel map by default decode options.
     * @tc.expected: step3. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap.get(), nullptr);
    /**
     * @tc.steps: step4. get image source information.
     * @tc.expected: step4. get image source information success and source state is parsed.
     */
    SourceInfo sourceInfo = imageSource->GetSourceInfo(errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_EQ(sourceInfo.state, SourceInfoState::FILE_INFO_PARSED);
    /**
     * @tc.steps: step5. compress the pixel map to jpeg file.
     * @tc.expected: step5. pack pixel map success and the jpeg compress file.
     */
    int64_t packSize = PackImage(IMAGE_OUTPUT_JPEG_FILE_PATH, std::move(pixelMap));
    ASSERT_NE(packSize, 0);
}

/**
 * @tc.name: WebpImageDecode002
 * @tc.desc: Create image source by correct webp file path and wrong format hit.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWebpTest, WebpImageDecode002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct webp file path and default format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_WEBP_PATH, opts, errorCode);
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
    ASSERT_EQ(imageInfo.size.width, 588);
    ASSERT_EQ(imageInfo.size.height, 662);
}

/**
 * @tc.name: WebpImageDecode003
 * @tc.desc: Create image source by correct webp file path and wrong format hit.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWebpTest, WebpImageDecode003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct webp file path and wrong format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/png";
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_WEBP_PATH, opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
}

/**
 * @tc.name: WebpImageDecode004
 * @tc.desc: Create image source by wrong webp file path and default format hit.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWebpTest, WebpImageDecode004, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by wrong webp file path and default format hit.
     * @tc.expected: step1. create image source error.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource("/data/jpeg/test.webp", opts, errorCode);
    ASSERT_EQ(errorCode, ERR_IMAGE_SOURCE_DATA);
    ASSERT_EQ(imageSource.get(), nullptr);
}

/**
 * @tc.name: WebpImageDecode005
 * @tc.desc: Decode webp image from buffer source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWebpTest, WebpImageDecode005, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by buffer source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool ret = ImageUtils::GetFileSize(IMAGE_INPUT_WEBP_PATH, bufferSize);
    ASSERT_EQ(ret, true);
    uint8_t *buffer = (uint8_t *)malloc(bufferSize);
    ASSERT_NE(buffer, nullptr);
    ret = ReadFileToBuffer(IMAGE_INPUT_WEBP_PATH, buffer, bufferSize);
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
     * @tc.expected: step3. pack pixel map success and the jpeg compress file size.
     */
    ImagePacker imagePacker;
    int64_t packSize = PackImage(IMAGE_OUTPUT_JPEG_BUFFER_PATH, std::move(pixelMap));
    ASSERT_NE(packSize, 0);
    free(buffer);
}

/**
 * @tc.name: WebpImageDecode006
 * @tc.desc: Decode webp image from istream source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWebpTest, WebpImageDecode006, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by istream source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    std::unique_ptr<std::fstream> fs = std::make_unique<std::fstream>();
    fs->open(IMAGE_INPUT_WEBP_PATH, std::fstream::binary | std::fstream::in);
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
    int64_t packSize = PackImage(IMAGE_OUTPUT_JPEG_ISTREAM_PATH, std::move(pixelMap));
    ASSERT_NE(packSize, 0);
}

/**
 * @tc.name: WebpImageDecode007
 * @tc.desc: Decode webp image from incremental source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWebpTest, WebpImageDecode007, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by incremental source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool fileRet = ImageUtils::GetFileSize(IMAGE_INPUT_WEBP_PATH, bufferSize);
    ASSERT_EQ(fileRet, true);
    uint8_t *buffer = (uint8_t *)malloc(bufferSize);
    ASSERT_NE(buffer, nullptr);
    fileRet = ReadFileToBuffer(IMAGE_INPUT_WEBP_PATH, buffer, bufferSize);
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
    decodeOpts.desiredSize.height = 1024;
    decodeOpts.desiredSize.width = 512;
    std::unique_ptr<IncrementalPixelMap> incPixelMap = imageSource->CreateIncrementalPixelMap(0, decodeOpts, errorCode);
    uint32_t updateSize = 0;
    srand(time(nullptr));
    bool isCompleted = false;
    while (updateSize < bufferSize) {
        uint32_t updateOnceSize = rand() % 1024;
        if (updateSize + updateOnceSize > bufferSize) {
            updateOnceSize = bufferSize - updateSize;
            isCompleted = true;
        }
        uint32_t ret = imageSource->UpdateData(buffer + updateSize, updateOnceSize, isCompleted);
        ASSERT_EQ(ret, SUCCESS);
        uint8_t decodeProgress = 0;
        incPixelMap->PromoteDecoding(decodeProgress);
        updateSize += updateOnceSize;
        usleep(DEFAULT_DELAY_UTIME);
    }
    incPixelMap->DetachFromDecoding();
    IncrementalDecodingStatus status = incPixelMap->GetDecodingStatus();
    ASSERT_EQ(status.decodingProgress, 100);
    /**
     * @tc.steps: step3. compress the pixel map to jpeg file.
     * @tc.expected: step3. pack pixel map success and the jpeg compress file size.
     */
    int64_t packSize = PackImage(IMAGE_OUTPUT_JPEG_INC_PATH, std::move(incPixelMap));
    ASSERT_NE(packSize, 0);
    free(buffer);
}

/**
 * @tc.name: WebpImageDecode008
 * @tc.desc: Decode webp image multiple times from one ImageSource
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWebpTest, WebpImageDecode008, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by webp file path.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(IMAGE_INPUT_WEBP_PATH, opts, errorCode);
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
     * @tc.expected: step4. pack pixel map success and the jpeg compress file size.
     */
    int64_t packSize = PackImage(IMAGE_OUTPUT_JPEG_MULTI_FILE1_PATH, std::move(pixelMap1));
    ASSERT_NE(packSize, 0);
    packSize = PackImage(IMAGE_OUTPUT_JPEG_MULTI_FILE2_PATH, std::move(pixelMap2));
    ASSERT_NE(packSize, 0);
}

/**
 * @tc.name: WebpImageDecode009
 * @tc.desc: Decode webp image by incremental mode and then decode again by one-time mode.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWebpTest, WebpImageDecode009, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by incremental source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool fileRet = ImageUtils::GetFileSize(IMAGE_INPUT_WEBP_PATH, bufferSize);
    ASSERT_EQ(fileRet, true);
    uint8_t *buffer = (uint8_t *)malloc(bufferSize);
    ASSERT_NE(buffer, nullptr);
    fileRet = ReadFileToBuffer(IMAGE_INPUT_WEBP_PATH, buffer, bufferSize);
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
    decodeOpts.desiredPixelFormat = PixelFormat::BGRA_8888;
    decodeOpts.rotateDegrees = 180;
    std::unique_ptr<IncrementalPixelMap> incPixelMap = imageSource->CreateIncrementalPixelMap(0, decodeOpts, errorCode);
    uint32_t updateSize = 0;
    srand(time(nullptr));
    bool isCompleted = false;
    while (updateSize < bufferSize) {
        uint32_t updateOnceSize = rand() % 1024;
        if (updateSize + updateOnceSize > bufferSize) {
            updateOnceSize = bufferSize - updateSize;
            isCompleted = true;
        }
        uint32_t ret = imageSource->UpdateData(buffer + updateSize, updateOnceSize, isCompleted);
        ASSERT_EQ(ret, SUCCESS);
        uint8_t decodeProgress = 0;
        incPixelMap->PromoteDecoding(decodeProgress);
        updateSize += updateOnceSize;
        usleep(DEFAULT_DELAY_UTIME);
    }
    incPixelMap->DetachFromDecoding();
    IncrementalDecodingStatus status = incPixelMap->GetDecodingStatus();
    ASSERT_EQ(status.decodingProgress, 100);
    /**
     * @tc.steps: step3. decode image source to pixel map by default decode options again.
     * @tc.expected: step3. decode image source to pixel map success.
     */
    std::unique_ptr<PixelMap> pixelMap1 = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap1.get(), nullptr);
    /**
     * @tc.steps: step4. compress the pixel map to jpeg file.
     * @tc.expected: step4. pack pixel map success and the jpeg compress file size.
     */
    int64_t packSize = PackImage(IMAGE_OUTPUT_JPEG_MULTI_INC1_PATH, std::move(incPixelMap));
    ASSERT_NE(packSize, 0);
    packSize = PackImage(IMAGE_OUTPUT_JPEG_MULTI_ONETIME1_PATH, std::move(pixelMap1));
    ASSERT_NE(packSize, 0);
    free(buffer);
}

/**
 * @tc.name: WebpImageDecode010
 * @tc.desc: Decode webp image by one-time mode and then decode again by incremental mode.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWebpTest, WebpImageDecode010, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by incremental source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool fileRet = ImageUtils::GetFileSize(IMAGE_INPUT_WEBP_PATH, bufferSize);
    ASSERT_EQ(fileRet, true);
    uint8_t *buffer = (uint8_t *)malloc(bufferSize);
    ASSERT_NE(buffer, nullptr);
    fileRet = ReadFileToBuffer(IMAGE_INPUT_WEBP_PATH, buffer, bufferSize);
    ASSERT_EQ(fileRet, true);
    uint32_t errorCode = 0;
    IncrementalSourceOptions incOpts;
    incOpts.incrementalMode = IncrementalMode::INCREMENTAL_DATA;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateIncrementalImageSource(incOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. update incremental stream every 10 ms with random data size.
     * @tc.expected: step2. update success.
     */
    uint32_t updateSize = 0;
    bool isCompleted = false;
    while (updateSize < bufferSize) {
        uint32_t updateOnceSize = rand() % 1024;
        if (updateSize + updateOnceSize > bufferSize) {
            updateOnceSize = bufferSize - updateSize;
            isCompleted = true;
        }
        uint32_t ret = imageSource->UpdateData(buffer + updateSize, updateOnceSize, isCompleted);
        ASSERT_EQ(ret, SUCCESS);
        updateSize += updateOnceSize;
    }

    /**
     * @tc.steps: step3. decode image source to pixel map by default decode options.
     * @tc.expected: step3. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap1 = imageSource->CreatePixelMap(decodeOpts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap1.get(), nullptr);

    /**
     * @tc.steps: step4. decode image source to pixel map by incremental mode again.
     * @tc.expected: step4. decode image source to pixel map success.
     */
    std::unique_ptr<IncrementalPixelMap> incPixelMap = imageSource->CreateIncrementalPixelMap(0, decodeOpts, errorCode);
    uint8_t decodeProgress = 0;
    incPixelMap->PromoteDecoding(decodeProgress);
    incPixelMap->DetachFromDecoding();
    IncrementalDecodingStatus status = incPixelMap->GetDecodingStatus();
    ASSERT_EQ(status.decodingProgress, 100);
    /**
     * @tc.steps: step4. compress the pixel map to jpeg file.
     * @tc.expected: step4. pack pixel map success and the jpeg compress file size.
     */
    int64_t packSize = PackImage(IMAGE_OUTPUT_JPEG_MULTI_INC2_PATH, std::move(incPixelMap));
    ASSERT_NE(packSize, 0);
    packSize = PackImage(IMAGE_OUTPUT_JPEG_MULTI_ONETIME2_PATH, std::move(pixelMap1));
    ASSERT_NE(packSize, 0);
    free(buffer);
}

/**
 * @tc.name: WebpImageCrop001
 * @tc.desc: Crop webp image from istream source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourceWebpTest, WebpImageCrop001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create webp image source by istream source stream and default format hit
     * @tc.expected: step1. create webp image source success.
     */
    std::unique_ptr<std::fstream> fs = std::make_unique<std::fstream>();
    fs->open("/sdcard/multimedia/image/test_large.webp", std::fstream::binary | std::fstream::in);
    bool isOpen = fs->is_open();
    ASSERT_EQ(isOpen, true);
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(std::move(fs), opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. crop jpg image source to pixel map crop options
     * @tc.expected: step2. crop jpg image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    decodeOpts.CropRect.top = 3;
    decodeOpts.CropRect.width = 151;
    decodeOpts.CropRect.left = 3;
    decodeOpts.CropRect.height = 183;
    decodeOpts.desiredSize.width = 200;
    decodeOpts.desiredSize.height = 300;
    decodeOpts.rotateDegrees = 90;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    HiLog::Debug(LABEL_TEST, "create pixel map error code=%{public}u.", errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap.get(), nullptr);
    EXPECT_EQ(200, pixelMap->GetWidth());
    EXPECT_EQ(300, pixelMap->GetHeight());
}