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

#ifndef PIXEL_CONVERT_ADAPTER_H
#define PIXEL_CONVERT_ADAPTER_H

#include "hilog/log.h"
#include "image_type.h"
#include "log_tags.h"

namespace OHOS {
namespace Media {
class PixelConvertAdapter {
public:
    static bool WritePixelsConvert(const void *srcPixels, uint32_t srcRowBytes, const ImageInfo &srcInfo,
                                   void *dstPixels, const Position &dstPos, uint32_t dstRowBytes,
                                   const ImageInfo &dstInfo);
    static bool ReadPixelsConvert(const void *srcPixels, const Position &srcPos, uint32_t srcRowBytes,
                                  const ImageInfo &srcInfo, void *dstPixels, uint32_t dstRowBytes,
                                  const ImageInfo &dstInfo);
    static bool EraseBitmap(const void *srcPixels, uint32_t srcRowBytes, const ImageInfo &srcInfo, uint32_t color);
};
} // namespace Media
} // namespace OHOS

#endif // PIXEL_CONVERT_ADAPTER_H