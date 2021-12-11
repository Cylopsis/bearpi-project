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

#include "image_log.h"
#include "image_utils.h"
#include "media_errors.h"
#include "scan_line_filter.h"
#ifndef _WIN32
#include "securec.h"
#else
#include "memory.h"
#endif

namespace OHOS {
namespace Media {
ScanlineFilter::ScanlineFilter(const PixelFormat &srcPixelFormat) : srcBpp_(ImageUtils::GetPixelBytes(srcPixelFormat))
{}

void ScanlineFilter::SetSrcPixelFormat(const PixelFormat &srcPixelFormat)
{
    srcBpp_ = ImageUtils::GetPixelBytes(srcPixelFormat);
}

FilterRowType ScanlineFilter::GetFilterRowType(const int32_t rowNum)
{
    if (rowNum < srcRegion_.top || (rowNum - srcRegion_.top) > srcRegion_.height) {
        return FilterRowType::NON_REFERENCE_ROW;
    }

    if ((rowNum - srcRegion_.top) == srcRegion_.height) {
        return FilterRowType::LAST_REFERENCE_ROW;
    }

    return FilterRowType::NORMAL_REFERENCE_ROW;
}

void ScanlineFilter::SetSrcRegion(const Rect &region)
{
    srcRegion_ = region;
}

// outer need judgement the src and dst imageInfo pixelFormat and alphaType
void ScanlineFilter::SetPixelConvert(const ImageInfo &srcImageInfo, const ImageInfo &dstImageInfo)
{
    needPixelConvert_ = true;
    pixelConverter_ = PixelConvert::Create(srcImageInfo, dstImageInfo);
}

uint32_t ScanlineFilter::FilterLine(void *destRowPixels, uint32_t destRowBytes, const void *srcRowPixels)
{
    if (destRowPixels == nullptr || srcRowPixels == nullptr) {
        IMAGE_LOGE("[ScanlineFilter]the src or dest pixel point is null.");
        return ERR_IMAGE_CROP;
    }
    auto startPixel = static_cast<const uint8_t *>(srcRowPixels) + srcRegion_.left * srcBpp_;
    if (startPixel == nullptr) {
        IMAGE_LOGE("[ScanlineFilter]the shift src pixel point is null.");
        return ERR_IMAGE_CROP;
    }
    if (!needPixelConvert_) {
        errno_t ret = memcpy_s(destRowPixels, destRowBytes, startPixel, srcRegion_.width * srcBpp_);
        if (ret != 0) {
            IMAGE_LOGE("[ScanlineFilter]memcpy failed,ret=%{public}d.", ret);
            return ERR_IMAGE_CROP;
        }
    } else {
        if (!ConvertPixels(destRowPixels, startPixel, srcRegion_.width)) {
            IMAGE_LOGE("[ScanlineFilter]convert color failed.");
            return ERR_IMAGE_COLOR_CONVERT;
        }
    }
    return SUCCESS;
}

bool ScanlineFilter::ConvertPixels(void *destRowPixels, const uint8_t *startPixel, uint32_t reqPixelNum)
{
    if (destRowPixels == nullptr || startPixel == nullptr) {
        IMAGE_LOGE("[ScanlineFilter]convert color failed, the destRowPixels or startPixel is null.");
        return false;
    }

    if (pixelConverter_ == nullptr) {
        IMAGE_LOGE("[ScanlineFilter]pixel converter is null");
        return false;
    }

    pixelConverter_->Convert(destRowPixels, startPixel, reqPixelNum);
    return true;
}
} // namespace Media
} // namespace OHOS
