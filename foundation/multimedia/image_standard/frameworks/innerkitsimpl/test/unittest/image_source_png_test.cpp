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
    LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "ImageSourcePngTest"
};
static constexpr uint32_t DEFAULT_DELAY_UTIME = 10000;  // 10 ms.

int64_t PackImage(const std::string &filePath, std::unique_ptr<PixelMap> pixelMap);
bool ReadFileToBuffer(const std::string &filePath, uint8_t *buffer, size_t bufferSize);

class ImageSourcePngTest : public testing::Test {
public:
    ImageSourcePngTest(){};
    ~ImageSourcePngTest(){};
};

/**
 * @tc.name: PngImageDecode001
 * @tc.desc: Decode png image from file source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourcePngTest, PngImageDecode001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct png file path and png format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/png";
    std::unique_ptr<ImageSource> imageSource =
        ImageSource::CreateImageSource("/sdcard/multimedia/image/test.png", opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. decode image source to pixel map by default decode options
     * @tc.expected: step2. decode image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    HiLog::Debug(LABEL_TEST, "create pixel map error code=%{public}u.", errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap.get(), nullptr);
    /**
     * @tc.steps: step3. compress the pixel map to png file.
     * @tc.expected: step3. pack pixel map success and the png compress file size equals to PNG_PACK_SIZE.
     */
    int64_t packSize = PackImage("/sdcard/multimedia/image/test_file.png", std::move(pixelMap));
    ASSERT_NE(packSize, 0);
}

/**
 * @tc.name: PngImageDecode002
 * @tc.desc: Create image source by correct png file path and wrong format hit.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourcePngTest, PngImageDecode002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct png file path and default format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource =
        ImageSource::CreateImageSource("/sdcard/multimedia/image/test.png", opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
}

/**
 * @tc.name: PngImageDecode003
 * @tc.desc: Create image source by correct png file path and wrong format hit.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourcePngTest, PngImageDecode003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by correct png file path and wrong format hit.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    opts.formatHint = "image/png";
    std::unique_ptr<ImageSource> imageSource =
        ImageSource::CreateImageSource("/sdcard/multimedia/image/test.png", opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
}

/**
 * @tc.name: PngImageDecode004
 * @tc.desc: Create image source by wrong png file path and default format hit.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourcePngTest, PngImageDecode004, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by wrong png file path and default format hit.
     * @tc.expected: step1. create image source error.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource =
        ImageSource::CreateImageSource("/sdcard/multimedia/image/png/test.png", opts, errorCode);
    ASSERT_EQ(errorCode, ERR_IMAGE_SOURCE_DATA);
    ASSERT_EQ(imageSource.get(), nullptr);
}

/**
 * @tc.name: PngImageDecode005
 * @tc.desc: Decode png image from buffer source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourcePngTest, PngImageDecode005, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by buffer source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool ret = ImageUtils::GetFileSize("/sdcard/multimedia/image/test.png", bufferSize);
    ASSERT_EQ(ret, true);
    uint8_t *buffer = (uint8_t *)malloc(bufferSize);
    ASSERT_NE(buffer, nullptr);
    ret = ReadFileToBuffer("/sdcard/multimedia/image/test.png", buffer, bufferSize);
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
    ImagePacker imagePacker;
    /**
     * @tc.steps: step3. compress the pixel map to jpeg file.
     * @tc.expected: step3. pack pixel map success and the jpeg compress file size equals to PNG_PACK_SIZE.
     */
    int64_t packSize = PackImage("/sdcard/multimedia/image/test_file.jpg", std::move(pixelMap));
    ASSERT_NE(packSize, 0);
    free(buffer);
}

/**
 * @tc.name: PngImageDecode006
 * @tc.desc: Decode png image from istream source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourcePngTest, PngImageDecode006, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by istream source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    std::unique_ptr<std::fstream> fs = std::make_unique<std::fstream>();
    fs->open("/sdcard/multimedia/image/test.png", std::fstream::binary | std::fstream::in);
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
    HiLog::Debug(LABEL_TEST, "create pixel map error code=%{public}u.", errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap.get(), nullptr);
    /**
     * @tc.steps: step3. compress the pixel map to png file.
     * @tc.expected: step3. pack pixel map success and the png compress file size equals to PNG_PACK_SIZE.
     */
    int64_t packSize = PackImage("/data/test_istream.png", std::move(pixelMap));
    ASSERT_NE(packSize, 0);
}

