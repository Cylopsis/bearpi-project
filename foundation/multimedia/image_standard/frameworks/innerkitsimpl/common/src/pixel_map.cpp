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

#include "pixel_map.h"
#include <iostream>
#include <unistd.h>
#include "hilog/log.h"
#include "image_utils.h"
#include "log_tags.h"
#include "media_errors.h"
#include "pixel_convert_adapter.h"
#include "pixel_map_utils.h"
#include "pixel_map_parcel.h"
#include "post_proc.h"
#include "parcel.h"
#include "ipc_file_descriptor.h"
#ifndef _WIN32
#include "securec.h"
#else
#include "memory.h"
#endif

#if !defined(_WIN32) && !defined(_APPLE)
#include <sys/mman.h>
#include "ashmem.h"
#endif

namespace OHOS {
namespace Media {
using namespace OHOS::HiviewDFX;
using namespace std;
constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_IMAGE, "PixelMap" };
constexpr int32_t MAX_DIMENSION = INT32_MAX >> 2;
constexpr uint8_t FOUR_BYTE_SHIFT = 2;
constexpr uint8_t BGRA_ALPHA_INDEX = 3;
constexpr uint8_t BGRA_BYTES = 4;
constexpr uint8_t PER_PIXEL_LEN = 1;

PixelMap::~PixelMap()
{
    FreePixelMap();
}

void PixelMap::FreePixelMap()
{
    if (data_ == nullptr) {
        return;
    }
    switch (allocatorType_) {
        case AllocatorType::HEAP_ALLOC: {
            free(data_);
            data_ = nullptr;
            break;
        }
        case AllocatorType::CUSTOM_ALLOC: {
            if (custFreePixelMap_ != nullptr) {
                custFreePixelMap_(data_, context_, pixelsSize_);
            }
            data_ = nullptr;
            context_ = nullptr;
            break;
        }
        case AllocatorType::SHARE_MEM_ALLOC: {
            ReleaseSharedMemory(data_, context_, pixelsSize_);
            data_ = nullptr;
            context_ = nullptr;
            break;
        }
        default: {
            HiLog::Error(LABEL, "unknown allocator type:[%{public}d].", allocatorType_);
            return;
        }
    }
}

void PixelMap::ReleaseSharedMemory(void *addr, void *context, uint32_t size)
{
#if !defined(_WIN32) && !defined(_APPLE)
    int *fd = static_cast<int *>(context);
    if (addr != nullptr) {
        ::munmap(addr, size);
    }
    if (fd != nullptr) {
        ::close(*fd);
        delete fd;
    }
#endif
}

void PixelMap::SetPixelsAddr(void *addr, void *context, uint32_t size, AllocatorType type, CustomFreePixelMap func)
{
    FreePixelMap();
    data_ = static_cast<uint8_t *>(addr);
    context_ = context;
    pixelsSize_ = size;
    allocatorType_ = type;
    custFreePixelMap_ = func;
}

unique_ptr<PixelMap> PixelMap::Create(const uint32_t *colors, uint32_t colorLength, const InitializationOptions &opts)
{
    return Create(colors, colorLength, 0, opts.size.width, opts);
}

unique_ptr<PixelMap> PixelMap::Create(const uint32_t *colors, uint32_t colorLength, int32_t offset, int32_t stride,
                                      const InitializationOptions &opts)
{
    if (!CheckParams(colors, colorLength, offset, stride, opts)) {
        return nullptr;
    }
    unique_ptr<PixelMap> dstPixelMap = make_unique<PixelMap>();
    if (dstPixelMap == nullptr) {
        HiLog::Error(LABEL, "create pixelMap pointer fail");
        return nullptr;
    }

    ImageInfo srcImageInfo =
        MakeImageInfo(stride, opts.size.height, PixelFormat::BGRA_8888, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);
    PixelFormat dstPixelFormat = (opts.pixelFormat == PixelFormat::UNKNOWN ? PixelFormat::RGBA_8888 : opts.pixelFormat);
    AlphaType dstAlphaType =
        (opts.alphaType == AlphaType::IMAGE_ALPHA_TYPE_UNKNOWN) ? AlphaType::IMAGE_ALPHA_TYPE_PREMUL : opts.alphaType;
    dstAlphaType = ImageUtils::GetValidAlphaTypeByFormat(dstAlphaType, dstPixelFormat);
    ImageInfo dstImageInfo = MakeImageInfo(opts.size.width, opts.size.height, dstPixelFormat, dstAlphaType);
    if (dstPixelMap->SetImageInfo(dstImageInfo) != SUCCESS) {
        HiLog::Error(LABEL, "set image info fail");
        return nullptr;
    }
    uint32_t bufferSize = dstPixelMap->GetByteCount();
    if (bufferSize == 0) {
        HiLog::Error(LABEL, "malloc parameter is zero");
        return nullptr;
    }
    void *dstPixels = malloc(bufferSize);
    if (dstPixels == nullptr) {
        HiLog::Error(LABEL, "allocate memory size %{public}u fail", bufferSize);
        return nullptr;
    }

    Position dstPosition;
    if (!PixelConvertAdapter::WritePixelsConvert(reinterpret_cast<const void *>(colors + offset),
        static_cast<uint32_t>(stride) << FOUR_BYTE_SHIFT, srcImageInfo,
        dstPixels, dstPosition, dstPixelMap->GetRowBytes(), dstImageInfo)) {
        HiLog::Error(LABEL, "pixel convert in adapter failed.");
        free(dstPixels);
        dstPixels = nullptr;
        return nullptr;
    }
    dstPixelMap->SetEditable(opts.editable);
    dstPixelMap->SetPixelsAddr(dstPixels, nullptr, bufferSize, AllocatorType::HEAP_ALLOC, nullptr);
    return dstPixelMap;
}

bool PixelMap::CheckParams(const uint32_t *colors, uint32_t colorLength, int32_t offset, int32_t stride,
                           const InitializationOptions &opts)
{
    if (colors == nullptr || colorLength <= 0 || colorLength > PIXEL_MAP_MAX_RAM_SIZE) {
        HiLog::Error(LABEL, "colors invalid");
        return false;
    }
    int32_t dstWidth = opts.size.width;
    int32_t dstHeight = opts.size.height;
    if (dstWidth <= 0 || dstHeight <= 0) {
        HiLog::Error(LABEL, "initial options size invalid");
        return false;
    }
    if (stride < dstWidth) {
        HiLog::Error(LABEL, "stride: %{public}d must >= width: %{public}d", stride, dstWidth);
        return false;
    }
    if (stride > MAX_DIMENSION) {
        HiLog::Error(LABEL, "stride %{public}d is out of range", stride);
        return false;
    }
    int64_t lastLine = static_cast<int64_t>(dstHeight - 1) * stride + offset;
    if (offset < 0 || static_cast<int64_t>(offset) + dstWidth > colorLength || lastLine + dstWidth > colorLength) {
        HiLog::Error(LABEL, "colors length: %{public}u, offset: %{public}d, stride: %{public}d  is invalid",
                     colorLength, offset, stride);
        return false;
    }
    return true;
}

unique_ptr<PixelMap> PixelMap::Create(const InitializationOptions &opts)
{
    unique_ptr<PixelMap> dstPixelMap = make_unique<PixelMap>();
    if (dstPixelMap == nullptr) {
        HiLog::Error(LABEL, "create pixelMap pointer fail");
        return nullptr;
    }
    PixelFormat dstPixelFormat = (opts.pixelFormat == PixelFormat::UNKNOWN ? PixelFormat::RGBA_8888 : opts.pixelFormat);
    AlphaType dstAlphaType =
        (opts.alphaType == AlphaType::IMAGE_ALPHA_TYPE_UNKNOWN) ? AlphaType::IMAGE_ALPHA_TYPE_PREMUL : opts.alphaType;
    dstAlphaType = ImageUtils::GetValidAlphaTypeByFormat(dstAlphaType, dstPixelFormat);
    ImageInfo dstImageInfo = MakeImageInfo(opts.size.width, opts.size.height, dstPixelFormat, dstAlphaType);
    if (dstPixelMap->SetImageInfo(dstImageInfo) != SUCCESS) {
        HiLog::Error(LABEL, "set image info fail");
        return nullptr;
    }
    uint32_t bufferSize = dstPixelMap->GetByteCount();
    uint8_t *dstPixels = static_cast<uint8_t *>(calloc(bufferSize, 1));
    if (dstPixels == nullptr) {
        HiLog::Error(LABEL, "allocate memory size %{public}u fail", bufferSize);
        return nullptr;
    }
    // update alpha opaque
    UpdatePixelsAlpha(dstImageInfo.alphaType, dstImageInfo.pixelFormat, dstPixels, *dstPixelMap.get());
    dstPixelMap->SetPixelsAddr(dstPixels, nullptr, bufferSize, AllocatorType::HEAP_ALLOC, nullptr);
    dstPixelMap->SetEditable(opts.editable);
    return dstPixelMap;
}

void PixelMap::UpdatePixelsAlpha(const AlphaType &alphaType, const PixelFormat &pixelFormat, uint8_t *dstPixels,
                                 PixelMap dstPixelMap)
{
    if (alphaType == AlphaType::IMAGE_ALPHA_TYPE_OPAQUE) {
        int8_t alphaIndex = -1;
        if (pixelFormat == PixelFormat::RGBA_8888 || pixelFormat == PixelFormat::BGRA_8888) {
            alphaIndex = BGRA_ALPHA_INDEX;
        } else if (pixelFormat == PixelFormat::ARGB_8888) {
            alphaIndex = 0;
        }
        if (alphaIndex != -1) {
            uint8_t pixelBytes = dstPixelMap.GetPixelBytes();
            uint32_t bufferSize = dstPixelMap.GetByteCount();
            uint32_t i = alphaIndex;
            while (i < bufferSize) {
                dstPixels[i] = ALPHA_OPAQUE;
                i += pixelBytes;
            }
        }
    }
}

unique_ptr<PixelMap> PixelMap::Create(PixelMap &source, const InitializationOptions &opts)
{
    Rect rect;
    return Create(source, rect, opts);
}

unique_ptr<PixelMap> PixelMap::Create(PixelMap &source, const Rect &srcRect, const InitializationOptions &opts)
{
    ImageInfo srcImageInfo;
    source.GetImageInfo(srcImageInfo);
    PostProc postProc;
    CropValue cropType = PostProc::GetCropValue(srcRect, srcImageInfo.size);
    if (cropType == CropValue::INVALID) {
        HiLog::Error(LABEL, "src crop range is invalid");
        return nullptr;
    }
    ImageInfo dstImageInfo;
    InitDstImageInfo(opts, srcImageInfo, dstImageInfo);
    Size targetSize = dstImageInfo.size;
    // use source if match
    bool isHasConvert = postProc.HasPixelConvert(srcImageInfo, dstImageInfo);
    if (opts.useSourceIfMatch && !source.IsEditable() && !opts.editable && (cropType == CropValue::NOCROP) &&
        !isHasConvert && IsSameSize(srcImageInfo.size, dstImageInfo.size)) {
        source.useSourceAsResponse_ = true;
        return unique_ptr<PixelMap>(&source);
    }
    unique_ptr<PixelMap> dstPixelMap = make_unique<PixelMap>();
    if (dstPixelMap == nullptr) {
        HiLog::Error(LABEL, "create pixelmap pointer fail");
        return nullptr;
    }
    if (cropType == CropValue::VALID) {
        dstImageInfo.size.width = srcRect.width;
        dstImageInfo.size.height = srcRect.height;
    } else {
        dstImageInfo.size = srcImageInfo.size;
    }
    if (dstPixelMap->SetImageInfo(dstImageInfo) != SUCCESS) {
        return nullptr;
    }
    // dst pixelmap is source crop and convert pixelmap
    if ((cropType == CropValue::VALID) || isHasConvert) {
        if (!SourceCropAndConvert(source, srcImageInfo, dstImageInfo, srcRect, *dstPixelMap.get())) {
            return nullptr;
        }
    } else {
        // only maybe size changed, copy source as scale operation
        if (!CopyPixelMap(source, *dstPixelMap.get())) {
            return nullptr;
        }
    }
    if (!ScalePixelMap(targetSize, dstImageInfo.size, opts.scaleMode, *dstPixelMap.get())) {
        return nullptr;
    }
    dstPixelMap->SetEditable(opts.editable);
    return dstPixelMap;
}

bool PixelMap::SourceCropAndConvert(PixelMap &source, const ImageInfo &srcImageInfo, const ImageInfo &dstImageInfo,
                                    const Rect &srcRect, PixelMap &dstPixelMap)
{
    uint32_t bufferSize = dstPixelMap.GetByteCount();
    void *dstPixels = malloc(bufferSize);
    if (dstPixels == nullptr) {
        HiLog::Error(LABEL, "allocate memory size %{public}u fail", bufferSize);
        return false;
    }

    Position srcPosition{ srcRect.left, srcRect.top };
    if (!PixelConvertAdapter::ReadPixelsConvert(source.GetPixels(), srcPosition, source.GetRowBytes(), srcImageInfo,
        dstPixels, dstPixelMap.GetRowBytes(), dstImageInfo)) {
        HiLog::Error(LABEL, "pixel convert in adapter failed.");
        free(dstPixels);
        dstPixels = nullptr;
        return false;
    }
    dstPixelMap.SetPixelsAddr(dstPixels, nullptr, bufferSize, AllocatorType::HEAP_ALLOC, nullptr);
    return true;
}

bool PixelMap::ScalePixelMap(const Size &targetSize, const Size &dstSize, const ScaleMode &scaleMode,
                             PixelMap &dstPixelMap)
{
    if (dstSize.width == targetSize.width && dstSize.height == targetSize.height) {
        return true;
    }
    PostProc postProc;
    if (scaleMode == ScaleMode::FIT_TARGET_SIZE) {
        if (!postProc.ScalePixelMap(targetSize, dstPixelMap)) {
            HiLog::Error(LABEL, "scale FIT_TARGET_SIZE fail");
            return false;
        }
    }
    if (scaleMode == ScaleMode::CENTER_CROP) {
        if (!postProc.CenterScale(targetSize, dstPixelMap)) {
            HiLog::Error(LABEL, "scale CENTER_CROP fail");
            return false;
        }
    }
    return true;
}

void PixelMap::InitDstImageInfo(const InitializationOptions &opts, const ImageInfo &srcImageInfo,
                                ImageInfo &dstImageInfo)
{
    dstImageInfo.size = opts.size;
    if (dstImageInfo.size.width == 0 && dstImageInfo.size.height == 0) {
        dstImageInfo.size = srcImageInfo.size;
    }
    dstImageInfo.pixelFormat = opts.pixelFormat;
    if (dstImageInfo.pixelFormat == PixelFormat::UNKNOWN) {
        dstImageInfo.pixelFormat = srcImageInfo.pixelFormat;
    }
    dstImageInfo.alphaType = opts.alphaType;
    if (dstImageInfo.alphaType == AlphaType::IMAGE_ALPHA_TYPE_UNKNOWN) {
        dstImageInfo.alphaType = srcImageInfo.alphaType;
    }
}

bool PixelMap::CopyPixelMap(PixelMap &source, PixelMap &dstPixelMap)
{
    uint32_t bufferSize = source.GetByteCount();
    if (bufferSize == 0 || source.GetPixels() == nullptr) {
        HiLog::Error(LABEL, "source pixelMap data invalid");
        return false;
    }
    uint8_t *dstPixels = static_cast<uint8_t *>(malloc(bufferSize));
    if (dstPixels == nullptr) {
        HiLog::Error(LABEL, "allocate memory size %{public}u fail", bufferSize);
        return false;
    }
    errno_t errRet = memcpy_s(dstPixels, bufferSize, source.GetPixels(), bufferSize);
    if (errRet != 0) {
        HiLog::Error(LABEL, "copy source memory size %{public}u fail, errorCode = %{public}d", bufferSize, errRet);
        free(dstPixels);
        dstPixels = nullptr;
        return false;
    }
    dstPixelMap.SetPixelsAddr(dstPixels, nullptr, bufferSize, AllocatorType::HEAP_ALLOC, nullptr);
    return true;
}

bool PixelMap::IsSameSize(const Size &src, const Size &dst)
{
    return (src.width == dst.width) && (src.height == dst.height);
}

bool PixelMap::GetPixelFormatDetail(const PixelFormat format)
{
    switch (format) {
        case PixelFormat::RGBA_8888: {
            pixelBytes_ = ARGB_8888_BYTES;
            colorProc_ = RGBA8888ToARGB;
            break;
        }
        case PixelFormat::BGRA_8888: {
            pixelBytes_ = ARGB_8888_BYTES;
            colorProc_ = BGRA8888ToARGB;
            break;
        }
        case PixelFormat::ARGB_8888: {
            pixelBytes_ = ARGB_8888_BYTES;
            colorProc_ = ARGB8888ToARGB;
            break;
        }
        case PixelFormat::ALPHA_8: {
            pixelBytes_ = ALPHA_8_BYTES;
            colorProc_ = ALPHA8ToARGB;
            break;
        }
        case PixelFormat::RGB_565: {
            pixelBytes_ = RGB_565_BYTES;
            colorProc_ = RGB565ToARGB;
            break;
        }
        case PixelFormat::RGB_888: {
            pixelBytes_ = RGB_888_BYTES;
            colorProc_ = RGB888ToARGB;
            break;
        }
        case PixelFormat::NV12:
        case PixelFormat::NV21: {
            pixelBytes_ = YUV420_BYTES;
            break;
        }
        case PixelFormat::CMYK:
            pixelBytes_ = ARGB_8888_BYTES;
            break;
        default: {
            HiLog::Error(LABEL, "pixel format:[%{public}d] not supported.", format);
            return false;
        }
    }
    return true;
}

uint32_t PixelMap::SetImageInfo(ImageInfo &info)
{
    return SetImageInfo(info, false);
}

uint32_t PixelMap::SetImageInfo(ImageInfo &info, bool isReused)
{
    if (info.size.width <= 0 || info.size.height <= 0) {
        HiLog::Error(LABEL, "pixel map image info invalid.");
        return ERR_IMAGE_DATA_ABNORMAL;
    }
    if (!GetPixelFormatDetail(info.pixelFormat)) {
        return ERR_IMAGE_DATA_UNSUPPORT;
    }

    if (pixelBytes_ <= 0) {
        ResetPixelMap();
        HiLog::Error(LABEL, "pixel map bytes is invalid.");
        return ERR_IMAGE_DATA_ABNORMAL;
    }

    if ((static_cast<uint64_t>(pixelBytes_) * info.size.width) > PIXEL_MAP_MAX_RAM_SIZE) {
        ResetPixelMap();
        HiLog::Error(LABEL, "image size is out of range.");
        return ERR_IMAGE_TOO_LARGE;
    }
    if (info.pixelFormat == PixelFormat::ALPHA_8) {
        rowDataSize_ = pixelBytes_ * ((info.size.width + 3) / 4 * 4);
        HiLog::Info(LABEL, "ALPHA_8 rowDataSize_ %{public}d.", rowDataSize_);
    } else {
        rowDataSize_ = pixelBytes_ * info.size.width;
    }
    if (info.size.height > (PIXEL_MAP_MAX_RAM_SIZE / rowDataSize_)) {
        ResetPixelMap();
        HiLog::Error(LABEL, "pixel map byte count out of range.");
        return ERR_IMAGE_TOO_LARGE;
    }
    if (!isReused) {
        FreePixelMap();
    }
    imageInfo_ = info;
    return SUCCESS;
}

const uint8_t *PixelMap::GetPixel8(int32_t x, int32_t y)
{
    if (!CheckValidParam(x, y) || (pixelBytes_ != ALPHA_8_BYTES)) {
        HiLog::Error(LABEL, "get addr8 pixel position:(%{public}d, %{public}d) pixel bytes:%{public}d invalid.", x, y,
                     pixelBytes_);
        return nullptr;
    }
    return (data_ + y * rowDataSize_ + x);
}

const uint16_t *PixelMap::GetPixel16(int32_t x, int32_t y)
{
    if (!CheckValidParam(x, y) || (pixelBytes_ != RGB_565_BYTES)) {
        HiLog::Error(LABEL, "get addr16 pixel position:(%{public}d, %{public}d) pixel bytes:%{public}d invalid.", x, y,
                     pixelBytes_);
        return nullptr;
    }
    // convert uint8_t* to uint16_t*
    return reinterpret_cast<uint16_t *>(data_ + y * rowDataSize_ + (static_cast<uint32_t>(x) << RGB_565_SHIFT));
}

const uint32_t *PixelMap::GetPixel32(int32_t x, int32_t y)
{
    if (!CheckValidParam(x, y) || (pixelBytes_ != ARGB_8888_BYTES)) {
        HiLog::Error(LABEL, "get addr32 pixel position:(%{public}d, %{public}d) pixel bytes:%{public}d invalid.", x, y,
                     pixelBytes_);
        return nullptr;
    }
    // convert uint8_t* to uint32_t*
    return reinterpret_cast<uint32_t *>(data_ + y * rowDataSize_ + (static_cast<uint32_t>(x) << ARGB_8888_SHIFT));
}

const uint8_t *PixelMap::GetPixel(int32_t x, int32_t y)
{
    if (!CheckValidParam(x, y)) {
        HiLog::Error(LABEL, "input pixel position:(%{public}d, %{public}d) invalid.", x, y);
        return nullptr;
    }
    return (data_ + y * rowDataSize_ + (static_cast<uint32_t>(x) * pixelBytes_));
}

bool PixelMap::GetARGB32Color(int32_t x, int32_t y, uint32_t &color)
{
    if (colorProc_ == nullptr) {
        HiLog::Error(LABEL, "pixel format not supported.");
        return false;
    }
    const uint8_t *src = GetPixel(x, y);
    if (src == nullptr) {
        HiLog::Error(LABEL, "get pixel color error.");
        return false;
    }
    // use founction point for frequently called interface
    return colorProc_(src, ONE_PIXEL_SIZE * pixelBytes_, &color, ONE_PIXEL_SIZE);
}

bool PixelMap::ALPHA8ToARGB(const uint8_t *in, uint32_t inCount, uint32_t *out, uint32_t outCount)
{
    if (inCount != outCount) {
        HiLog::Error(LABEL, "input count:%{public}u is not match to output count:%{public}u.", inCount, outCount);
        return false;
    }
    const uint8_t *src = in;
    for (uint32_t i = 0; i < outCount; i++) {
        *out++ = GetColorARGB(*src++, BYTE_ZERO, BYTE_ZERO, BYTE_ZERO);
    }
    return true;
}

bool PixelMap::RGB565ToARGB(const uint8_t *in, uint32_t inCount, uint32_t *out, uint32_t outCount)
{
    if (((inCount / RGB_565_BYTES) != outCount) && ((inCount % RGB_565_BYTES) != 0)) {
        HiLog::Error(LABEL, "input count:%{public}u is not match to output count:%{public}u.", inCount, outCount);
        return false;
    }
    const uint16_t *src = reinterpret_cast<const uint16_t *>(in);
    for (uint32_t i = 0; i < outCount; i++) {
        uint16_t color = *src++;
        *out++ = GetColorARGB(BYTE_FULL, RGB565ToR32(color), RGB565ToG32(color), RGB565ToB32(color));
    }
    return true;
}

bool PixelMap::ARGB8888ToARGB(const uint8_t *in, uint32_t inCount, uint32_t *out, uint32_t outCount)
{
    if (((inCount / ARGB_8888_BYTES) != outCount) && ((inCount % ARGB_8888_BYTES) != 0)) {
        HiLog::Error(LABEL, "input count:%{public}u is not match to output count:%{public}u.", inCount, outCount);
        return false;
    }
    const uint32_t *src = reinterpret_cast<const uint32_t *>(in);
    for (uint32_t i = 0; i < outCount; i++) {
        uint32_t color = *src++;
        *out++ = GetColorARGB(GetColorComp(color, ARGB32_A_SHIFT), GetColorComp(color, ARGB32_R_SHIFT),
                              GetColorComp(color, ARGB32_G_SHIFT), GetColorComp(color, ARGB32_B_SHIFT));
    }
    return true;
}

bool PixelMap::RGBA8888ToARGB(const uint8_t *in, uint32_t inCount, uint32_t *out, uint32_t outCount)
{
    if (((inCount / ARGB_8888_BYTES) != outCount) && ((inCount % ARGB_8888_BYTES) != 0)) {
        HiLog::Error(LABEL, "input count:%{public}u is not match to output count:%{public}u.", inCount, outCount);
        return false;
    }
    const uint32_t *src = reinterpret_cast<const uint32_t *>(in);
    for (uint32_t i = 0; i < outCount; i++) {
        uint32_t color = *src++;
        *out++ = GetColorARGB(GetColorComp(color, RGBA32_A_SHIFT), GetColorComp(color, RGBA32_R_SHIFT),
                              GetColorComp(color, RGBA32_G_SHIFT), GetColorComp(color, RGBA32_B_SHIFT));
    }
    return true;
}

bool PixelMap::BGRA8888ToARGB(const uint8_t *in, uint32_t inCount, uint32_t *out, uint32_t outCount)
{
    if (((inCount / ARGB_8888_BYTES) != outCount) && ((inCount % ARGB_8888_BYTES) != 0)) {
        HiLog::Error(LABEL, "input count:%{public}u is not match to output count:%{public}u.", inCount, outCount);
        return false;
    }
    const uint32_t *src = reinterpret_cast<const uint32_t *>(in);
    for (uint32_t i = 0; i < outCount; i++) {
        uint32_t color = *src++;
        *out++ = GetColorARGB(GetColorComp(color, BGRA32_A_SHIFT), GetColorComp(color, BGRA32_R_SHIFT),
                              GetColorComp(color, BGRA32_G_SHIFT), GetColorComp(color, BGRA32_B_SHIFT));
    }
    return true;
}

bool PixelMap::RGB888ToARGB(const uint8_t *in, uint32_t inCount, uint32_t *out, uint32_t outCount)
{
    if (((inCount / RGB_888_BYTES) != outCount) && ((inCount % RGB_888_BYTES) != 0)) {
        HiLog::Error(LABEL, "input count:%{public}u is not match to output count:%{public}u.", inCount, outCount);
        return false;
    }
    const uint8_t *src = in;
    for (uint32_t i = 0; i < outCount; i++) {
        uint8_t colorR = *src++;
        uint8_t colorG = *src++;
        uint8_t colorB = *src++;
        *out++ = GetColorARGB(BYTE_FULL, colorR, colorG, colorB);
    }
    return true;
}

int32_t PixelMap::GetPixelBytes()
{
    return pixelBytes_;
}

int32_t PixelMap::GetRowBytes()
{
    return rowDataSize_;
}

int32_t PixelMap::GetByteCount()
{
    return rowDataSize_ * imageInfo_.size.height;
}

int32_t PixelMap::GetWidth()
{
    return imageInfo_.size.width;
}

int32_t PixelMap::GetHeight()
{
    return imageInfo_.size.height;
}

int32_t PixelMap::GetBaseDensity()
{
    return imageInfo_.baseDensity;
}

void PixelMap::GetImageInfo(ImageInfo &imageInfo)
{
    imageInfo = imageInfo_;
}

PixelFormat PixelMap::GetPixelFormat()
{
    return imageInfo_.pixelFormat;
}

ColorSpace PixelMap::GetColorSpace()
{
    return imageInfo_.colorSpace;
}

AlphaType PixelMap::GetAlphaType()
{
    return imageInfo_.alphaType;
}

const uint8_t *PixelMap::GetPixels()
{
    return data_;
}

uint8_t PixelMap::GetARGB32ColorA(uint32_t color)
{
    return (color >> ARGB_A_SHIFT) & ARGB_MASK;
}

uint8_t PixelMap::GetARGB32ColorR(uint32_t color)
{
    return (color >> ARGB_R_SHIFT) & ARGB_MASK;
}

uint8_t PixelMap::GetARGB32ColorG(uint32_t color)
{
    return (color >> ARGB_G_SHIFT) & ARGB_MASK;
}

uint8_t PixelMap::GetARGB32ColorB(uint32_t color)
{
    return (color >> ARGB_B_SHIFT) & ARGB_MASK;
}

bool PixelMap::IsSameImage(const PixelMap &other)
{
    if (data_ == nullptr || other.data_ == nullptr) {
        return false;
    }
    if (imageInfo_.size.width != other.imageInfo_.size.width ||
        imageInfo_.size.height != other.imageInfo_.size.height ||
        imageInfo_.pixelFormat != other.imageInfo_.pixelFormat || imageInfo_.alphaType != other.imageInfo_.alphaType) {
        return false;
    }
    uint64_t size = static_cast<uint64_t>(rowDataSize_) * imageInfo_.size.height;
    if (memcmp(data_, other.data_, size) != 0) {
        return false;
    }
    return true;
}

uint32_t PixelMap::ReadPixels(const uint64_t &bufferSize, uint8_t *dst)
{
    if (dst == nullptr) {
        HiLog::Error(LABEL, "read pixels by buffer input dst address is null.");
        return ERR_IMAGE_READ_PIXELMAP_FAILED;
    }
    if (data_ == nullptr) {
        HiLog::Error(LABEL, "read pixels by buffer current PixelMap data is null.");
        return ERR_IMAGE_READ_PIXELMAP_FAILED;
    }
    if (bufferSize < static_cast<uint64_t>(pixelsSize_)) {
        HiLog::Error(LABEL, "read pixels by buffer input dst buffer(%{public}llu) < current pixelmap size(%{public}u).",
                     static_cast<unsigned long long>(bufferSize), pixelsSize_);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    errno_t ret = memcpy_s(dst, bufferSize, data_, pixelsSize_);
    if (ret != 0) {
        HiLog::Error(LABEL, "read pixels by buffer memcpy the pixelmap data to dst fail, error:%{public}d", ret);
        return ERR_IMAGE_READ_PIXELMAP_FAILED;
    }
    return SUCCESS;
}

bool PixelMap::CheckPixelsInput(const uint8_t *dst, const uint64_t &bufferSize, const uint32_t &offset,
                                const uint32_t &stride, const Rect &region)
{
    if (dst == nullptr) {
        HiLog::Error(LABEL, "CheckPixelsInput input dst address is null.");
        return false;
    }
    if (region.left < 0 || region.top < 0 || stride > numeric_limits<int32_t>::max() ||
        static_cast<uint64_t>(offset) > bufferSize) {
        HiLog::Error(
            LABEL,
            "CheckPixelsInput left(%{public}d) or top(%{public}d) or stride(%{public}u) or offset(%{public}u) < 0.",
            region.left, region.top, stride, offset);
        return false;
    }
    if (region.width <= 0 || region.height <= 0 || region.width > MAX_DIMENSION || region.height > MAX_DIMENSION) {
        HiLog::Error(LABEL, "CheckPixelsInput width(%{public}d) or height(%{public}d) is < 0.", region.width,
                     region.height);
        return false;
    }
    if (region.left > GetWidth() - region.width) {
        HiLog::Error(LABEL, "CheckPixelsInput left(%{public}d) + width(%{public}d) is > pixelmap width(%{public}d).",
                     region.left, region.width, GetWidth());
        return false;
    }
    if (region.top > GetHeight() - region.height) {
        HiLog::Error(LABEL, "CheckPixelsInput top(%{public}d) + height(%{public}d) is > pixelmap height(%{public}d).",
                     region.top, region.height, GetHeight());
        return false;
    }
    uint32_t regionStride = static_cast<uint32_t>(region.width) * 4;  // bytes count, need multiply by 4
    if (stride < regionStride) {
        HiLog::Error(LABEL, "CheckPixelsInput left(%{public}d) + width(%{public}d) is > pixelmap width(%{public}d).",
                     region.left, region.width, GetWidth());
        return false;
    }
    uint64_t lastLinePos = offset + static_cast<uint64_t>(region.height - 1) * stride;  // "1" is except the last line.
    if (static_cast<uint64_t>(offset) > (bufferSize - regionStride) || lastLinePos > (bufferSize - regionStride)) {
        HiLog::Error(LABEL,
                     "CheckPixelsInput fail, height(%{public}d), width(%{public}d), lastLine(%{public}llu), "
                     "offset(%{public}u), bufferSize:%{public}llu.",
                     region.height, region.width, static_cast<unsigned long long>(lastLinePos), offset,
                     static_cast<unsigned long long>(bufferSize));
        return false;
    }
    return true;
}

uint32_t PixelMap::ReadPixels(const uint64_t &bufferSize, const uint32_t &offset, const uint32_t &stride,
                              const Rect &region, uint8_t *dst)
{
    if (!CheckPixelsInput(dst, bufferSize, offset, stride, region)) {
        HiLog::Error(LABEL, "read pixels by rect input parameter fail.");
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    if (data_ == nullptr) {
        HiLog::Error(LABEL, "read pixels by rect this pixel data is null.");
        return ERR_IMAGE_READ_PIXELMAP_FAILED;
    }
    ImageInfo dstImageInfo =
        MakeImageInfo(region.width, region.height, PixelFormat::BGRA_8888, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);
    Position srcPosition{ region.left, region.top };
    if (!PixelConvertAdapter::ReadPixelsConvert(data_, srcPosition, rowDataSize_, imageInfo_, dst + offset, stride,
        dstImageInfo)) {
        HiLog::Error(LABEL, "read pixels by rect call ReadPixelsConvert fail.");
        return ERR_IMAGE_READ_PIXELMAP_FAILED;
    }
    return SUCCESS;
}

uint32_t PixelMap::ReadPixel(const Position &pos, uint32_t &dst)
{
    if (pos.x < 0 || pos.y < 0 || pos.x >= GetWidth() || pos.y >= GetHeight()) {
        HiLog::Error(LABEL, "read pixel by pos input invalid exception. [x(%{public}d), y(%{public}d)]", pos.x, pos.y);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    if (data_ == nullptr) {
        HiLog::Error(LABEL, "read pixel by pos source data is null.");
        return ERR_IMAGE_READ_PIXELMAP_FAILED;
    }
    ImageInfo dstImageInfo =
        MakeImageInfo(PER_PIXEL_LEN, PER_PIXEL_LEN, PixelFormat::BGRA_8888, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);
    uint32_t dstRowBytes = BGRA_BYTES;
    Position srcPosition{ pos.x, pos.y };
    if (!PixelConvertAdapter::ReadPixelsConvert(data_, srcPosition, rowDataSize_, imageInfo_, &dst, dstRowBytes,
        dstImageInfo)) {
        HiLog::Error(LABEL, "read pixel by pos call ReadPixelsConvert fail.");
        return ERR_IMAGE_READ_PIXELMAP_FAILED;
    }
    return SUCCESS;
}

uint32_t PixelMap::ResetConfig(const Size &size, const PixelFormat &format)
{
    if (size.width <= 0 || size.height <= 0) {
        HiLog::Error(LABEL, "ResetConfig reset input width(%{public}d) or height(%{public}d) is < 0.", size.width,
                     size.height);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    uint32_t bytesPerPixel = ImageUtils::GetPixelBytes(format);
    if (bytesPerPixel == 0) {
        HiLog::Error(LABEL, "ResetConfig get bytes by per pixel fail.");
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    uint64_t dstSize = static_cast<uint64_t>(size.width) * size.height * bytesPerPixel;
    if (dstSize > static_cast<uint64_t>(pixelsSize_)) {
        HiLog::Error(LABEL, "ResetConfig reset dstSize(%{public}llu) > current(%{public}u).",
                     static_cast<unsigned long long>(dstSize), pixelsSize_);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    AlphaType dstAlphaType = ImageUtils::GetValidAlphaTypeByFormat(GetAlphaType(), format);
    if (dstAlphaType == AlphaType::IMAGE_ALPHA_TYPE_UNKNOWN) {
        HiLog::Error(LABEL, "ResetConfig Failed to get validate alpha type.");
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    ImageInfo dstInfo = MakeImageInfo(size.width, size.height, format, dstAlphaType);
    uint32_t ret = SetImageInfo(dstInfo, true);
    if (ret != SUCCESS) {
        HiLog::Error(LABEL, "ResetConfig call SetImageInfo Failed. ret:%{public}u", ret);
        return ERR_IMAGE_CONFIG_FAILED;
    }
    return SUCCESS;
}

bool PixelMap::SetAlphaType(const AlphaType &alphaType)
{
    AlphaType type = ImageUtils::GetValidAlphaTypeByFormat(alphaType, imageInfo_.pixelFormat);
    if (type == AlphaType::IMAGE_ALPHA_TYPE_UNKNOWN) {
        HiLog::Error(LABEL, "SetAlphaType Failed to get validate alpha type.");
        return false;
    }
    ImageInfo dstInfo = imageInfo_;
    dstInfo.alphaType = type;
    uint32_t ret = SetImageInfo(dstInfo, true);
    if (ret != SUCCESS) {
        HiLog::Error(LABEL, "SetAlphaType call SetImageInfo Failed. ret:%{public}u", ret);
        return false;
    }
    return true;
}

uint32_t PixelMap::WritePixel(const Position &pos, const uint32_t &color)
{
    if (pos.x < 0 || pos.y < 0 || pos.x >= GetWidth() || pos.y >= GetHeight()) {
        HiLog::Error(LABEL,
                     "write pixel by pos but input position is invalid. [x(%{public}d), y(%{public}d)]"\
                     "Width() %{public}d,  Height() %{public}d, ",
                     pos.x, pos.y, GetWidth(), GetHeight());
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    if (!IsEditable()) {
        HiLog::Error(LABEL, "write pixel by pos pixelmap is not editable.");
        return ERR_IMAGE_PIXELMAP_NOT_ALLOW_MODIFY;
    }
    if (!ImageUtils::IsValidImageInfo(imageInfo_)) {
        HiLog::Error(LABEL, "write pixel by pos current pixelmap image info is invalid.");
        return ERR_IMAGE_WRITE_PIXELMAP_FAILED;
    }
    if (data_ == nullptr) {
        HiLog::Error(LABEL, "write pixel by pos but current pixelmap data is nullptr.");
        return ERR_IMAGE_WRITE_PIXELMAP_FAILED;
    }
    ImageInfo srcImageInfo =
        MakeImageInfo(PER_PIXEL_LEN, PER_PIXEL_LEN, PixelFormat::BGRA_8888, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);
    uint32_t srcRowBytes = BGRA_BYTES;
    Position dstPosition{ pos.x, pos.y };  // source is per pixel.
    if (!PixelConvertAdapter::WritePixelsConvert(&color, srcRowBytes, srcImageInfo, data_, dstPosition, rowDataSize_,
        imageInfo_)) {
        HiLog::Error(LABEL, "write pixel by pos call WritePixelsConvert fail.");
        return ERR_IMAGE_WRITE_PIXELMAP_FAILED;
    }
    return SUCCESS;
}

uint32_t PixelMap::WritePixels(const uint8_t *source, const uint64_t &bufferSize, const uint32_t &offset,
                               const uint32_t &stride, const Rect &region)
{
    if (!CheckPixelsInput(source, bufferSize, offset, stride, region)) {
        HiLog::Error(LABEL, "write pixel by rect input parameter fail.");
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    if (!IsEditable()) {
        HiLog::Error(LABEL, "write pixel by rect pixelmap data is not editable.");
        return ERR_IMAGE_PIXELMAP_NOT_ALLOW_MODIFY;
    }
    if (!ImageUtils::IsValidImageInfo(imageInfo_)) {
        HiLog::Error(LABEL, "write pixel by rect current pixelmap image info is invalid.");
        return ERR_IMAGE_WRITE_PIXELMAP_FAILED;
    }
    if (data_ == nullptr) {
        HiLog::Error(LABEL, "write pixel by rect current pixel map data is null.");
        return ERR_IMAGE_WRITE_PIXELMAP_FAILED;
    }
    uint32_t bytesPerPixel = ImageUtils::GetPixelBytes(imageInfo_.pixelFormat);
    if (bytesPerPixel == 0) {
        HiLog::Error(LABEL, "write pixel by rect get bytes by per pixel fail.");
        return ERR_IMAGE_WRITE_PIXELMAP_FAILED;
    }
    Position dstPosition{ region.left, region.top };
    ImageInfo srcInfo =
        MakeImageInfo(region.width, region.height, PixelFormat::BGRA_8888, AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL);
    if (!PixelConvertAdapter::WritePixelsConvert(source + offset, stride, srcInfo, data_, dstPosition, rowDataSize_,
        imageInfo_)) {
        HiLog::Error(LABEL, "write pixel by rect call WritePixelsConvert fail.");
        return ERR_IMAGE_WRITE_PIXELMAP_FAILED;
    }
    return SUCCESS;
}

uint32_t PixelMap::WritePixels(const uint8_t *source, const uint64_t &bufferSize)
{
    if (source == nullptr || bufferSize < static_cast<uint64_t>(pixelsSize_)) {
        HiLog::Error(LABEL, "write pixels by buffer source is nullptr or size(%{public}llu) < pixelSize(%{public}u).",
                     static_cast<unsigned long long>(bufferSize), pixelsSize_);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    if (!IsEditable()) {
        HiLog::Error(LABEL, "write pixels by buffer pixelmap data is not editable.");
        return ERR_IMAGE_PIXELMAP_NOT_ALLOW_MODIFY;
    }
    if (!ImageUtils::IsValidImageInfo(imageInfo_)) {
        HiLog::Error(LABEL, "write pixels by buffer current pixelmap image info is invalid.");
        return ERR_IMAGE_WRITE_PIXELMAP_FAILED;
    }
    if (data_ == nullptr) {
        HiLog::Error(LABEL, "write pixels by buffer current pixelmap data is nullptr.");
        return ERR_IMAGE_WRITE_PIXELMAP_FAILED;
    }
    errno_t ret = memcpy_s(data_, pixelsSize_, source, pixelsSize_);
    if (ret != 0) {
        HiLog::Error(LABEL, "write pixels by buffer memcpy to pixelmap data from source fail, error:%{public}d", ret);
        return ERR_IMAGE_WRITE_PIXELMAP_FAILED;
    }
    return SUCCESS;
}

bool PixelMap::WritePixels(const uint32_t &color)
{
    if (!IsEditable()) {
        HiLog::Error(LABEL, "erase pixels by color pixelmap data is not editable.");
        return false;
    }
    if (!ImageUtils::IsValidImageInfo(imageInfo_)) {
        HiLog::Error(LABEL, "erase pixels by color current pixelmap image info is invalid.");
        return false;
    }
    if (data_ == nullptr) {
        HiLog::Error(LABEL, "erase pixels by color current pixel map data is null.");
        return false;
    }
    ImageInfo srcInfo =
        MakeImageInfo(imageInfo_.size.width, imageInfo_.size.height, imageInfo_.pixelFormat, imageInfo_.alphaType);
    if (!PixelConvertAdapter::EraseBitmap(data_, rowDataSize_, srcInfo, color)) {
        HiLog::Error(LABEL, "erase pixels by color call EraseBitmap fail.");
        return false;
    }
    return true;
}

AllocatorType PixelMap::GetAllocatorType()
{
    return allocatorType_;
}

void *PixelMap::GetFd() const
{
    return context_;
}

bool PixelMap::WriteFileDescriptor(Parcel &data, int fd) const
{
    if (fd < 0) {
        return false;
    }
    int dupFd = dup(fd);
    if (dupFd < 0) {
        return false;
    }
    sptr<IPCFileDescriptor> descriptor = new IPCFileDescriptor(dupFd);
    return data.WriteObject<IPCFileDescriptor>(descriptor);
}

int PixelMap::ReadFileDescriptor(Parcel &data)
{
    sptr<IPCFileDescriptor> descriptor = data.ReadObject<IPCFileDescriptor>();
    if (descriptor == nullptr) {
        return -1;
    }
    int fd = descriptor->GetFd();
    if (fd < 0) {
        return -1;
    }
    return dup(fd);
}

bool PixelMap::Marshalling(Parcel &data) const
{
    int32_t PIXEL_MAP_INFO_MAX_LENGTH = 128;
    int32_t bufferSize = rowDataSize_ * imageInfo_.size.height;

    if (static_cast<size_t>(bufferSize + PIXEL_MAP_INFO_MAX_LENGTH) > data.GetDataCapacity() &&
        !data.SetDataCapacity(bufferSize + PIXEL_MAP_INFO_MAX_LENGTH)) {
        HiLog::Error(LABEL, "set parcel max capacity:[%{public}d] failed.", bufferSize + PIXEL_MAP_INFO_MAX_LENGTH);
        return false;
    }
    if (!data.WriteInt32(imageInfo_.size.width)) {
        HiLog::Error(LABEL, "write pixel map width:[%{public}d] to parcel failed.", imageInfo_.size.width);
        return false;
    }
    if (!data.WriteInt32(imageInfo_.size.height)) {
        HiLog::Error(LABEL, "write pixel map height:[%{public}d] to parcel failed.", imageInfo_.size.height);
        return false;
    }
    if (!data.WriteInt32(static_cast<int32_t>(imageInfo_.pixelFormat))) {
        HiLog::Error(LABEL, "write pixel map pixel format:[%{public}d] to parcel failed.", imageInfo_.pixelFormat);
        return false;
    }
    if (!data.WriteInt32(static_cast<int32_t>(imageInfo_.colorSpace))) {
        HiLog::Error(LABEL, "write pixel map color space:[%{public}d] to parcel failed.", imageInfo_.colorSpace);
        return false;
    }
    if (!data.WriteInt32(static_cast<int32_t>(imageInfo_.alphaType))) {
        HiLog::Error(LABEL, "write pixel map alpha type:[%{public}d] to parcel failed.", imageInfo_.alphaType);
        return false;
    }
    if (!data.WriteInt32(imageInfo_.baseDensity)) {
        HiLog::Error(LABEL, "write pixel map base density:[%{public}d] to parcel failed.", imageInfo_.baseDensity);
        return false;
    }
    if (!data.WriteInt32(bufferSize)) {
        HiLog::Error(LABEL, "write pixel map buffer size:[%{public}d] to parcel failed.", bufferSize);
        return false;
    }
    if (!data.WriteInt32(static_cast<int32_t>(allocatorType_))) {
        HiLog::Error(LABEL, "write pixel map allocator type:[%{public}d] to parcel failed.",
                     allocatorType_);
        return false;
    }
    if (allocatorType_ == AllocatorType::SHARE_MEM_ALLOC) {
        int *fd = static_cast<int *>(context_);
        if (*fd < 0) {
            HiLog::Error(LABEL, "write pixel map failed, fd < 0.");
            return false;
        }
        if (!WriteFileDescriptor(data, *fd)) {
            HiLog::Error(LABEL, "write pixel map fd:[%{public}d] to parcel failed.", *fd);
            return false;
        }
    } else {
        const uint8_t *addr = data_;
        if (addr == nullptr) {
            HiLog::Error(LABEL, "write to parcel failed, pixel memory is null.");
            return false;
        }
        if (!data.WriteBuffer(addr, bufferSize)) {
            HiLog::Error(LABEL, "write pixel map buffer to parcel failed.");
            return false;
        }
    }
    return true;
}

PixelMap *PixelMap::Unmarshalling(Parcel &data)
{
    PixelMap *pixelMap = new PixelMap();
    if (pixelMap == nullptr) {
        HiLog::Error(LABEL, "create pixelmap pointer fail");
        return nullptr;
    }

    ImageInfo imgInfo;
    imgInfo.size.width = data.ReadInt32();
    imgInfo.size.height = data.ReadInt32();
    imgInfo.pixelFormat = static_cast<PixelFormat>(data.ReadInt32());
    imgInfo.colorSpace = static_cast<ColorSpace>(data.ReadInt32());
    imgInfo.alphaType = static_cast<AlphaType>(data.ReadInt32());
    imgInfo.baseDensity = data.ReadInt32();
    int32_t bufferSize = data.ReadInt32();
    AllocatorType allocType = static_cast<AllocatorType>(data.ReadInt32());
    uint8_t *base = nullptr;
    void *context = nullptr;
    if (allocType == AllocatorType::SHARE_MEM_ALLOC) {
        int fd = ReadFileDescriptor(data);
        if (fd < 0) {
            HiLog::Error(LABEL, "fd < 0");
            return nullptr;
        }
        HiLog::Debug(LABEL, "ReadFileDescriptor fd %{public}d.", fd);
        void* ptr = ::mmap(nullptr, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (ptr == MAP_FAILED) {
            ::close(fd);
            HiLog::Error(LABEL, "shared memory map failed");
            return nullptr;
        }
        context = new int32_t();
        if (context == nullptr) {
            HiLog::Error(LABEL, "alloc context error.");
            ::munmap(ptr, bufferSize);
            ::close(fd);
            return nullptr;
        }
        *static_cast<int32_t *>(context) = fd;
        base = static_cast<uint8_t *>(ptr);
    } else {
        const uint8_t *addr = data.ReadBuffer(bufferSize);
        if (addr == nullptr) {
            HiLog::Error(LABEL, "read buffer from parcel failed, read buffer addr is null");
            return nullptr;
        }
        base = static_cast<uint8_t *>(malloc(bufferSize));
        if (base == nullptr) {
            HiLog::Error(LABEL, "alloc output pixel memory size:[%{public}d] error.", bufferSize);
            return nullptr;
        }
        if (memcpy_s(base, bufferSize, addr, bufferSize) != 0) {
            free(base);
            base = nullptr;
            HiLog::Error(LABEL, "memcpy pixel data size:[%{public}d] error.", bufferSize);
            return nullptr;
        }
    }

    uint32_t ret = pixelMap->SetImageInfo(imgInfo);
    if (ret != SUCCESS) {
        if (allocType == AllocatorType::SHARE_MEM_ALLOC) {
            int *fd = static_cast<int *>(context);
            if (base != nullptr) {
                ::munmap(base, bufferSize);
            }
            if (fd != nullptr) {
                ::close(*fd);
                delete fd;
            }
        } else if (allocType == AllocatorType::HEAP_ALLOC) {
            if (base != nullptr) {
                free(base);
                base = nullptr;
            }
        }
        HiLog::Error(LABEL, "create pixel map from parcel failed, set image info error.");
        return nullptr;
    }
    pixelMap->SetPixelsAddr(base, context, bufferSize, allocType, nullptr);
    return pixelMap;
}
} // namespace Media
} // namespace OHOS
