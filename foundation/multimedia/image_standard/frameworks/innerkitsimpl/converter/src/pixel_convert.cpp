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

#include <map>
#include <mutex>
#include "pixel_convert.h"

namespace OHOS {
namespace Media {
using namespace std;
using namespace OHOS::HiviewDFX;
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "PixelConvert" };
#if __BYTE_ORDER == __LITTLE_ENDIAN
constexpr bool IS_LITTLE_ENDIAN = true;
#else
constexpr bool IS_LITTLE_ENDIAN = false;
#endif

static void AlphaTypeConvertOnRGB(uint32_t &A, uint32_t &R, uint32_t &G, uint32_t &B,
                                  const ProcFuncExtension &extension)
{
    switch (extension.alphaConvertType) {
        case AlphaConvertType::PREMUL_CONVERT_UNPREMUL:
            R = Unpremul255(R, A);
            G = Unpremul255(G, A);
            B = Unpremul255(B, A);
            break;
        case AlphaConvertType::PREMUL_CONVERT_OPAQUE:
            R = Unpremul255(R, A);
            G = Unpremul255(G, A);
            B = Unpremul255(B, A);
            A = ALPHA_OPAQUE;
            break;
        case AlphaConvertType::UNPREMUL_CONVERT_PREMUL:
            R = Premul255(R, A);
            G = Premul255(G, A);
            B = Premul255(B, A);
            break;
        case AlphaConvertType::UNPREMUL_CONVERT_OPAQUE:
            A = ALPHA_OPAQUE;
            break;
        default:
            break;
    }
}

static uint32_t FillARGB8888(uint32_t A, uint32_t R, uint32_t G, uint32_t B)
{
    if (IS_LITTLE_ENDIAN) {
        return ((B << SHIFT_24_BIT) | (G << SHIFT_16_BIT) | (R << SHIFT_8_BIT) | A);
    }
    return ((A << SHIFT_24_BIT) | (R << SHIFT_16_BIT) | (G << SHIFT_8_BIT) | B);
}

static uint32_t FillABGR8888(uint32_t A, uint32_t B, uint32_t G, uint32_t R)
{
    if (IS_LITTLE_ENDIAN) {
        return ((R << SHIFT_24_BIT) | (G << SHIFT_16_BIT) | (B << SHIFT_8_BIT) | A);
    }
    return ((A << SHIFT_24_BIT) | (B << SHIFT_16_BIT) | (G << SHIFT_8_BIT) | R);
}

static uint32_t FillRGBA8888(uint32_t R, uint32_t G, uint32_t B, uint32_t A)
{
    if (IS_LITTLE_ENDIAN) {
        return ((A << SHIFT_24_BIT) | (B << SHIFT_16_BIT) | (G << SHIFT_8_BIT) | R);
    }
    return ((R << SHIFT_24_BIT) | (G << SHIFT_16_BIT) | (B << SHIFT_8_BIT) | A);
}

static uint32_t FillBGRA8888(uint32_t B, uint32_t G, uint32_t R, uint32_t A)
{
    if (IS_LITTLE_ENDIAN) {
        return ((A << SHIFT_24_BIT) | (R << SHIFT_16_BIT) | (G << SHIFT_8_BIT) | B);
    }
    return ((B << SHIFT_24_BIT) | (G << SHIFT_16_BIT) | (R << SHIFT_8_BIT) | A);
}

static uint16_t FillRGB565(uint32_t R, uint32_t G, uint32_t B)
{
    if (IS_LITTLE_ENDIAN) {
        return ((B << SHIFT_11_BIT) | (G << SHIFT_5_BIT) | R);
    }
    return ((R << SHIFT_11_BIT) | (G << SHIFT_5_BIT) | B);
}

constexpr uint8_t BYTE_BITS = 8;
constexpr uint8_t BYTE_BITS_MAX_INDEX = 7;
template<typename T>
static void BitConvert(T *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth, uint32_t white,
                       uint32_t black)
{
    destinationRow[0] = (*sourceRow & GET_8_BIT) ? white : black;
    uint32_t bitIndex = 0;
    uint8_t currentSource = 0;
    /*
     * 1 byte = 8 bit
     * 7: 8 bit index
     */
    for (uint32_t i = 1; i < sourceWidth; i++) {
        bitIndex = i % BYTE_BITS;
        currentSource = *(sourceRow + i / BYTE_BITS);
        destinationRow[i] = ((currentSource >> (BYTE_BITS_MAX_INDEX - bitIndex)) & GET_1_BIT) ? white : black;
    }
}

static void BitConvertGray(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                           const ProcFuncExtension &extension)
{
    uint8_t *newDestinationRow = static_cast<uint8_t *>(destinationRow);
    BitConvert(newDestinationRow, sourceRow, sourceWidth, GRAYSCALE_WHITE, GRAYSCALE_BLACK);
}

static void BitConvertARGB8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                               const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    BitConvert(newDestinationRow, sourceRow, sourceWidth, ARGB_WHITE, ARGB_BLACK);
}