/**
 * @tc.name: PngImageDecode007
 * @tc.desc: Decode png image from incremental source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourcePngTest, PngImageDecode007, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by incremental source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool fileRet = ImageUtils::GetFileSize("/sdcard/multimedia/image/test.png", bufferSize);
    ASSERT_EQ(fileRet, true);
    uint8_t *buffer = (uint8_t *)malloc(bufferSize);
    ASSERT_NE(buffer, nullptr);
    fileRet = ReadFileToBuffer("/sdcard/multimedia/image/test.png", buffer, bufferSize);
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
        HiLog::Debug(LABEL_TEST, "updateOnceSize:%{public}u,updateSize:%{public}u,bufferSize:%{public}zu",
                     updateOnceSize, updateSize, bufferSize);
        incPixelMap->PromoteDecoding(decodeProgress);
        updateSize += updateOnceSize;
        usleep(DEFAULT_DELAY_UTIME);
    }
    incPixelMap->DetachFromDecoding();
    IncrementalDecodingStatus status = incPixelMap->GetDecodingStatus();
    ASSERT_EQ(status.decodingProgress, 100);

    /**
     * @tc.steps: step3. compress the pixel map to jpeg file.
     * @tc.expected: step3. pack pixel map success and the jpeg compress file size equals to PNG_PACK_SIZE.
     */
    int64_t packSize = PackImage("/sdcard/multimedia/image/test_file.jpg", std::move(incPixelMap));
    ASSERT_NE(packSize, 0);
    free(buffer);
}

/**
 * @tc.name: PngImageDecode008
 * @tc.desc: Decode png image multiple times from one ImageSource
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourcePngTest, PngImageDecode008, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by png file path.
     * @tc.expected: step1. create image source success.
     */
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource =
        ImageSource::CreateImageSource("/sdcard/multimedia/image/test.png", opts, errorCode);
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
     * @tc.steps: step4. compress the pixlel map to jpeg file.
     * @tc.expected: step4. pack pixel map success and the jpeg compress file size equals to PNG_PACK_SIZE.
     */
    int64_t packSize = PackImage("/sdcard/multimedia/image/test_png_file1.jpg", std::move(pixelMap1));
    ASSERT_NE(packSize, 0);
    packSize = PackImage("/sdcard/multimedia/image/test_png_file2.jpg", std::move(pixelMap2));
    ASSERT_NE(packSize, 0);
}

/**
 * @tc.name: PngImageDecode009
 * @tc.desc: Decode png image by incremental mode and then decode again by one-time mode.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourcePngTest, PngImageDecode009, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by incremental source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool fileRet = ImageUtils::GetFileSize("/sdcard/multimedia/image/test.png", bufferSize);
    ASSERT_EQ(fileRet, true);
    uint8_t *buffer = (uint8_t *)malloc(bufferSize);
    ASSERT_NE(buffer, nullptr);
    fileRet = ReadFileToBuffer("/sdcard/multimedia/image/test.png", buffer, bufferSize);
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
     * @tc.expected: step4. pack bitmap success and the jpeg compress file size equals to PNG_PACK_SIZE.
     */
    int64_t packSize = PackImage("/sdcard/multimedia/image/test_png_inc1.jpg", std::move(incPixelMap));
    ASSERT_NE(packSize, 0);
    packSize = PackImage("/sdcard/multimedia/image/test_png_onetime1.jpg", std::move(pixelMap1));
    ASSERT_NE(packSize, 0);
    free(buffer);
}

/**
 * @tc.name: PngImageDecode010
 * @tc.desc: Decode jpeg image by one-time mode and then decode again by incremental mode.
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourcePngTest, PngImageDecode010, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create image source by incremental source stream and default format hit
     * @tc.expected: step1. create image source success.
     */
    size_t bufferSize = 0;
    bool fileRet = ImageUtils::GetFileSize("/sdcard/multimedia/image/test.png", bufferSize);
    ASSERT_EQ(fileRet, true);
    uint8_t *buffer = (uint8_t *)malloc(bufferSize);
    ASSERT_NE(buffer, nullptr);
    fileRet = ReadFileToBuffer("/sdcard/multimedia/image/test.png", buffer, bufferSize);
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
        updateSize += updateOnceSize;
        usleep(DEFAULT_DELAY_UTIME);
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
     * @tc.expected: step4. pack pixel map success and the jpeg compress file size equals to PNG_PACK_SIZE.
     */
    int64_t packSize = PackImage("/sdcard/multimedia/image/test_png_inc2.jpg", std::move(incPixelMap));
    ASSERT_NE(packSize, 0);
    packSize = PackImage("/sdcard/multimedia/image/test_png_onetime2.jpg", std::move(pixelMap1));
    ASSERT_NE(packSize, 0);
    free(buffer);
}

