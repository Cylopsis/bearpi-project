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

#ifndef PIXEL_MAP_UTILS_H
#define PIXEL_MAP_UTILS_H

#include "image_type.h"
#include "log_tags.h"
#include "pixel_map.h"

namespace OHOS {
namespace Media {
// Define bytes per pixel
constexpr int8_t ALPHA_8_BYTES = 1;
constexpr int8_t RGB_565_BYTES = 2;
constexpr int8_t RGB_888_BYTES = 3;
constexpr int8_t ARGB_8888_BYTES = 4;
constexpr int8_t YUV420_BYTES = 2;  // in fact NV21 one pixel used 1.5 bytes.

// Define shift bits of bytes per pixel
constexpr int8_t ALPHA_8_SHIFT = 0;
constexpr int8_t RGB_565_SHIFT = 1;
constexpr int8_t ARGB_8888_SHIFT = 2;

// Convert RGB565 16bit pixel to 32bit pixel
constexpr uint8_t RGB565_R_BITS = 5;
constexpr uint8_t RGB565_G_BITS = 6;
constexpr uint8_t RGB565_B_BITS = 5;

#if __BYTE_ORDER == __LITTLE_ENDIAN
constexpr uint8_t RGB565_R_SHIFT = 0;
constexpr uint8_t RGB565_G_SHIFT = RGB565_R_BITS;
constexpr uint8_t RGB565_B_SHIFT = RGB565_R_BITS + RGB565_G_BITS;
constexpr uint16_t RGB565_R_MASK = 0x001F;
constexpr uint16_t RGB565_G_MASK = 0x07E0;
constexpr uint16_t RGB565_B_MASK = 0xF800;
#else
constexpr uint8_t RGB565_R_SHIFT = RGB565_B_BITS + RGB565_G_BITS;
constexpr uint8_t RGB565_G_SHIFT = RGB565_B_BITS;
constexpr uint8_t RGB565_B_SHIFT = 0;
constexpr uint16_t RGB565_R_MASK = 0xF800;
constexpr uint16_t RGB565_G_MASK = 0x07E0;
constexpr uint16_t RGB565_B_MASK = 0x001F;
#endif
constexpr uint8_t BYTE_BITS = 8;
constexpr uint8_t RGB565_CONVERT_BIT = 2;
constexpr uint8_t ARGB8888_CONVERT_BIT = 24;

// Convert for ARGB_8888 32bit pixel
#if __BYTE_ORDER == __LITTLE_ENDIAN
constexpr uint8_t ARGB32_A_SHIFT = 0;
constexpr uint8_t ARGB32_R_SHIFT = 8;
constexpr uint8_t ARGB32_G_SHIFT = 16;
constexpr uint8_t ARGB32_B_SHIFT = 24;
#else
constexpr uint8_t ARGB32_A_SHIFT = 24;
constexpr uint8_t ARGB32_R_SHIFT = 16;
constexpr uint8_t ARGB32_G_SHIFT = 8;
constexpr uint8_t ARGB32_B_SHIFT = 0;
#endif

// Convert for RGBA_8888 32bit pixel
#if __BYTE_ORDER == __LITTLE_ENDIAN
constexpr uint8_t RGBA32_R_SHIFT = 0;
constexpr uint8_t RGBA32_G_SHIFT = 8;
constexpr uint8_t RGBA32_B_SHIFT = 16;
constexpr uint8_t RGBA32_A_SHIFT = 24;
#else
constexpr uint8_t RGBA32_R_SHIFT = 24;
constexpr uint8_t RGBA32_G_SHIFT = 16;
constexpr uint8_t RGBA32_B_SHIFT = 8;
constexpr uint8_t RGBA32_A_SHIFT = 0;
#endif

// Convert for BGRA_8888 32bit pixel
#if __BYTE_ORDER == __LITTLE_ENDIAN
constexpr uint8_t BGRA32_B_SHIFT = 0;
constexpr uint8_t BGRA32_G_SHIFT = 8;
constexpr uint8_t BGRA32_R_SHIFT = 16;
constexpr uint8_t BGRA32_A_SHIFT = 24;
#else
constexpr uint8_t BGRA32_B_SHIFT = 24;
constexpr uint8_t BGRA32_G_SHIFT = 16;
constexpr uint8_t BGRA32_R_SHIFT = 8;
constexpr uint8_t BGRA32_A_SHIFT = 0;
#endif

constexpr uint8_t BYTE_FULL = 0xFF;
constexpr uint8_t BYTE_ZERO = 0;
constexpr uint8_t ONE_PIXEL_SIZE = 1;

/*
 * For RGB_565
 * 1. get R(5-bits)/G(6-bits)/B(5-bits) channel value form color value(uint16_t)
 * 2. convert R(5-bits)/G(6-bits)/B(5-bits) value to R(8-bits)/G(8-bits)/B(8-bits)
 * 3. construct normalized color value with A(255)/R(8-bits)/G(8-bits)/B(8-bits)
 * 4. the normalized color format: (A << 24 | R << 16 | G << 8 | B << 0)
 */
static uint8_t GetRGB565Channel(uint16_t color, uint16_t mask, uint8_t shift)
{
    return (color & mask) >> shift;
}

static uint8_t RGB565To32(uint8_t channel, uint8_t bits)
{
    return (channel << (BYTE_BITS - bits)) | (channel >> (RGB565_CONVERT_BIT * bits - BYTE_BITS));
}

static uint8_t RGB565ToR32(uint16_t color)
{
    return RGB565To32(GetRGB565Channel(color, RGB565_R_MASK, RGB565_R_SHIFT), RGB565_R_BITS);
}

static uint8_t RGB565ToG32(uint16_t color)
{
    return RGB565To32(GetRGB565Channel(color, RGB565_G_MASK, RGB565_G_SHIFT), RGB565_G_BITS);
}

static uint8_t RGB565ToB32(uint16_t color)
{
    return RGB565To32(GetRGB565Channel(color, RGB565_B_MASK, RGB565_B_SHIFT), RGB565_B_BITS);
}

/*
 * For ARGB_8888
 * 1. get A(8-bits)/R(8-bits)/G(8-bits)/B(8-bits) channel value form color value(uint32_t)
 * 2. construct normalized color value with A(8-bits)/R(8-bits)/G(8-bits)/B(8-bits)
 * 3. the normalized color format: (A << 24 | R << 16 | G << 8 | B << 0)
 */
static uint8_t GetColorComp(uint32_t color, uint8_t shift)
{
    return ((color) << (ARGB8888_CONVERT_BIT - shift)) >> ARGB8888_CONVERT_BIT;
}

static uint32_t GetColorARGB(uint8_t a, uint8_t r, uint8_t g, uint8_t b)
{
    return ((a << ARGB_A_SHIFT) | (r << ARGB_R_SHIFT) | (g << ARGB_G_SHIFT) | (b << ARGB_B_SHIFT));
}

static ImageInfo MakeImageInfo(int width, int height, PixelFormat pf, AlphaType at, ColorSpace cs = ColorSpace::SRGB)
{
    ImageInfo info;
    info.size.width = width;
    info.size.height = height;
    info.pixelFormat = pf;
    info.alphaType = at;
    info.colorSpace = cs;
    return info;
}
} // namespace Media
} // namespace OHOS

#endif // PIXEL_MAP_UTILS_H