static void BitConvertRGB565(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                             const ProcFuncExtension &extension)
{
    uint16_t *newDestinationRow = static_cast<uint16_t *>(destinationRow);
    BitConvert(newDestinationRow, sourceRow, sourceWidth, RGB_WHITE, RGB_BLACK);
}

constexpr uint32_t BRANCH_GRAY_TO_ARGB8888 = 0x00000001;
constexpr uint32_t BRANCH_GRAY_TO_RGB565 = 0x00000002;
template<typename T>
static void GrayConvert(T *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth, uint32_t branch)
{
    for (uint32_t i = 0; i < sourceWidth; i++) {
        uint32_t R = sourceRow[i];
        uint32_t G = sourceRow[i];
        uint32_t B = sourceRow[i];
        if (branch == BRANCH_GRAY_TO_ARGB8888) {
            uint32_t A = ALPHA_OPAQUE;
            destinationRow[i] = ((A << SHIFT_24_BIT) | (R << SHIFT_16_BIT) | (G << SHIFT_8_BIT) | B);
        } else if (branch == BRANCH_GRAY_TO_RGB565) {
            R = R >> SHIFT_3_BIT;
            G = G >> SHIFT_2_BIT;
            B = B >> SHIFT_3_BIT;
            destinationRow[i] = ((R << SHIFT_11_BIT) | (G << SHIFT_5_BIT) | B);
        } else {
            break;
        }
    }
}

static void GrayConvertARGB8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    GrayConvert(newDestinationRow, sourceRow, sourceWidth, BRANCH_GRAY_TO_ARGB8888);
}

static void GrayConvertRGB565(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                              const ProcFuncExtension &extension)
{
    uint16_t *newDestinationRow = static_cast<uint16_t *>(destinationRow);
    GrayConvert(newDestinationRow, sourceRow, sourceWidth, BRANCH_GRAY_TO_RGB565);
}

constexpr uint32_t BRANCH_ARGB8888 = 0x10000001;
constexpr uint32_t BRANCH_ALPHA = 0x10000002;
template<typename T>
static void GrayAlphaConvert(T *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth, uint32_t branch,
                             const ProcFuncExtension &extension)
{
    for (uint32_t i = 0; i < sourceWidth; i++) {
        uint32_t A = sourceRow[1];
        uint32_t R = sourceRow[0];
        uint32_t G = sourceRow[0];
        uint32_t B = sourceRow[0];
        AlphaTypeConvertOnRGB(A, R, G, B, extension);
        if (branch == BRANCH_ARGB8888) {
            destinationRow[i] = FillARGB8888(A, R, G, B);
        } else if (branch == BRANCH_ALPHA) {
            destinationRow[i] = A;
        } else {
            break;
        }
        sourceRow += SIZE_2_BYTE;
    }
}

static void GrayAlphaConvertARGB8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                     const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    GrayAlphaConvert(newDestinationRow, sourceRow, sourceWidth, BRANCH_ARGB8888, extension);
}

static void GrayAlphaConvertAlpha(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                  const ProcFuncExtension &extension)
{
    uint8_t *newDestinationRow = static_cast<uint8_t *>(destinationRow);
    GrayAlphaConvert(newDestinationRow, sourceRow, sourceWidth, BRANCH_ALPHA, extension);
}

constexpr uint32_t BRANCH_BGR888_TO_ARGB8888 = 0x20000001;
constexpr uint32_t BRANCH_BGR888_TO_RGBA8888 = 0x20000002;
constexpr uint32_t BRANCH_BGR888_TO_BGRA8888 = 0x20000003;
constexpr uint32_t BRANCH_BGR888_TO_RGB565 = 0x20000004;
template<typename T>
static void BGR888Convert(T *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth, uint32_t branch)
{
    for (uint32_t i = 0; i < sourceWidth; i++) {
        uint32_t R = sourceRow[2];
        uint32_t G = sourceRow[1];
        uint32_t B = sourceRow[0];
        uint32_t A = ALPHA_OPAQUE;
        if (branch == BRANCH_BGR888_TO_ARGB8888) {
            destinationRow[i] = FillARGB8888(A, R, G, B);
        } else if (branch == BRANCH_BGR888_TO_RGBA8888) {
            destinationRow[i] = FillRGBA8888(R, G, B, A);
        } else if (branch == BRANCH_BGR888_TO_BGRA8888) {
            destinationRow[i] = FillBGRA8888(B, G, R, A);
        } else if (branch == BRANCH_BGR888_TO_RGB565) {
            R = R >> SHIFT_3_BIT;
            G = G >> SHIFT_2_BIT;
            B = B >> SHIFT_3_BIT;
            destinationRow[i] = ((B << SHIFT_11_BIT) | (G << SHIFT_5_BIT) | R);
        } else {
            break;
        }
        sourceRow += SIZE_3_BYTE;
    }
}

