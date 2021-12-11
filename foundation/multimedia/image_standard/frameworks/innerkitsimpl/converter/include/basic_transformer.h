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

#ifndef BASIC_TRANSFORMER_H
#define BASIC_TRANSFORMER_H

#include <algorithm>
#include <string>
#include "image_log.h"
#include "image_type.h"
#include "matrix.h"

namespace OHOS {
namespace Media {
static constexpr uint32_t IMAGE_SUCCESS = 0;                                     // success
static constexpr uint32_t IMAGE_BASE_ERROR = 1000;                               // base error
static constexpr uint32_t ERR_IMAGE_GENERAL_ERROR = IMAGE_BASE_ERROR + 1;        // general error
static constexpr uint32_t ERR_IMAGE_INVALID_PIXEL = IMAGE_BASE_ERROR + 2;        // invalid pixel
static constexpr uint32_t ERR_IMAGE_MATRIX_NOT_INVERT = IMAGE_BASE_ERROR + 3;    // matrix can not invert
static constexpr uint32_t ERR_IMAGE_ALLOC_MEMORY_FAILED = IMAGE_BASE_ERROR + 4;  // alloc memory failed

static constexpr float FHALF = 0.5f;
static constexpr uint32_t BASIC = 1 << 16;
static constexpr uint32_t HALF_BASIC = 1 << 15;
static constexpr float MULTI_65536 = 65536.0f;
static constexpr uint32_t SUB_VALUE_SHIFT = 12;
static constexpr uint8_t COLOR_DEFAULT = 0;
static constexpr int32_t RGB888_BYTE = 3;

static inline bool CheckOutOfRange(const Point &pt, const Size &size)
{
    if (pt.x >= 0 && pt.x < size.width && pt.y >= 0 && pt.y < size.height) {
        return false;
    }
    return true;
}

static inline int32_t ClampMax(int value, int max)
{
    if (value > max) {
        value = max;
    }
    return (value > 0) ? value : 0;
}

static inline uint32_t GetSubValue(int32_t value)
{
    // In order to speed up the calculation, use offset
    return ((value >> SUB_VALUE_SHIFT) & 0xF);
}

struct PixmapInfo {
    ImageInfo imageInfo;
    uint8_t *data = nullptr;
    uint32_t bufferSize = 0;
    bool isAutoDestruct = true;
    PixmapInfo(){};

    ~PixmapInfo()
    {
        if (isAutoDestruct) {
            if (data != nullptr) {
                free(data);
                data = nullptr;
            }
        }
    }

    explicit PixmapInfo(bool isAuto)
    {
        isAutoDestruct = isAuto;
    }
    explicit PixmapInfo(const PixmapInfo &src)
    {
        Init(src);
    }

    void Init(const PixmapInfo &src)
    {
        imageInfo = src.imageInfo;
        data = nullptr;
        bufferSize = 0;
    }

    void Destroy()
    {
        if (data != nullptr) {
            free(data);
            data = nullptr;
        }
    }

    void PrintPixmapInfo(const std::string &strFlag) const
    {
        IMAGE_LOGD("[PixmapInfo][%{public}s][width, height:%{public}d, %{public}d]\
                    [bufferSize:%{public}u][pixelFormat:%{public}d].",
                   strFlag.c_str(), imageInfo.size.width, imageInfo.size.height, bufferSize,
                   static_cast<int32_t>(imageInfo.pixelFormat));
    }
};

class BasicTransformer {
public:
    using AllocateMem = uint8_t *(*)(const Size &size, const uint64_t bufferSize, int &fd);

    BasicTransformer()
    {
        ResetParam();
    };
    ~BasicTransformer(){};

    // Reset pixel map info transform param, back to the original state
    void ResetParam();

    // Reserved interface
    void SetTranslateParam(const float tx, const float ty);

    void SetScaleParam(const float sx, const float sy);

    /** Set rotates param by degrees about a point at (px, py). Positive degrees rotates
     * clockwise.
     *
     * @param degrees  amount to rotate, in degrees
     * @param px       x-axis value of the point to rotate about
     * @param py       y-axis value of the point to rotate about
     */
    void SetRotateParam(const float degrees, const float px = 0.0f, const float py = 0.0f);

    /**
     * Transform pixel map info. before transform, you should set pixel transform param first.
     * @param inPixmap The input pixel map info
     * @param outPixmap The output pixel map info, the pixelFormat and colorSpace same as the inPixmap
     * @param allocate This is func pointer, if it is null, this function will new heap memory,
     * so you must active release memory, if it is not null, that means you need allocate memory by yourself,
     * so you should invoke GetDstWH function to get the dest width and height,
     * then fill to outPixmap's width and height.
     * @return the error no
     */
    uint32_t TransformPixmap(const PixmapInfo &inPixmap, PixmapInfo &outPixmap, AllocateMem allocate = nullptr);

    void GetDstDimension(const Size &srcSize, Size &dstSize);

private:
    struct AroundPixels {
        uint32_t color00 = 0;
        uint32_t color01 = 0;
        uint32_t color10 = 0;
        uint32_t color11 = 0;
    };
    struct AroundPos {
        uint32_t x0 = 0;
        uint32_t x1 = 0;
        uint32_t y0 = 0;
        uint32_t y1 = 0;
    };

    uint32_t RightShift16Bit(uint32_t num, int32_t maxNum);

    void GetRotateDimension(Matrix::CalcXYProc fInvProc, const Size &srcSize, Size &dstSize);

    bool DrawPixelmap(const PixmapInfo &pixmapInfo, const int32_t pixelBytes, const Size &size, uint8_t *data);

    bool CheckAllocateBuffer(PixmapInfo &outPixmap, AllocateMem allocate, int &fd, uint64_t &bufferSize, Size &dstSize);

    void BilinearProc(const Point &pt, const PixmapInfo &pixmapInfo, const uint32_t rb, const int32_t shiftBytes,
                      uint8_t *data);
    void GetAroundPixelRGB565(const AroundPos aroundPos, uint8_t *data, uint32_t rb, AroundPixels &aroundPixels);

    void GetAroundPixelRGB888(const AroundPos aroundPos, uint8_t *data, uint32_t rb, AroundPixels &aroundPixels);

    void GetAroundPixelRGBA(const AroundPos aroundPos, uint8_t *data, uint32_t rb, AroundPixels &aroundPixels);

    void GetAroundPixelALPHA8(const AroundPos aroundPos, uint8_t *data, uint32_t rb, AroundPixels &aroundPixels);

    /* Calculate the target pixel based on the pixels of 4 nearby points.
     * Fill in new pixels with formula
     * f(i+u,j+v) = (1-u)(1-v)f(i,j) + (1-u)vf(i,j+1) + u(1-v)f(i+1,j) + uvf(i+1,j+1)
     */
    uint32_t FilterProc(const uint32_t subx, const uint32_t suby, const AroundPixels &aroundPixels);

    void ReleaseBuffer(AllocatorType allocatorType, int fd, int dataSize, uint8_t *buffer);

    Matrix matrix_;
    float minX_ = 0.0f;
    float minY_ = 0.0f;
};
} // namespace Media
} // namespace OHOS
#endif // BASIC_TRANSFORMER_H
