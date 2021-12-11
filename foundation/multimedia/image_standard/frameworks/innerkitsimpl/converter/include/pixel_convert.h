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

#ifndef PIXEL_CONVERT_H
#define PIXEL_CONVERT_H

#include <cstdint>
#include <memory>
#include "hilog/log.h"
#include "image_type.h"
#include "log_tags.h"

namespace OHOS {
namespace Media {
enum class AlphaConvertType : uint32_t {
    NO_CONVERT = 0,
    PREMUL_CONVERT_UNPREMUL = 1,
    PREMUL_CONVERT_OPAQUE = 2,
    UNPREMUL_CONVERT_PREMUL = 3,
    UNPREMUL_CONVERT_OPAQUE = 4,
};

// now support AlphaConvertType
struct ProcFuncExtension {
    AlphaConvertType alphaConvertType;
};

// These values SHOULD be sync with image_type.h PixelFormat
constexpr uint32_t GRAY_BIT = 0x80000001; /* Tow value image, just white or black. */
constexpr uint32_t GRAY_ALPHA = 0x80000002;
constexpr uint32_t ARGB_8888 = 0x00000001;
constexpr uint32_t RGB_565 = 0x00000002;
constexpr uint32_t RGBA_8888 = 0x00000003;
constexpr uint32_t BGRA_8888 = 0x00000004;
constexpr uint32_t RGB_888 = 0x00000005;
constexpr uint32_t ALPHA_8 = 0x00000006; /* Gray image, 8 bit = 255 color. */
constexpr uint32_t ABGR_8888 = 0x00000008;
constexpr uint32_t BGR_888 = 0x40000002;
constexpr uint32_t RGB_161616 = 0x40000007;
constexpr uint32_t RGBA_16161616 = 0x40000008;

constexpr uint32_t CMKY = 0x0000000A;

constexpr uint32_t SIZE_1_BYTE = 0x00000001; /* a pixel has 8 bit = 1 byte */
constexpr uint32_t SIZE_2_BYTE = 0x00000002; /* a pixel has 16 bit = 2 byte */
constexpr uint32_t SIZE_3_BYTE = 0x00000003;
constexpr uint32_t SIZE_4_BYTE = 0x00000004;
constexpr uint32_t SIZE_6_BYTE = 0x00000006;
constexpr uint32_t SIZE_8_BYTE = 0x00000008;

constexpr uint8_t GRAYSCALE_WHITE = 0xFF;
constexpr uint8_t GRAYSCALE_BLACK = 0x00;
constexpr uint32_t ARGB_WHITE = 0xFFFFFFFF;
constexpr uint32_t ARGB_BLACK = 0xFF000000;
constexpr uint16_t RGB_WHITE = 0xFFFF;
constexpr uint16_t RGB_BLACK = 0x0000;

constexpr uint8_t ALPHA_OPAQUE = 0xFF;
constexpr uint8_t ALPHA_TRANSPARENT = 0x00;

constexpr uint32_t GET_8_BIT = 0x80;
constexpr uint32_t GET_1_BIT = 0x01;

constexpr uint32_t SHIFT_24_BIT = 0x18;
constexpr uint32_t SHIFT_16_BIT = 0x10;
constexpr uint32_t SHIFT_8_BIT = 0x08;
constexpr uint32_t SHIFT_11_BIT = 0x0B;
constexpr uint32_t SHIFT_5_BIT = 0x05;
constexpr uint32_t SHIFT_3_BIT = 0x03;
constexpr uint32_t SHIFT_2_BIT = 0x02;

constexpr uint8_t SHIFT_5_MASK = 0x1F;
constexpr uint8_t SHIFT_3_MASK = 0x07;

constexpr uint16_t MAX_15_BIT_VALUE = 0x7FFF;
constexpr float HALF_ONE = 0.5F;
static inline uint32_t Premul255(uint32_t colorComponent, uint32_t alpha)
{
    if (colorComponent > MAX_15_BIT_VALUE || alpha > MAX_15_BIT_VALUE) {
        return 0;
    }
    uint32_t product = colorComponent * alpha + GET_8_BIT;
    return ((product + (product >> SHIFT_8_BIT)) >> SHIFT_8_BIT);
}

static inline uint32_t Unpremul255(uint32_t colorComponent, uint32_t alpha)
{
    if (colorComponent > ALPHA_OPAQUE || alpha > ALPHA_OPAQUE) {
        return 0;
    }
    if (alpha == ALPHA_TRANSPARENT) {
        return ALPHA_TRANSPARENT;
    }
    if (alpha == ALPHA_OPAQUE) {
        return colorComponent;
    }
    uint32_t result = static_cast<float>(colorComponent) * ALPHA_OPAQUE / alpha + HALF_ONE;
    return (result > ALPHA_OPAQUE) ? ALPHA_OPAQUE : result;
}

using ProcFuncType = void (*)(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                              const ProcFuncExtension &extension);
class PixelConvert {
public:
    ~PixelConvert() = default;
    static std::unique_ptr<PixelConvert> Create(const ImageInfo &srcInfo, const ImageInfo &dstInfo);
    void Convert(void *destinationPixels, const uint8_t *sourcePixels, uint32_t sourcePixelsNum);

private:
    PixelConvert(ProcFuncType funcPtr, ProcFuncExtension extension, bool isNeedConvert);
    static AlphaConvertType GetAlphaConvertType(const AlphaType &srcType, const AlphaType &dstType);

    ProcFuncType procFunc_;
    ProcFuncExtension procFuncExtension_;
    bool isNeedConvert_ = true;
};
} // namespace Media
} // namespace OHOS

#endif /* PIXEL_CONVERT_H */