static void BGR888ConvertARGB8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                  const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    BGR888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_BGR888_TO_ARGB8888);
}

static void BGR888ConvertRGBA8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                  const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    BGR888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_BGR888_TO_RGBA8888);
}

static void BGR888ConvertBGRA8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                  const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    BGR888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_BGR888_TO_BGRA8888);
}

static void BGR888ConvertRGB565(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                const ProcFuncExtension &extension)
{
    uint16_t *newDestinationRow = static_cast<uint16_t *>(destinationRow);
    BGR888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_BGR888_TO_RGB565);
}

constexpr uint32_t BRANCH_RGB888_TO_ARGB8888 = 0x30000001;
constexpr uint32_t BRANCH_RGB888_TO_RGBA8888 = 0x30000002;
constexpr uint32_t BRANCH_RGB888_TO_BGRA8888 = 0x30000003;
constexpr uint32_t BRANCH_RGB888_TO_RGB565 = 0x30000004;
template<typename T>
static void RGB888Convert(T *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth, uint32_t branch)
{
    for (uint32_t i = 0; i < sourceWidth; i++) {
        uint32_t R = sourceRow[0];
        uint32_t G = sourceRow[1];
        uint32_t B = sourceRow[2];
        uint32_t A = ALPHA_OPAQUE;
        if (branch == BRANCH_RGB888_TO_ARGB8888) {
            destinationRow[i] = FillARGB8888(A, R, G, B);
        } else if (branch == BRANCH_RGB888_TO_RGBA8888) {
            destinationRow[i] = FillRGBA8888(R, G, B, A);
        } else if (branch == BRANCH_RGB888_TO_BGRA8888) {
            destinationRow[i] = FillBGRA8888(B, G, R, A);
        } else if (branch == BRANCH_RGB888_TO_RGB565) {
            R = R >> SHIFT_3_BIT;
            G = G >> SHIFT_2_BIT;
            B = B >> SHIFT_3_BIT;
            destinationRow[i] = FillRGB565(R, G, B);
        } else {
            break;
        }
        sourceRow += SIZE_3_BYTE;
    }
}
static void RGB888ConvertARGB8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                  const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    RGB888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGB888_TO_ARGB8888);
}

static void RGB888ConvertRGBA8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                  const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    RGB888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGB888_TO_RGBA8888);
}

static void RGB888ConvertBGRA8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                  const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    RGB888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGB888_TO_BGRA8888);
}

static void RGB888ConvertRGB565(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                const ProcFuncExtension &extension)
{
    uint16_t *newDestinationRow = static_cast<uint16_t *>(destinationRow);
    RGB888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGB888_TO_RGB565);
}
constexpr uint32_t BRANCH_RGBA8888_TO_RGBA8888_ALPHA = 0x40000001;
constexpr uint32_t BRANCH_RGBA8888_TO_ARGB8888 = 0x40000002;
constexpr uint32_t BRANCH_RGBA8888_TO_BGRA8888 = 0x40000003;
constexpr uint32_t BRANCH_RGBA8888_TO_RGB565 = 0x40000004;
template<typename T>
static void RGBA8888Convert(T *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth, uint32_t branch,
                            const ProcFuncExtension &extension)
{
    for (uint32_t i = 0; i < sourceWidth; i++) {
        uint32_t R = sourceRow[0];
        uint32_t G = sourceRow[1];
        uint32_t B = sourceRow[2];
        uint32_t A = sourceRow[3];
        AlphaTypeConvertOnRGB(A, R, G, B, extension);
        if (branch == BRANCH_RGBA8888_TO_RGBA8888_ALPHA) {
            destinationRow[i] = FillRGBA8888(R, G, B, A);
        } else if (branch == BRANCH_RGBA8888_TO_ARGB8888) {
            destinationRow[i] = FillARGB8888(A, R, G, B);
        } else if (branch == BRANCH_RGBA8888_TO_BGRA8888) {
            destinationRow[i] = FillBGRA8888(B, G, R, A);
        } else if (branch == BRANCH_RGBA8888_TO_RGB565) {
            R = R >> SHIFT_3_BIT;
            G = G >> SHIFT_2_BIT;
            B = B >> SHIFT_3_BIT;
            destinationRow[i] = FillRGB565(R, G, B);
        } else {
            break;
        }
        sourceRow += SIZE_4_BYTE;
    }
}

static void RGBA8888ConvertRGBA8888Alpha(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                         const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    RGBA8888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGBA8888_TO_RGBA8888_ALPHA, extension);
}

static void RGBA8888ConvertARGB8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                    const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    RGBA8888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGBA8888_TO_ARGB8888, extension);
}
static void RGBA8888ConvertBGRA8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                    const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    RGBA8888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGBA8888_TO_BGRA8888, extension);
}

static void RGBA8888ConvertRGB565(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                  const ProcFuncExtension &extension)
{
    uint16_t *newDestinationRow = static_cast<uint16_t *>(destinationRow);
    RGBA8888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGBA8888_TO_RGB565, extension);
}

