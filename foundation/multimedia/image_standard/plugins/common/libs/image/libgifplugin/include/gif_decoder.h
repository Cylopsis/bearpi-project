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

#ifndef GIF_DECODER_H
#define GIF_DECODER_H

#include <cstdint>
#include <map>
#include <string>
#include "abs_image_decoder.h"
#include "gif_lib.h"
#include "hilog/log.h"
#include "log_tags.h"
#include "media_errors.h"
#include "nocopyable.h"
#include "plugin_class_base.h"
#ifndef _WIN32
#include "securec.h"
#else
#include "memory.h"
#endif

namespace OHOS {
namespace ImagePlugin {
static constexpr uint8_t PIXEL_FORMAT_BYTE_SIZE = 4;

class GifDecoder : public AbsImageDecoder, public OHOS::MultimediaPlugin::PluginClassBase {
public:
    GifDecoder();
    ~GifDecoder() override;
    void SetSource(InputDataStream &sourceStream) override;
    void Reset() override;
    uint32_t SetDecodeOptions(uint32_t index, const PixelDecodeOptions &opts, PlImageInfo &info) override;
    uint32_t Decode(uint32_t index, DecodeContext &context) override;
    uint32_t PromoteIncrementalDecode(uint32_t index, ProgDecodeContext &context) override;
    uint32_t GetTopLevelImageNum(uint32_t &num) override;
    uint32_t GetImageSize(uint32_t index, PlSize &size) override;
    uint32_t GetImagePropertyInt(uint32_t index, const std::string &key, int32_t &value) override;

private:
    static int32_t InputStreamReader(GifFileType *gif, GifByteType *bytes, int32_t size);
    DISALLOW_COPY_AND_MOVE(GifDecoder);
    uint32_t CheckIndex(uint32_t index);
    uint32_t OverlapFrame(uint32_t startIndex, uint32_t endIndex);
    uint32_t RedirectOutputBuffer(DecodeContext &context);
    void GetTransparentAndDisposal(uint32_t index, int32_t &transparentColor, int32_t &disposalMode);
    GraphicsControlBlock GetGraphicsControlBlock(uint32_t index);
    uint32_t PaddingBgColor(const SavedImage *savedImage);
    bool IsFramePreviousCoveredCurrent(const SavedImage *preSavedImage, const SavedImage *curSavedImage);
    uint32_t PaddingData(const SavedImage *savedImage, int32_t transparentColor);
    void CopyLine(const GifByteType *srcFrame, uint32_t *dstFrame, int32_t frameWidth, int32_t transparentColor,
                  const ColorMapObject *colorMap);
    uint32_t GetPixelColor(uint32_t red, uint32_t green, uint32_t blue, uint32_t alpha);
    void ParseBgColor();
    uint32_t UpdateGifFileType(int32_t updateFrameIndex);
    uint32_t CreateGifFileTypeIfNotExist();
    uint32_t ParseFrameDetail();
    uint32_t SetSavedImageRasterBits(SavedImage *saveImagePtr, int32_t frameIndex, uint64_t imageSize,
                                     int32_t imageWidth, int32_t imageHeight);
    uint32_t ParseFrameExtension();
    uint32_t AllocateLocalPixelMapBuffer();
    void FreeLocalPixelMapBuffer();
    uint32_t DisposeBackground(uint32_t frameIndex, const SavedImage *curSavedImage);
    uint32_t GetImageDelayTime(uint32_t index, int32_t &value);
    uint32_t GetImageLoopCount(uint32_t index, int32_t &value);

    InputDataStream *inputStreamPtr_ = nullptr;
    GifFileType *gifPtr_ = nullptr;
    uint32_t *localPixelMapBuffer_ = nullptr;
    uint32_t bgColor_ = 0;
    int32_t lastPixelMapIndex_ = -1;
    bool isLoadAllFrame_ = false;
    int32_t savedFrameIndex_ = -1;
};
} // namespace ImagePlugin
} // namespace OHOS

#endif // GIF_DECODER_H
