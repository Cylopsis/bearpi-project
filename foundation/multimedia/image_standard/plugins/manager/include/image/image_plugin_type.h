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

#ifndef IMAGE_PLUGIN_TYPE_H
#define IMAGE_PLUGIN_TYPE_H

#include <cstdint>

namespace OHOS {
namespace ImagePlugin {
enum class PlColorSpace {
    // unknown color space.
    UNKNOWN = 0,

    // based on SMPTE RP 431-2-2007 & IEC 61966-2.1:1999.
    DISPLAY_P3 = 1,

    // standard Red Green Blue based on IEC 61966-2.1:1999.
    SRGB = 2,

    // SRGB with a linear transfer function based on IEC 61966-2.1:1999.
    LINEAR_SRGB = 3,

    // based on IEC 61966-2-2:2003.
    EXTENDED_SRGB = 4,

    // based on IEC 61966-2-2:2003.
    LINEAR_EXTENDED_SRGB = 5,

    // based on standard illuminant D50 as the white point.
    GENERIC_XYZ = 6,

    // based on CIE XYZ D50 as the profile conversion space.
    GENERIC_LAB = 7,

    // based on SMPTE ST 2065-1:2012.
    ACES = 8,

    // based on Academy S-2014-004.
    ACES_CG = 9,

    // based on Adobe RGB (1998).
    ADOBE_RGB_1998 = 10,

    // based on SMPTE RP 431-2-2007.
    DCI_P3 = 11,

    // based on Rec. ITU-R BT.709-5.
    ITU_709 = 12,

    // based on Rec. ITU-R BT.2020-1.
    ITU_2020 = 13,

    // based on ROMM RGB ISO 22028-2:2013.
    ROMM_RGB = 14,

    // based on 1953 standard.
    NTSC_1953 = 15,

    // based on SMPTE C.
    SMPTE_C = 16
};

enum class PlEncodedFormat {
    UNKNOWN = 0,
    JPEG = 1,
    PNG = 2,
    GIF = 3,
    HEIF = 4
};

enum class PlPixelFormat {
    UNKNOWN = 0,
    ARGB_8888 = 1,
    RGB_565 = 2,
    RGBA_8888 = 3,
    BGRA_8888 = 4,
    RGB_888 = 5,
    ALPHA_8 = 6,
    RGBA_F16 = 7,
    NV21 = 8,
    NV12 = 9,
    CMYK = 10,
};

enum class PlAlphaType : int32_t {
    IMAGE_ALPHA_TYPE_UNKNOWN = 0,
    IMAGE_ALPHA_TYPE_OPAQUE = 1,
    IMAGE_ALPHA_TYPE_PREMUL = 2,
    IMAGE_ALPHA_TYPE_UNPREMUL = 3,
};

struct PlPosition {
    uint32_t x = 0;
    uint32_t y = 0;
};

struct PlRect {
    uint32_t left = 0;
    uint32_t top = 0;
    uint32_t width = 0;
    uint32_t height = 0;
};

struct PlSize {
    uint32_t width = 0;
    uint32_t height = 0;
};

struct PlImageInfo {
    PlSize size;
    PlPixelFormat pixelFormat = PlPixelFormat::UNKNOWN;
    PlColorSpace colorSpace = PlColorSpace::UNKNOWN;
    PlAlphaType alphaType = PlAlphaType::IMAGE_ALPHA_TYPE_UNKNOWN;
};

struct PlImageBuffer {
    void *buffer = nullptr;
    uint32_t bufferSize = 0;
    uint32_t dataSize = 0;
    void *context = nullptr;
};
} // namespace ImagePlugin
} // namespace OHOS

#endif // IMAGE_PLUGIN_TYPE_H