/**
 * @tc.name: PngImageCrop001
 * @tc.desc: Crop png image from istream source stream
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourcePngTest, PngImageCrop001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create png image source by istream source stream and default format hit
     * @tc.expected: step1. create png image source success.
     */
    std::unique_ptr<std::fstream> fs = std::make_unique<std::fstream>();
    fs->open("/sdcard/multimedia/image/test.png", std::fstream::binary | std::fstream::in);
    bool isOpen = fs->is_open();
    ASSERT_EQ(isOpen, true);
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(std::move(fs), opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. crop png image source to pixel map crop options
     * @tc.expected: step2. crop png image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    decodeOpts.CropRect.top = 3;
    decodeOpts.CropRect.width = 200;
    decodeOpts.CropRect.left = 3;
    decodeOpts.CropRect.height = 40;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);
    HiLog::Debug(LABEL_TEST, "create pixel map error code=%{public}u.", errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(pixelMap.get(), nullptr);
    EXPECT_EQ(200, pixelMap->GetWidth());
    EXPECT_EQ(40, pixelMap->GetHeight());
}
/**
 * @tc.name: PngNinePatch001
 * @tc.desc: Decoding nine-patch picture
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourcePngTest, PngNinePatch001, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create png image source by istream source stream and default format hit
     * @tc.expected: step1. create png image source success.
     */
    std::unique_ptr<std::fstream> fs = std::make_unique<std::fstream>();
    fs->open("/sdcard/multimedia/image/test.9.png", std::fstream::binary | std::fstream::in);
    bool isOpen = fs->is_open();
    ASSERT_EQ(isOpen, true);
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(std::move(fs), opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. decode png image source to pixel map
     * @tc.expected: step2. decode png image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);

    /**
     * @tc.steps: step3. get png nine patch info
     * @tc.expected: step3. get png nine patch info success.
     */
    const NinePatchInfo &ninePatch = imageSource->GetNinePatchInfo();
    ASSERT_NE(ninePatch.ninePatch, nullptr);
    ASSERT_EQ(static_cast<int32_t>(ninePatch.patchSize), 84);
}

/**
 * @tc.name: PngNinePatch002
 * @tc.desc: Decoding non-nine-patch picture
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourcePngTest, PngNinePatch002, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create png image source by istream source stream and default format hit
     * @tc.expected: step1. create png image source success.
     */
    std::unique_ptr<std::fstream> fs = std::make_unique<std::fstream>();
    fs->open("/sdcard/multimedia/image/test.png", std::fstream::binary | std::fstream::in);
    bool isOpen = fs->is_open();
    ASSERT_EQ(isOpen, true);
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(std::move(fs), opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. decode png image source to pixel map
     * @tc.expected: step2. decode png image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);

    /**
     * @tc.steps: step3. get png nine patch info
     * @tc.expected: step3. get png nine patch info failed.
     */
    const NinePatchInfo &ninePatch = imageSource->GetNinePatchInfo();
    ASSERT_EQ(ninePatch.ninePatch, nullptr);
}

/**
 * @tc.name: PngNinePatch003
 * @tc.desc: Decoding nine-patch picture with scale and pixelformat convert
 * @tc.type: FUNC
 */
HWTEST_F(ImageSourcePngTest, PngNinePatch003, TestSize.Level3)
{
    /**
     * @tc.steps: step1. create png image source by istream source stream and default format hit
     * @tc.expected: step1. create png image source success.
     */
    std::unique_ptr<std::fstream> fs = std::make_unique<std::fstream>();
    fs->open("/sdcard/multimedia/image/test.9.png", std::fstream::binary | std::fstream::in);
    bool isOpen = fs->is_open();
    ASSERT_EQ(isOpen, true);
    uint32_t errorCode = 0;
    SourceOptions opts;
    std::unique_ptr<ImageSource> imageSource = ImageSource::CreateImageSource(std::move(fs), opts, errorCode);
    ASSERT_EQ(errorCode, SUCCESS);
    ASSERT_NE(imageSource.get(), nullptr);
    /**
     * @tc.steps: step2. scale and covert pixelformat png image source to pixel map
     * @tc.expected: step2. scale and covert pixelformat png image source to pixel map success.
     */
    DecodeOptions decodeOpts;
    decodeOpts.desiredSize = { 186, 160 };
    decodeOpts.desiredPixelFormat = PixelFormat::RGB_565;
    std::unique_ptr<PixelMap> pixelMap = imageSource->CreatePixelMap(decodeOpts, errorCode);

    /**
     * @tc.steps: step3. get png nine patch info
     * @tc.expected: step3. get png nine patch info success.
     */
    const NinePatchInfo &ninePatch = imageSource->GetNinePatchInfo();
    ASSERT_NE(ninePatch.ninePatch, nullptr);
    ASSERT_EQ(static_cast<int32_t>(ninePatch.patchSize), 84);
}
