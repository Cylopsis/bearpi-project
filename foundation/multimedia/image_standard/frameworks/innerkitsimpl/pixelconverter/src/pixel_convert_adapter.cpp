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

#include "pixel_convert_adapter.h"
#include "include/core/SkBitmap.h"
#include "include/core/SkCanvas.h"
#include "include/core/SkColor.h"
#include "include/core/SkColorSpace.h"
#include "include/core/SkImageInfo.h"
#include "include/core/SkPaint.h"
#include "include/core/SkPixmap.h"
#ifdef _WIN32
#include <iomanip>
#endif

namespace OHOS {
namespace Media {
using namespace OHOS::HiviewDFX;

static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "PixelConvertAdapter" };

static SkColorType PixelFormatConvert(const PixelFormat &pixelFormat)
{
    SkColorType colorType;
    switch (pixelFormat) {
        case PixelFormat::BGRA_8888:
            colorType = SkColorType::kBGRA_8888_SkColorType;
            break;
        case PixelFormat::RGBA_8888:
            colorType = SkColorType::kRGBA_8888_SkColorType;
            break;
        case PixelFormat::RGB_565:
            colorType = SkColorType::kRGB_565_SkColorType;
            break;
        case PixelFormat::ALPHA_8:
            colorType = SkColorType::kAlpha_8_SkColorType;
            break;
        default:
            colorType = SkColorType::kUnknown_SkColorType;
            break;
    }
    return colorType;
}

bool PixelConvertAdapter::WritePixelsConvert(const void *srcPixels, uint32_t srcRowBytes, const ImageInfo &srcInfo,
                                             void *dstPixels, const Position &dstPos, uint32_t dstRowBytes,
                                             const ImageInfo &dstInfo)
{
    // basic valid check, other parameters valid check in writePixels method
    if (srcPixels == nullptr || dstPixels == nullptr) {
        HiLog::Error(LABEL, "src or dst pixels invalid.");
        return false;
    }
    SkAlphaType srcAlphaType = static_cast<SkAlphaType>(srcInfo.alphaType);
    SkAlphaType dstAlphaType = static_cast<SkAlphaType>(dstInfo.alphaType);
    SkColorType srcColorType = PixelFormatConvert(srcInfo.pixelFormat);
    SkColorType dstColorType = PixelFormatConvert(dstInfo.pixelFormat);
    SkImageInfo srcImageInfo = SkImageInfo::Make(srcInfo.size.width, srcInfo.size.height, srcColorType, srcAlphaType);
    SkImageInfo dstImageInfo = SkImageInfo::Make(dstInfo.size.width, dstInfo.size.height, dstColorType, dstAlphaType);

    SkBitmap dstBitmap;
    SkPixmap srcPixmap(srcImageInfo, srcPixels, srcRowBytes);
    if (!dstBitmap.installPixels(dstImageInfo, dstPixels, dstRowBytes)) {
        HiLog::Error(LABEL, "WritePixelsConvert dst bitmap install pixels failed.");
        return false;
    }
    if (!dstBitmap.writePixels(srcPixmap, dstPos.x, dstPos.y)) {
        HiLog::Error(LABEL, "WritePixelsConvert dst bitmap write pixels by source failed.");
        return false;
    }

    return true;
}

bool PixelConvertAdapter::ReadPixelsConvert(const void *srcPixels, const Position &srcPos, uint32_t srcRowBytes,
                                            const ImageInfo &srcInfo, void *dstPixels, uint32_t dstRowBytes,
                                            const ImageInfo &dstInfo)
{
    // basic valid check, other parameters valid check in readPixels method
    if (srcPixels == nullptr || dstPixels == nullptr) {
        HiLog::Error(LABEL, "src or dst pixels invalid.");
        return false;
    }
    SkAlphaType srcAlphaType = static_cast<SkAlphaType>(srcInfo.alphaType);
    SkAlphaType dstAlphaType = static_cast<SkAlphaType>(dstInfo.alphaType);
    SkColorType srcColorType = PixelFormatConvert(srcInfo.pixelFormat);
    SkColorType dstColorType = PixelFormatConvert(dstInfo.pixelFormat);
    SkImageInfo srcImageInfo = SkImageInfo::Make(srcInfo.size.width, srcInfo.size.height, srcColorType, srcAlphaType);
    SkImageInfo dstImageInfo = SkImageInfo::Make(dstInfo.size.width, dstInfo.size.height, dstColorType, dstAlphaType);

    SkBitmap srcBitmap;
    if (!srcBitmap.installPixels(srcImageInfo, const_cast<void *>(srcPixels), srcRowBytes)) {
        HiLog::Error(LABEL, "ReadPixelsConvert src bitmap install pixels failed.");
        return false;
    }
    if (!srcBitmap.readPixels(dstImageInfo, dstPixels, dstRowBytes, srcPos.x, srcPos.y)) {
        HiLog::Error(LABEL, "ReadPixelsConvert read dst pixels from source failed.");
        return false;
    }
    return true;
}

bool PixelConvertAdapter::EraseBitmap(const void *srcPixels, uint32_t srcRowBytes, const ImageInfo &srcInfo,
                                      uint32_t color)
{
    if (srcPixels == nullptr) {
        HiLog::Error(LABEL, "srcPixels is null.");
        return false;
    }
    SkAlphaType srcAlphaType = static_cast<SkAlphaType>(srcInfo.alphaType);
    SkColorType srcColorType = PixelFormatConvert(srcInfo.pixelFormat);
    SkImageInfo srcImageInfo = SkImageInfo::Make(srcInfo.size.width, srcInfo.size.height, srcColorType, srcAlphaType);
    SkBitmap srcBitmap;
    if (!srcBitmap.installPixels(srcImageInfo, const_cast<void *>(srcPixels), srcRowBytes)) {
        HiLog::Error(LABEL, "ReadPixelsConvert src bitmap install pixels failed.");
        return false;
    }
    const SkColor4f skColor = SkColor4f::FromColor(color);
    SkPaint paint;
    paint.setColor4f(skColor, SkColorSpace::MakeSRGB().get());
    paint.setBlendMode(SkBlendMode::kSrc);
    SkCanvas canvas(srcBitmap);
    canvas.drawPaint(paint);
    return true;
}
} // namespace Media
} // namespace OHOS
