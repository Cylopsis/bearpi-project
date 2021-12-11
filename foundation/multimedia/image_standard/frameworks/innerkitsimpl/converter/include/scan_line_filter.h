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

#ifndef SCAN_LINE_FILTER_H
#define SCAN_LINE_FILTER_H

#include "image_type.h"
#include "pixel_convert.h"

namespace OHOS {
namespace Media {
enum class FilterRowType : int32_t { NON_REFERENCE_ROW = 0, NORMAL_REFERENCE_ROW = 1, LAST_REFERENCE_ROW = 2 };
class ScanlineFilter {
public:
    ScanlineFilter() = default;
    ScanlineFilter(const ScanlineFilter &) = delete;
    ScanlineFilter &operator=(const ScanlineFilter &) = delete;
    explicit ScanlineFilter(const PixelFormat &srcPixelFormat);
    ~ScanlineFilter() = default;
    FilterRowType GetFilterRowType(const int32_t rowNum);
    void SetSrcPixelFormat(const PixelFormat &srcPixelFormat);
    void SetSrcRegion(const Rect &region);
    void SetPixelConvert(const ImageInfo &srcImageInfo, const ImageInfo &dstImageInfo);
    uint32_t FilterLine(void *destRowPixels, uint32_t destRowBytes, const void *srcRowPixels);

private:
    bool ConvertPixels(void *destRowPixels, const uint8_t *startPixel, uint32_t reqPixelNum);
    int32_t srcBpp_ = 0;  // Bytes per pixel of source image.
    Rect srcRegion_;
    std::unique_ptr<PixelConvert> pixelConverter_ = nullptr;
    bool needPixelConvert_ = false;
};
} // namespace Media
} // namespace OHOS

#endif // SCAN_LINE_FILTER_H