constexpr uint32_t BRANCH_BGRA8888_TO_BGRA8888_ALPHA = 0x80000001;
constexpr uint32_t BRANCH_BGRA8888_TO_ARGB8888 = 0x80000002;
constexpr uint32_t BRANCH_BGRA8888_TO_RGBA8888 = 0x80000003;
constexpr uint32_t BRANCH_BGRA8888_TO_RGB565 = 0x80000004;
template<typename T>
static void BGRA8888Convert(T *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth, uint32_t branch,
                            const ProcFuncExtension &extension)
{
    for (uint32_t i = 0; i < sourceWidth; i++) {
        uint32_t B = sourceRow[0];
        uint32_t G = sourceRow[1];
        uint32_t R = sourceRow[2];
        uint32_t A = sourceRow[3];
        AlphaTypeConvertOnRGB(A, R, G, B, extension);
        if (branch == BRANCH_BGRA8888_TO_BGRA8888_ALPHA) {
            destinationRow[i] = FillBGRA8888(B, G, R, A);
        } else if (branch == BRANCH_BGRA8888_TO_ARGB8888) {
            destinationRow[i] = FillARGB8888(A, R, G, B);
        } else if (branch == BRANCH_BGRA8888_TO_RGBA8888) {
            destinationRow[i] = FillRGBA8888(R, G, B, A);
        } else if (branch == BRANCH_BGRA8888_TO_RGB565) {
            R = R >> SHIFT_3_BIT;
            G = G >> SHIFT_2_BIT;
            B = B >> SHIFT_3_BIT;
            destinationRow[i] = FillRGB565(R, G, B);
        } else {
            break;
        }
        sourceRow += SIZE_4_BYTE;
    }
}

static void BGRA8888ConvertBGRA8888Alpha(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                         const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    BGRA8888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_BGRA8888_TO_BGRA8888_ALPHA, extension);
}

static void BGRA8888ConvertARGB8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                    const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    BGRA8888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_BGRA8888_TO_ARGB8888, extension);
}

static void BGRA8888ConvertRGBA8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                    const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    BGRA8888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_BGRA8888_TO_RGBA8888, extension);
}

static void BGRA8888ConvertRGB565(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                  const ProcFuncExtension &extension)
{
    uint16_t *newDestinationRow = static_cast<uint16_t *>(destinationRow);
    BGRA8888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_BGRA8888_TO_RGB565, extension);
}

constexpr uint32_t BRANCH_ARGB8888_TO_ARGB8888_ALPHA = 0x90000001;
constexpr uint32_t BRANCH_ARGB8888_TO_RGBA8888 = 0x90000002;
constexpr uint32_t BRANCH_ARGB8888_TO_BGRA8888 = 0x90000003;
constexpr uint32_t BRANCH_ARGB8888_TO_RGB565 = 0x90000004;
template<typename T>
static void ARGB8888Convert(T *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth, uint32_t branch,
                            const ProcFuncExtension &extension)
{
    for (uint32_t i = 0; i < sourceWidth; i++) {
        uint32_t A = sourceRow[0];
        uint32_t R = sourceRow[1];
        uint32_t G = sourceRow[2];
        uint32_t B = sourceRow[3];
        AlphaTypeConvertOnRGB(A, R, G, B, extension);
        if (branch == BRANCH_ARGB8888_TO_ARGB8888_ALPHA) {
            destinationRow[i] = FillARGB8888(A, R, G, B);
        } else if (branch == BRANCH_ARGB8888_TO_RGBA8888) {
            destinationRow[i] = FillRGBA8888(R, G, B, A);
        } else if (branch == BRANCH_ARGB8888_TO_BGRA8888) {
            destinationRow[i] = FillBGRA8888(B, G, R, A);
        } else if (branch == BRANCH_ARGB8888_TO_RGB565) {
            R = R >> SHIFT_3_BIT;
            G = G >> SHIFT_2_BIT;
            B = B >> SHIFT_3_BIT;
            destinationRow[i] = FillRGB565(R, G, B);
        } else {
            break;
        }
        sourceRow += SIZE_4_BYTE;
    }
}

static void ARGB8888ConvertARGB8888Alpha(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                         const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    ARGB8888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_ARGB8888_TO_ARGB8888_ALPHA, extension);
}

static void ARGB8888ConvertRGBA8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                    const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    ARGB8888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_ARGB8888_TO_RGBA8888, extension);
}

static void ARGB8888ConvertBGRA8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                    const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    ARGB8888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_ARGB8888_TO_BGRA8888, extension);
}

static void ARGB8888ConvertRGB565(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                  const ProcFuncExtension &extension)
{
    uint16_t *newDestinationRow = static_cast<uint16_t *>(destinationRow);
    ARGB8888Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_ARGB8888_TO_RGB565, extension);
}

