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

#ifndef POST_PROC_H
#define POST_PROC_H

#include <vector>
#include "basic_transformer.h"
#include "image_type.h"
#include "pixel_map.h"
#include "scan_line_filter.h"

namespace OHOS {
namespace Media {
enum class CropValue : int32_t { INVALID, VALID, NOCROP };

class PostProc {
public:
    uint32_t DecodePostProc(const DecodeOptions &opts, PixelMap &pixelMap,
                            FinalOutputStep finalOutputStep = FinalOutputStep::NO_CHANGE);
    uint32_t ConvertProc(const Rect &cropRect, ImageInfo &dstImageInfo, PixelMap &pixelMap, ImageInfo &srcImageInfo);
    static bool IsHasCrop(const Rect &rect);
    bool HasPixelConvert(const ImageInfo &srcImageInfo, ImageInfo &dstImageInfo);
    bool RotatePixelMap(float rotateDegrees, PixelMap &pixelMap);
    bool ScalePixelMap(const Size &size, PixelMap &pixelMap);
    bool ScalePixelMap(float scaleX, float scaleY, PixelMap &pixelMap);
    bool CenterScale(const Size &size, PixelMap &pixelMap);
    static CropValue GetCropValue(const Rect &rect, const Size &size);

private:
    static uint8_t *AllocSharedMemory(const Size &size, const uint64_t bufferSize, int &fd);
    uint32_t NeedScanlineFilter(const Rect &cropRect, const Size &srcSize, const bool &hasPixelConvert);
    void GetDstImageInfo(const DecodeOptions &opts, PixelMap &pixelMap,
                         ImageInfo srcImageInfo, ImageInfo &dstImageInfo);
    uint32_t PixelConvertProc(ImageInfo &dstImageInfo, PixelMap &pixelMap, ImageInfo &srcImageInfo);
    uint32_t AllocBuffer(ImageInfo imageInfo, uint8_t **resultData, uint64_t &dataSize, int &fd);
    bool AllocHeapBuffer(uint64_t bufferSize, uint8_t **buffer);
    void ReleaseBuffer(AllocatorType allocatorType, int fd, uint64_t dataSize, uint8_t **buffer);
    bool Transform(BasicTransformer &trans, const PixmapInfo &input, PixelMap &pixelMap);
    void ConvertPixelMapToPixmapInfo(PixelMap &pixelMap, PixmapInfo &pixmapInfo);
    void SetScanlineCropAndConvert(const Rect &cropRect, ImageInfo &dstImageInfo, ImageInfo &srcImageInfo,
                                   ScanlineFilter &scanlineFilter, bool hasPixelConvert);
    bool CenterDisplay(PixelMap &pixelMap, int32_t srcWidth, int32_t srcHeight, int32_t targetWidth,
                       int32_t targetHeight);
    uint32_t CheckScanlineFilter(const Rect &cropRect, ImageInfo &dstImageInfo, PixelMap &pixelMap,
                                 int32_t pixelBytes, ScanlineFilter &scanlineFilter);
    DecodeOptions decodeOpts_;
};
} // namespace Media
} // namespace OHOS

#endif // POST_PROC_H