constexpr uint32_t BRANCH_RGB161616_TO_ARGB8888 = 0x50000001;
constexpr uint32_t BRANCH_RGB161616_TO_ABGR8888 = 0x50000002;
constexpr uint32_t BRANCH_RGB161616_TO_RGBA8888 = 0x50000003;
constexpr uint32_t BRANCH_RGB161616_TO_BGRA8888 = 0x50000004;
constexpr uint32_t BRANCH_RGB161616_TO_RGB565 = 0x50000005;
template<typename T>
static void RGB161616Convert(T *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth, uint32_t branch)
{
    for (uint32_t i = 0; i < sourceWidth; i++) {
        uint32_t R = sourceRow[0];
        uint32_t G = sourceRow[2];
        uint32_t B = sourceRow[4];
        uint32_t A = ALPHA_OPAQUE;
        if (branch == BRANCH_RGB161616_TO_ARGB8888) {
            destinationRow[i] = FillARGB8888(A, R, G, B);
        } else if (branch == BRANCH_RGB161616_TO_ABGR8888) {
            destinationRow[i] = FillABGR8888(A, B, G, R);
        } else if (branch == BRANCH_RGB161616_TO_RGBA8888) {
            destinationRow[i] = FillRGBA8888(R, G, B, A);
        } else if (branch == BRANCH_RGB161616_TO_BGRA8888) {
            destinationRow[i] = FillBGRA8888(B, G, R, A);
        } else if (branch == BRANCH_RGB161616_TO_RGB565) {
            R = R >> SHIFT_3_BIT;
            G = G >> SHIFT_2_BIT;
            B = B >> SHIFT_3_BIT;
            destinationRow[i] = FillRGB565(R, G, B);
        } else {
            break;
        }
        sourceRow += SIZE_6_BYTE;
    }
}

static void RGB161616ConvertARGB8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                     const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    RGB161616Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGB161616_TO_ARGB8888);
}

static void RGB161616ConvertABGR8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                     const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    RGB161616Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGB161616_TO_ABGR8888);
}

static void RGB161616ConvertRGBA8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                     const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    RGB161616Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGB161616_TO_RGBA8888);
}

static void RGB161616ConvertBGRA8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                     const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    RGB161616Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGB161616_TO_BGRA8888);
}

static void RGB161616ConvertRGB565(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                   const ProcFuncExtension &extension)
{
    uint16_t *newDestinationRow = static_cast<uint16_t *>(destinationRow);
    RGB161616Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGB161616_TO_RGB565);
}

constexpr uint32_t BRANCH_RGBA16161616_TO_ARGB8888 = 0x60000001;
constexpr uint32_t BRANCH_RGBA16161616_TO_ABGR8888 = 0x60000002;
constexpr uint32_t BRANCH_RGBA16161616_TO_RGBA8888 = 0x60000003;
constexpr uint32_t BRANCH_RGBA16161616_TO_BGRA8888 = 0x60000004;
template<typename T>
static void RGBA16161616Convert(T *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth, uint32_t branch,
                                const ProcFuncExtension &extension)
{
    for (uint32_t i = 0; i < sourceWidth; i++) {
        uint32_t R = sourceRow[0];
        uint32_t G = sourceRow[2];
        uint32_t B = sourceRow[4];
        uint32_t A = sourceRow[6];
        AlphaTypeConvertOnRGB(A, R, G, B, extension);
        if (branch == BRANCH_RGBA16161616_TO_ARGB8888) {
            destinationRow[i] = FillARGB8888(A, R, G, B);
        } else if (branch == BRANCH_RGBA16161616_TO_ABGR8888) {
            destinationRow[i] = FillABGR8888(A, B, G, R);
        } else if (branch == BRANCH_RGBA16161616_TO_RGBA8888) {
            destinationRow[i] = FillRGBA8888(A, B, G, R);
        } else if (branch == BRANCH_RGBA16161616_TO_BGRA8888) {
            destinationRow[i] = FillBGRA8888(A, B, G, R);
        } else {
            break;
        }
        sourceRow += SIZE_8_BYTE;
    }
}

static void RGBA16161616ConvertARGB8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                        const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    RGBA16161616Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGBA16161616_TO_ARGB8888, extension);
}

static void RGBA16161616ConvertABGR8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                        const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    RGBA16161616Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGBA16161616_TO_ABGR8888, extension);
}

static void RGBA16161616ConvertRGBA8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                        const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    RGBA16161616Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGBA16161616_TO_RGBA8888, extension);
}

static void RGBA16161616ConvertBGRA8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                        const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    RGBA16161616Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGBA16161616_TO_BGRA8888, extension);
}

constexpr uint32_t BRANCH_CMYK_TO_ARGB8888 = 0x70000001;
constexpr uint32_t BRANCH_CMYK_TO_ABGR8888 = 0x70000002;
constexpr uint32_t BRANCH_CMYK_TO_RGBA8888 = 0x70000003;
constexpr uint32_t BRANCH_CMYK_TO_BGRA8888 = 0x70000004;
constexpr uint32_t BRANCH_CMYK_TO_RGB565 = 0x70000005;
template<typename T>
static void CMYKConvert(T *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth, uint32_t branch)
{
    for (uint32_t i = 0; i < sourceWidth; i++) {
        uint8_t C = sourceRow[0];
        uint8_t M = sourceRow[1];
        uint8_t Y = sourceRow[2];
        uint8_t K = sourceRow[3];
        uint32_t R = Premul255(C, K);
        uint32_t G = Premul255(M, K);
        uint32_t B = Premul255(Y, K);
        uint32_t A = ALPHA_OPAQUE;
        if (branch == BRANCH_CMYK_TO_ARGB8888) {
            destinationRow[i] = FillARGB8888(A, R, G, B);
        } else if (branch == BRANCH_CMYK_TO_ABGR8888) {
            destinationRow[i] = FillABGR8888(A, B, G, R);
        } else if (branch == BRANCH_CMYK_TO_RGBA8888) {
            destinationRow[i] = FillRGBA8888(R, G, B, A);
        } else if (branch == BRANCH_CMYK_TO_BGRA8888) {
            destinationRow[i] = FillBGRA8888(B, G, R, A);
        } else if (branch == BRANCH_CMYK_TO_RGB565) {
            R = R >> SHIFT_3_BIT;
            G = R >> SHIFT_2_BIT;
            B = B >> SHIFT_3_BIT;
            destinationRow[i] = FillRGB565(R, G, B);
        } else {
            break;
        }
        sourceRow += SIZE_4_BYTE;
    }
}

static void CMYKConvertARGB8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    CMYKConvert(newDestinationRow, sourceRow, sourceWidth, BRANCH_CMYK_TO_ARGB8888);
}

static void CMYKConvertABGR8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    CMYKConvert(newDestinationRow, sourceRow, sourceWidth, BRANCH_CMYK_TO_ABGR8888);
}

static void CMYKConvertRGBA8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    CMYKConvert(newDestinationRow, sourceRow, sourceWidth, BRANCH_CMYK_TO_RGBA8888);
}

static void CMYKConvertBGRA8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    CMYKConvert(newDestinationRow, sourceRow, sourceWidth, BRANCH_CMYK_TO_BGRA8888);
}

static void CMYKConvertRGB565(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                              const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    CMYKConvert(newDestinationRow, sourceRow, sourceWidth, BRANCH_CMYK_TO_RGB565);
}

constexpr uint32_t BRANCH_RGB565_TO_ARGB8888 = 0x11000001;
constexpr uint32_t BRANCH_RGB565_TO_RGBA8888 = 0x11000002;
constexpr uint32_t BRANCH_RGB565_TO_BGRA8888 = 0x11000003;
template<typename T>
static void RGB565Convert(T *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth, uint32_t branch)
{
    for (uint32_t i = 0; i < sourceWidth; i++) {
        uint32_t R = (sourceRow[0] >> SHIFT_3_BIT) & SHIFT_5_MASK;
        uint32_t G = ((sourceRow[0] & SHIFT_3_MASK) << SHIFT_3_BIT) | ((sourceRow[1] >> SHIFT_5_BIT) & SHIFT_3_MASK);
        uint32_t B = sourceRow[1] & SHIFT_5_MASK;
        uint32_t A = ALPHA_OPAQUE;
        if (branch == BRANCH_RGB565_TO_ARGB8888) {
            destinationRow[i] = FillARGB8888(A, R, G, B);
        } else if (branch == BRANCH_RGB565_TO_RGBA8888) {
            destinationRow[i] = FillRGBA8888(R, G, B, A);
        } else if (branch == BRANCH_RGB565_TO_BGRA8888) {
            destinationRow[i] = FillBGRA8888(B, G, R, A);
        } else {
            break;
        }
        sourceRow += SIZE_2_BYTE;
    }
}

static void RGB565ConvertARGB8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                  const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    RGB565Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGB565_TO_ARGB8888);
}

static void RGB565ConvertRGBA8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                  const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    RGB565Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGB565_TO_RGBA8888);
}

static void RGB565ConvertBGRA8888(void *destinationRow, const uint8_t *sourceRow, uint32_t sourceWidth,
                                  const ProcFuncExtension &extension)
{
    uint32_t *newDestinationRow = static_cast<uint32_t *>(destinationRow);
    RGB565Convert(newDestinationRow, sourceRow, sourceWidth, BRANCH_RGB565_TO_BGRA8888);
}

static map<string, ProcFuncType> g_procMapping;
static mutex g_procMutex;

static string MakeKey(uint32_t srcFormat, uint32_t dstFormat)
{
    return to_string(srcFormat) + ("_") + to_string(dstFormat);
}

static void InitGrayProc()
{
    g_procMapping.emplace(MakeKey(GRAY_BIT, ARGB_8888), &BitConvertARGB8888);
    g_procMapping.emplace(MakeKey(GRAY_BIT, RGB_565), &BitConvertRGB565);
    g_procMapping.emplace(MakeKey(GRAY_BIT, ALPHA_8), &BitConvertGray);

    g_procMapping.emplace(MakeKey(ALPHA_8, ARGB_8888), &GrayConvertARGB8888);
    g_procMapping.emplace(MakeKey(ALPHA_8, RGB_565), &GrayConvertRGB565);

    g_procMapping.emplace(MakeKey(GRAY_ALPHA, ARGB_8888), &GrayAlphaConvertARGB8888);
    g_procMapping.emplace(MakeKey(GRAY_ALPHA, ALPHA_8), &GrayAlphaConvertAlpha);
}

static void InitRGBProc()
{
    g_procMapping.emplace(MakeKey(RGB_888, ARGB_8888), &RGB888ConvertARGB8888);
    g_procMapping.emplace(MakeKey(RGB_888, RGBA_8888), &RGB888ConvertRGBA8888);
    g_procMapping.emplace(MakeKey(RGB_888, BGRA_8888), &RGB888ConvertBGRA8888);
    g_procMapping.emplace(MakeKey(RGB_888, RGB_565), &RGB888ConvertRGB565);

    g_procMapping.emplace(MakeKey(BGR_888, ARGB_8888), &BGR888ConvertARGB8888);
    g_procMapping.emplace(MakeKey(BGR_888, RGBA_8888), &BGR888ConvertRGBA8888);
    g_procMapping.emplace(MakeKey(BGR_888, BGRA_8888), &BGR888ConvertBGRA8888);
    g_procMapping.emplace(MakeKey(BGR_888, RGB_565), &BGR888ConvertRGB565);

    g_procMapping.emplace(MakeKey(RGB_161616, ARGB_8888), &RGB161616ConvertARGB8888);
    g_procMapping.emplace(MakeKey(RGB_161616, ABGR_8888), &RGB161616ConvertABGR8888);
    g_procMapping.emplace(MakeKey(RGB_161616, RGBA_8888), &RGB161616ConvertRGBA8888);
    g_procMapping.emplace(MakeKey(RGB_161616, BGRA_8888), &RGB161616ConvertBGRA8888);
    g_procMapping.emplace(MakeKey(RGB_161616, RGB_565), &RGB161616ConvertRGB565);

    g_procMapping.emplace(MakeKey(RGB_565, ARGB_8888), &RGB565ConvertARGB8888);
    g_procMapping.emplace(MakeKey(RGB_565, RGBA_8888), &RGB565ConvertRGBA8888);
    g_procMapping.emplace(MakeKey(RGB_565, BGRA_8888), &RGB565ConvertBGRA8888);
}

static void InitRGBAProc()
{
    g_procMapping.emplace(MakeKey(RGBA_8888, RGBA_8888), &RGBA8888ConvertRGBA8888Alpha);
    g_procMapping.emplace(MakeKey(RGBA_8888, ARGB_8888), &RGBA8888ConvertARGB8888);
    g_procMapping.emplace(MakeKey(RGBA_8888, BGRA_8888), &RGBA8888ConvertBGRA8888);
    g_procMapping.emplace(MakeKey(RGBA_8888, RGB_565), &RGBA8888ConvertRGB565);

    g_procMapping.emplace(MakeKey(BGRA_8888, RGBA_8888), &BGRA8888ConvertRGBA8888);
    g_procMapping.emplace(MakeKey(BGRA_8888, ARGB_8888), &BGRA8888ConvertARGB8888);
    g_procMapping.emplace(MakeKey(BGRA_8888, BGRA_8888), &BGRA8888ConvertBGRA8888Alpha);
    g_procMapping.emplace(MakeKey(BGRA_8888, RGB_565), &BGRA8888ConvertRGB565);

    g_procMapping.emplace(MakeKey(ARGB_8888, RGBA_8888), &ARGB8888ConvertRGBA8888);
    g_procMapping.emplace(MakeKey(ARGB_8888, ARGB_8888), &ARGB8888ConvertARGB8888Alpha);
    g_procMapping.emplace(MakeKey(ARGB_8888, BGRA_8888), &ARGB8888ConvertBGRA8888);
    g_procMapping.emplace(MakeKey(ARGB_8888, RGB_565), &ARGB8888ConvertRGB565);

    g_procMapping.emplace(MakeKey(RGBA_16161616, ARGB_8888), &RGBA16161616ConvertARGB8888);
    g_procMapping.emplace(MakeKey(RGBA_16161616, RGBA_8888), &RGBA16161616ConvertRGBA8888);
    g_procMapping.emplace(MakeKey(RGBA_16161616, BGRA_8888), &RGBA16161616ConvertBGRA8888);
    g_procMapping.emplace(MakeKey(RGBA_16161616, ABGR_8888), &RGBA16161616ConvertABGR8888);
}

static void InitCMYKProc()
{
    g_procMapping.emplace(MakeKey(CMKY, ARGB_8888), &CMYKConvertARGB8888);
    g_procMapping.emplace(MakeKey(CMKY, RGBA_8888), &CMYKConvertRGBA8888);
    g_procMapping.emplace(MakeKey(CMKY, BGRA_8888), &CMYKConvertBGRA8888);
    g_procMapping.emplace(MakeKey(CMKY, ABGR_8888), &CMYKConvertABGR8888);
    g_procMapping.emplace(MakeKey(CMKY, RGB_565), &CMYKConvertRGB565);
}

static ProcFuncType GetProcFuncType(uint32_t srcPixelFormat, uint32_t dstPixelFormat)
{
    unique_lock<mutex> guard(g_procMutex);
    if (g_procMapping.empty()) {
        InitGrayProc();
        InitRGBProc();
        InitRGBAProc();
        InitCMYKProc();
    }
    guard.unlock();
    string procKey = MakeKey(srcPixelFormat, dstPixelFormat);
    map<string, ProcFuncType>::iterator iter = g_procMapping.find(procKey);
    if (iter != g_procMapping.end()) {
        return iter->second;
    }
    return nullptr;
}

PixelConvert::PixelConvert(ProcFuncType funcPtr, ProcFuncExtension extension, bool isNeedConvert)
    : procFunc_(funcPtr), procFuncExtension_(extension), isNeedConvert_(isNeedConvert)
{}

// caller need setting the correct pixelFormat and alphaType
std::unique_ptr<PixelConvert> PixelConvert::Create(const ImageInfo &srcInfo, const ImageInfo &dstInfo)
{
    if (srcInfo.pixelFormat == PixelFormat::UNKNOWN || dstInfo.pixelFormat == PixelFormat::UNKNOWN) {
        HiLog::Error(LABEL, "source or destination pixel format unknown");
        return nullptr;
    }
    uint32_t srcFormat = static_cast<uint32_t>(srcInfo.pixelFormat);
    uint32_t dstFormat = static_cast<uint32_t>(dstInfo.pixelFormat);
    ProcFuncType funcPtr = GetProcFuncType(srcFormat, dstFormat);
    if (funcPtr == nullptr) {
        HiLog::Error(LABEL, "not found convert function. pixelFormat %{public}u -> %{public}u", srcFormat, dstFormat);
        return nullptr;
    }
    ProcFuncExtension extension;
    extension.alphaConvertType = GetAlphaConvertType(srcInfo.alphaType, dstInfo.alphaType);
    bool isNeedConvert = true;
    if ((srcInfo.pixelFormat == dstInfo.pixelFormat) && (extension.alphaConvertType == AlphaConvertType::NO_CONVERT)) {
        isNeedConvert = false;
    }
    return unique_ptr<PixelConvert>(new (nothrow) PixelConvert(funcPtr, extension, isNeedConvert));
}

AlphaConvertType PixelConvert::GetAlphaConvertType(const AlphaType &srcType, const AlphaType &dstType)
{
    if (srcType == AlphaType::IMAGE_ALPHA_TYPE_UNKNOWN || dstType == AlphaType::IMAGE_ALPHA_TYPE_UNKNOWN) {
        HiLog::Debug(LABEL, "source or destination alpha type unknown");
        return AlphaConvertType::NO_CONVERT;
    }
    if ((srcType == AlphaType::IMAGE_ALPHA_TYPE_PREMUL) && (dstType == AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL)) {
        return AlphaConvertType::PREMUL_CONVERT_UNPREMUL;
    }
    if ((srcType == AlphaType::IMAGE_ALPHA_TYPE_PREMUL) && (dstType == AlphaType::IMAGE_ALPHA_TYPE_OPAQUE)) {
        return AlphaConvertType::PREMUL_CONVERT_OPAQUE;
    }
    if ((srcType == AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL) && (dstType == AlphaType::IMAGE_ALPHA_TYPE_PREMUL)) {
        return AlphaConvertType::UNPREMUL_CONVERT_PREMUL;
    }
    if ((srcType == AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL) && (dstType == AlphaType::IMAGE_ALPHA_TYPE_OPAQUE)) {
        return AlphaConvertType::UNPREMUL_CONVERT_OPAQUE;
    }
    return AlphaConvertType::NO_CONVERT;
}

void PixelConvert::Convert(void *destinationPixels, const uint8_t *sourcePixels, uint32_t sourcePixelsNum)
{
    if ((destinationPixels == nullptr) || (sourcePixels == nullptr)) {
        HiLog::Error(LABEL, "destinationPixel or sourcePixel is null");
        return;
    }
    if (!isNeedConvert_) {
        HiLog::Debug(LABEL, "no need convert");
        return;
    }
    procFunc_(destinationPixels, sourcePixels, sourcePixelsNum, procFuncExtension_);
}
} // namespace Media
} // namespace OHOS
