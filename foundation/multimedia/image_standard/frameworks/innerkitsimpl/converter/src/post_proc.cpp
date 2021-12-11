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

#include "post_proc.h"
#include <unistd.h>
#include "basic_transformer.h"
#include "image_log.h"
#include "image_trace.h"
#include "image_utils.h"
#include "media_errors.h"
#include "pixel_convert_adapter.h"
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
using namespace std;
constexpr uint32_t NEED_NEXT = 1;
constexpr float EPSILON = 1e-6;
constexpr uint8_t HALF = 2;

uint32_t PostProc::DecodePostProc(const DecodeOptions &opts, PixelMap &pixelMap, FinalOutputStep finalOutputStep)
{
    ImageInfo srcImageInfo;
    pixelMap.GetImageInfo(srcImageInfo);
    ImageInfo dstImageInfo;
    GetDstImageInfo(opts, pixelMap, srcImageInfo, dstImageInfo);
    if (finalOutputStep == FinalOutputStep::ROTATE_CHANGE || finalOutputStep == FinalOutputStep::SIZE_CHANGE ||
        finalOutputStep == FinalOutputStep::DENSITY_CHANGE) {
        decodeOpts_.allocatorType = AllocatorType::HEAP_ALLOC;
    }
    uint32_t errorCode = ConvertProc(opts.CropRect, dstImageInfo, pixelMap, srcImageInfo);
    if (errorCode != SUCCESS) {
        IMAGE_LOGE("[PostProc]crop pixel map failed, errcode:%{public}u", errorCode);
        return errorCode;
    }
    decodeOpts_.allocatorType = opts.allocatorType;
    bool isNeedRotate = !ImageUtils::FloatCompareZero(opts.rotateDegrees);
    if (isNeedRotate) {
        if (finalOutputStep == FinalOutputStep::SIZE_CHANGE || finalOutputStep == FinalOutputStep::DENSITY_CHANGE) {
            decodeOpts_.allocatorType = AllocatorType::HEAP_ALLOC;
        }
        if (!RotatePixelMap(opts.rotateDegrees, pixelMap)) {
            IMAGE_LOGE("[PostProc]rotate:transform pixel map failed");
            return ERR_IMAGE_TRANSFORM;
        }
    }
    decodeOpts_.allocatorType = opts.allocatorType;
    if (opts.desiredSize.height > 0 && opts.desiredSize.width > 0) {
        if (!ScalePixelMap(opts.desiredSize, pixelMap)) {
            IMAGE_LOGE("[PostProc]scale:transform pixel map failed");
            return ERR_IMAGE_TRANSFORM;
        }
    } else {
        ImageInfo info;
        pixelMap.GetImageInfo(info);
        if ((finalOutputStep == FinalOutputStep::DENSITY_CHANGE) && (info.baseDensity != 0)) {
            int targetWidth = (pixelMap.GetWidth() * opts.fitDensity + (info.baseDensity >> 1)) / info.baseDensity;
            int targetHeight = (pixelMap.GetHeight() * opts.fitDensity + (info.baseDensity >> 1)) / info.baseDensity;
            Size size;
            size.height = targetHeight;
            size.width = targetWidth;
            if (!ScalePixelMap(size, pixelMap)) {
                IMAGE_LOGE("[PostProc]density scale:transform pixel map failed");
                return ERR_IMAGE_TRANSFORM;
            }
        }
    }
    return SUCCESS;
}

void PostProc::GetDstImageInfo(const DecodeOptions &opts, PixelMap &pixelMap,
                               ImageInfo srcImageInfo, ImageInfo &dstImageInfo)
{
    dstImageInfo.size = opts.desiredSize;
    dstImageInfo.pixelFormat = opts.desiredPixelFormat;
    dstImageInfo.baseDensity = srcImageInfo.baseDensity;
    decodeOpts_ = opts;
    if (opts.desiredPixelFormat == PixelFormat::UNKNOWN) {
        if (opts.preference == MemoryUsagePreference::LOW_RAM &&
            srcImageInfo.alphaType == AlphaType::IMAGE_ALPHA_TYPE_OPAQUE) {
            dstImageInfo.pixelFormat = PixelFormat::RGB_565;
        } else {
            dstImageInfo.pixelFormat = PixelFormat::RGBA_8888;
        }
    }
    // decode use, this value may be changed by real pixelFormat
    if (pixelMap.GetAlphaType() == AlphaType::IMAGE_ALPHA_TYPE_UNPREMUL) {
        dstImageInfo.alphaType = AlphaType::IMAGE_ALPHA_TYPE_PREMUL;
    } else {
        dstImageInfo.alphaType = pixelMap.GetAlphaType();
    }
}

bool PostProc::CenterScale(const Size &size, PixelMap &pixelMap)
{
    int32_t srcWidth = pixelMap.GetWidth();
    int32_t srcHeight = pixelMap.GetHeight();
    int32_t targetWidth = size.width;
    int32_t targetHeight = size.height;
    if (targetWidth <= 0 || targetHeight <= 0 || srcWidth <= 0 || srcHeight <= 0) {
        IMAGE_LOGE("[PostProc]params invalid, targetWidth:%{public}d, targetHeight:%{public}d, "
                   "srcWidth:%{public}d, srcHeight:%{public}d",
                   targetWidth, targetHeight, srcWidth, srcHeight);
        return false;
    }
    float widthScale = static_cast<float>(targetWidth) / static_cast<float>(srcWidth);
    float heightScale = static_cast<float>(targetHeight) / static_cast<float>(srcHeight);
    float scale = max(widthScale, heightScale);
    if (!ScalePixelMap(scale, scale, pixelMap)) {
        IMAGE_LOGE("[PostProc]center scale pixelmap %{public}f fail", scale);
        return false;
    }
    srcWidth = pixelMap.GetWidth();
    srcHeight = pixelMap.GetHeight();
    if (srcWidth == targetWidth && srcHeight == targetHeight) {
        return true;
    }
    if (srcWidth < targetWidth || srcHeight < targetHeight) {
        IMAGE_LOGE("[PostProc]src size [%{public}d, %{public}d] must less than dst size [%{public}d, %{public}d]",
                   srcWidth, srcHeight, targetWidth, targetHeight);
        return false;
    }
    if (CenterDisplay(pixelMap, srcWidth, srcHeight, targetWidth, targetHeight)) {
        return true;
    }
    return false;
}

bool PostProc::CenterDisplay(PixelMap &pixelMap, int32_t srcWidth, int32_t srcHeight, int32_t targetWidth,
                             int32_t targetHeight)
{
    ImageInfo dstImageInfo;
    pixelMap.GetImageInfo(dstImageInfo);
    dstImageInfo.size.width = targetWidth;
    dstImageInfo.size.height = targetHeight;
    if (pixelMap.SetImageInfo(dstImageInfo, true) != SUCCESS) {
        IMAGE_LOGE("update ImageInfo failed");
        return false;
    }

    int32_t left = max(0, srcWidth - targetWidth) / HALF;
    int32_t top = max(0, srcHeight - targetHeight) / HALF;
    int32_t bufferSize = pixelMap.GetByteCount();
    uint8_t *dstPixels = nullptr;
    if (!AllocHeapBuffer(bufferSize, &dstPixels)) {
        return false;
    }
    int32_t copyHeight = srcHeight;
    if (srcHeight > targetHeight) {
        copyHeight = targetHeight;
    }
    int32_t copyWidth = srcWidth;
    if (srcWidth > targetWidth) {
        copyWidth = targetWidth;
    }
    int32_t pixelBytes = pixelMap.GetPixelBytes();
    uint8_t *srcPixels = const_cast<uint8_t *>(pixelMap.GetPixels()) + (top * srcWidth + left) * pixelBytes;
    uint8_t *dstStartPixel = nullptr;
    uint8_t *srcStartPixel = nullptr;
    uint32_t targetRowBytes = targetWidth * pixelBytes;
    uint32_t srcRowBytes = srcWidth * pixelBytes;
    uint32_t copyRowBytes = copyWidth * pixelBytes;
    for (int32_t scanLine = 0; scanLine < copyHeight; scanLine++) {
        dstStartPixel = dstPixels + scanLine * targetRowBytes;
        srcStartPixel = srcPixels + scanLine * srcRowBytes;
        errno_t errRet = memcpy_s(dstStartPixel, targetRowBytes, srcStartPixel, copyRowBytes);
        if (errRet != 0) {
            IMAGE_LOGE("[PostProc]memcpy scanline %{public}d fail, errorCode = %{public}d", scanLine, errRet);
            free(dstPixels);
            dstPixels = nullptr;
            return false;
        }
    }
    pixelMap.SetPixelsAddr(dstPixels, nullptr, bufferSize, AllocatorType::HEAP_ALLOC, nullptr);
    return true;
}

uint32_t PostProc::CheckScanlineFilter(const Rect &cropRect, ImageInfo &dstImageInfo, PixelMap &pixelMap,
                                       int32_t pixelBytes, ScanlineFilter &scanlineFilter)
{
    uint64_t bufferSize = static_cast<uint64_t>(dstImageInfo.size.width) * dstImageInfo.size.height * pixelBytes;
    uint8_t *resultData = nullptr;
    int fd = 0;
    if (decodeOpts_.allocatorType == AllocatorType::SHARE_MEM_ALLOC) {
        resultData = AllocSharedMemory(dstImageInfo.size, bufferSize, fd);
        if (resultData == nullptr) {
            IMAGE_LOGE("[PostProc]AllocSharedMemory failed");
            return ERR_IMAGE_CROP;
        }
    } else {
        if (!AllocHeapBuffer(bufferSize, &resultData)) {
            return ERR_IMAGE_CROP;
        }
    }
    auto srcData = pixelMap.GetPixels();
    int32_t scanLine = 0;
    if (ImageUtils::CheckMulOverflow(dstImageInfo.size.width, pixelBytes)) {
        IMAGE_LOGE("[PostProc]size.width:%{public}d, is too large",
            dstImageInfo.size.width);
        ReleaseBuffer(decodeOpts_.allocatorType, fd, bufferSize, &resultData);
        return ERR_IMAGE_CROP;
    }
    uint32_t rowBytes = pixelBytes * dstImageInfo.size.width;
    while (scanLine < pixelMap.GetHeight()) {
        FilterRowType filterRow = scanlineFilter.GetFilterRowType(scanLine);
        if (filterRow == FilterRowType::NON_REFERENCE_ROW) {
            scanLine++;
            continue;
        }
        if (filterRow == FilterRowType::LAST_REFERENCE_ROW) {
            break;
        }
        uint32_t ret = scanlineFilter.FilterLine(resultData + ((scanLine - cropRect.top) * rowBytes), rowBytes,
                                                 srcData + (scanLine * pixelMap.GetRowBytes()));
        if (ret != SUCCESS) {
            IMAGE_LOGE("[PostProc]scan line failed, ret:%{public}u", ret);
            ReleaseBuffer(decodeOpts_.allocatorType, fd, bufferSize, &resultData);
            return ret;
        }
        scanLine++;
    }
    uint32_t ret = pixelMap.SetImageInfo(dstImageInfo);
    if (ret != SUCCESS) {
        ReleaseBuffer(decodeOpts_.allocatorType, fd, bufferSize, &resultData);
        return ret;
    }
    pixelMap.SetPixelsAddr(resultData, nullptr, bufferSize, decodeOpts_.allocatorType, nullptr);
    return ret;
}

uint32_t PostProc::ConvertProc(const Rect &cropRect, ImageInfo &dstImageInfo, PixelMap &pixelMap,
                               ImageInfo &srcImageInfo)
{
    bool hasPixelConvert = HasPixelConvert(srcImageInfo, dstImageInfo);
    uint32_t ret = NeedScanlineFilter(cropRect, srcImageInfo.size, hasPixelConvert);
    if (ret != NEED_NEXT) {
        return ret;
    }

    // we suppose a quick method to scanline in mostly seen cases: NO CROP && hasPixelConvert
    if (GetCropValue(cropRect, srcImageInfo.size) == CropValue::NOCROP
        && dstImageInfo.pixelFormat == PixelFormat::ARGB_8888 && hasPixelConvert) {
        IMAGE_LOGI("[PostProc]no need crop, only pixel convert.");
        return PixelConvertProc(dstImageInfo, pixelMap, srcImageInfo);
    }

    ScanlineFilter scanlineFilter(srcImageInfo.pixelFormat);
    // this method maybe update dst image size to crop size
    SetScanlineCropAndConvert(cropRect, dstImageInfo, srcImageInfo, scanlineFilter, hasPixelConvert);

    int32_t pixelBytes = ImageUtils::GetPixelBytes(dstImageInfo.pixelFormat);
    if (pixelBytes == 0) {
        return ERR_IMAGE_CROP;
    }
    if (ImageUtils::CheckMulOverflow(dstImageInfo.size.width, dstImageInfo.size.height, pixelBytes)) {
        IMAGE_LOGE("[PostProc]size.width:%{public}d, size.height:%{public}d is too large",
            dstImageInfo.size.width, dstImageInfo.size.height);
        return ERR_IMAGE_CROP;
    }
    return CheckScanlineFilter(cropRect, dstImageInfo, pixelMap, pixelBytes, scanlineFilter);
}

uint32_t PostProc::PixelConvertProc(ImageInfo &dstImageInfo, PixelMap &pixelMap,
                                    ImageInfo &srcImageInfo)
{
    uint32_t ret;
    int fd = 0;
    uint64_t bufferSize = 0;
    uint8_t *resultData = nullptr;

    // as no crop, the size is same as src
    dstImageInfo.size = srcImageInfo.size;
    if (AllocBuffer(dstImageInfo, &resultData, bufferSize, fd) != SUCCESS) {
        ReleaseBuffer(decodeOpts_.allocatorType, fd, bufferSize, &resultData);
        return ERR_IMAGE_CROP;
    }

    int32_t pixelBytes = ImageUtils::GetPixelBytes(srcImageInfo.pixelFormat);
    if (pixelBytes == 0) {
        return ERR_IMAGE_CROP;
    }
//    uint32_t rowBytes = pixelBytes * dstImageInfo.size.width;

    // copy src buffer to new buffer
/*
    auto srcData = pixelMap.GetPixels();
    Position dstPosition;
    if (!PixelConvertAdapter::WritePixelsConvert(reinterpret_cast<const void *>(srcData), rowBytes, srcImageInfo,
        resultData, dstPosition, pixelMap.GetRowBytes(), dstImageInfo)) {
        IMAGE_LOGE("pixel convert in adapter failed.");
        ReleaseBuffer(decodeOpts_.allocatorType, fd, bufferSize, &resultData);
        return ERR_IMAGE_CROP;
    }
*/
    ret = pixelMap.SetImageInfo(dstImageInfo);
    if (ret != SUCCESS) {
        ReleaseBuffer(decodeOpts_.allocatorType, fd, bufferSize, &resultData);
        return ret;
    }
    pixelMap.SetPixelsAddr(resultData, nullptr, bufferSize, decodeOpts_.allocatorType, nullptr);
    return ret;
}

uint32_t PostProc::AllocBuffer(ImageInfo imageInfo, uint8_t **resultData, uint64_t &bufferSize, int &fd)
{
    int32_t pixelBytes = ImageUtils::GetPixelBytes(imageInfo.pixelFormat);
    if (pixelBytes == 0) {
        return ERR_IMAGE_CROP;
    }
    if (ImageUtils::CheckMulOverflow(imageInfo.size.width, imageInfo.size.height, pixelBytes)) {
        IMAGE_LOGE("[PostProc]size.width:%{public}d, size.height:%{public}d is too large",
                   imageInfo.size.width, imageInfo.size.height);
        return ERR_IMAGE_CROP;
    }
    bufferSize = static_cast<uint64_t>(imageInfo.size.width) * imageInfo.size.height * pixelBytes;
    IMAGE_LOGD("[PostProc]size.width:%{public}d, size.height:%{public}d, bufferSize:%{public}lld",
               imageInfo.size.width, imageInfo.size.height, (long long)bufferSize);
    if (decodeOpts_.allocatorType == AllocatorType::SHARE_MEM_ALLOC) {
        *resultData = AllocSharedMemory(imageInfo.size, bufferSize, fd);
        if (*resultData == nullptr) {
            IMAGE_LOGE("[PostProc]AllocSharedMemory failed");
            return ERR_IMAGE_CROP;
        }
    } else {
        if (!AllocHeapBuffer(bufferSize, resultData)) {
            return ERR_IMAGE_CROP;
        }
    }
    return SUCCESS;
}

bool PostProc::AllocHeapBuffer(uint64_t bufferSize, uint8_t **buffer)
{
    *buffer = static_cast<uint8_t *>(malloc(bufferSize));
    if (*buffer == nullptr) {
        IMAGE_LOGE("[PostProc]alloc covert color buffersize[%{public}llu] failed.",
                   static_cast<unsigned long long>(bufferSize));
        return false;
    }
#ifdef _WIN32
    memset(*buffer, 0, bufferSize);
    return true;
#else
    errno_t errRet = memset_s(*buffer, bufferSize, 0, bufferSize);
    if (errRet != EOK) {
        IMAGE_LOGE("[PostProc]memset convertData fail, errorCode = %{public}d", errRet);
        ReleaseBuffer(AllocatorType::HEAP_ALLOC, 0, 0, buffer);
        return false;
    }
    return true;
#endif
}

uint8_t *PostProc::AllocSharedMemory(const Size &size, const uint64_t bufferSize, int &fd)
{
#if defined(_WIN32) || defined(_APPLE)
        return nullptr;
#else
    fd = AshmemCreate("Parcel RawData", bufferSize);
    if (fd < 0) {
        IMAGE_LOGE("[PostProc]AllocSharedMemory fd error, bufferSize %{public}lld", (long long)bufferSize);
        return nullptr;
    }
    int result = AshmemSetProt(fd, PROT_READ | PROT_WRITE);
    if (result < 0) {
        IMAGE_LOGE("[PostProc]AshmemSetProt error");
        ::close(fd);
        return nullptr;
    }
    void* ptr = ::mmap(nullptr, bufferSize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (ptr == MAP_FAILED) {
        IMAGE_LOGE("[PostProc]mmap error, errno: %{public}s, fd %{public}d, bufferSize %{public}lld",
            strerror(errno), fd, (long long)bufferSize);
        ::close(fd);
        return nullptr;
    }
    return (uint8_t *)ptr;
#endif
}

void PostProc::ReleaseBuffer(AllocatorType allocatorType, int fd, uint64_t dataSize, uint8_t **buffer)
{
#if !defined(_WIN32) && !defined(_APPLE)
    if (allocatorType == AllocatorType::SHARE_MEM_ALLOC) {
        if (*buffer != nullptr) {
            ::munmap(*buffer, dataSize);
            ::close(fd);
        }
        return;
    }
#endif

    if (allocatorType == AllocatorType::HEAP_ALLOC) {
        if (*buffer != nullptr) {
            free(*buffer);
            *buffer = nullptr;
        }
        return;
    }
}

uint32_t PostProc::NeedScanlineFilter(const Rect &cropRect, const Size &srcSize, const bool &hasPixelConvert)
{
    CropValue value = GetCropValue(cropRect, srcSize);
    if (value == CropValue::NOCROP && !hasPixelConvert) {
        IMAGE_LOGI("[PostProc]no need crop and pixel convert.");
        return SUCCESS;
    } else if (value == CropValue::INVALID) {
        IMAGE_LOGE("[PostProc]invalid corp region, top:%{public}d, left:%{public}d, "
                   "width:%{public}d, height:%{public}d",
                   cropRect.top, cropRect.left, cropRect.width, cropRect.height);
        return ERR_IMAGE_CROP;
    }
    return NEED_NEXT;
}

void PostProc::ConvertPixelMapToPixmapInfo(PixelMap &pixelMap, PixmapInfo &pixmapInfo)
{
    pixmapInfo.imageInfo.size.width = pixelMap.GetWidth();
    pixmapInfo.imageInfo.size.height = pixelMap.GetHeight();
    pixmapInfo.imageInfo.pixelFormat = pixelMap.GetPixelFormat();
    pixmapInfo.imageInfo.colorSpace = pixelMap.GetColorSpace();
    pixmapInfo.imageInfo.alphaType = pixelMap.GetAlphaType();
    pixmapInfo.imageInfo.baseDensity = pixelMap.GetBaseDensity();
    pixmapInfo.data = const_cast<uint8_t *>(pixelMap.GetPixels());
    pixmapInfo.bufferSize = pixelMap.GetByteCount();
}

bool PostProc::RotatePixelMap(float rotateDegrees, PixelMap &pixelMap)
{
    BasicTransformer trans;
    PixmapInfo input(false);
    ConvertPixelMapToPixmapInfo(pixelMap, input);
    // Default rotation at the center of the image, so divide 2
    trans.SetRotateParam(rotateDegrees, static_cast<float>(input.imageInfo.size.width) * FHALF,
                         static_cast<float>(input.imageInfo.size.height) * FHALF);
    return Transform(trans, input, pixelMap);
}

bool PostProc::ScalePixelMap(const Size &size, PixelMap &pixelMap)
{
    int32_t srcWidth = pixelMap.GetWidth();
    int32_t srcHeight = pixelMap.GetHeight();
    if (srcWidth <= 0 || srcHeight <= 0) {
        IMAGE_LOGE("[PostProc]src width:%{public}d, height:%{public}d is invalid.", srcWidth, srcHeight);
        return false;
    }
    float scaleX = static_cast<float>(size.width) / static_cast<float>(srcWidth);
    float scaleY = static_cast<float>(size.height) / static_cast<float>(srcHeight);
    return ScalePixelMap(scaleX, scaleY, pixelMap);
}

bool PostProc::ScalePixelMap(float scaleX, float scaleY, PixelMap &pixelMap)
{
    // returns directly with a scale of 1.0
    if ((fabs(scaleX - 1.0f) < EPSILON) && (fabs(scaleY - 1.0f) < EPSILON)) {
        return true;
    }
    BasicTransformer trans;
    PixmapInfo input(false);
    ConvertPixelMapToPixmapInfo(pixelMap, input);

    trans.SetScaleParam(scaleX, scaleY);
    return Transform(trans, input, pixelMap);
}

bool PostProc::Transform(BasicTransformer &trans, const PixmapInfo &input, PixelMap &pixelMap)
{
    PixmapInfo output(false);
    uint32_t ret;
    if (decodeOpts_.allocatorType == AllocatorType::SHARE_MEM_ALLOC) {
        typedef uint8_t *(*AllocMemory)(const Size &size, const uint64_t bufferSize, int &fd);
        AllocMemory allcFunc = AllocSharedMemory;
        ret = trans.TransformPixmap(input, output, allcFunc);
    } else {
        ret = trans.TransformPixmap(input, output);
    }
    if (ret != IMAGE_SUCCESS) {
        output.Destroy();
        return false;
    }

    if (pixelMap.SetImageInfo(output.imageInfo) != SUCCESS) {
        output.Destroy();
        return false;
    }
    pixelMap.SetPixelsAddr(output.data, nullptr, output.bufferSize, decodeOpts_.allocatorType, nullptr);
    return true;
}

CropValue PostProc::GetCropValue(const Rect &rect, const Size &size)
{
    bool isSameSize = (rect.top == 0 && rect.left == 0 && rect.height == size.height && rect.width == size.width);
    if (!IsHasCrop(rect) || isSameSize) {
        return CropValue::NOCROP;
    }
    bool isValid = ((rect.top >= 0 && rect.width > 0 && rect.left >= 0 && rect.height > 0) &&
                    (rect.top + rect.height <= size.height) && (rect.left + rect.width <= size.width));
    if (!isValid) {
        return CropValue::INVALID;
    }
    return CropValue::VALID;
}

bool PostProc::IsHasCrop(const Rect &rect)
{
    return (rect.top != 0 || rect.left != 0 || rect.width != 0 || rect.height != 0);
}

bool PostProc::HasPixelConvert(const ImageInfo &srcImageInfo, ImageInfo &dstImageInfo)
{
    dstImageInfo.alphaType = ImageUtils::GetValidAlphaTypeByFormat(dstImageInfo.alphaType, dstImageInfo.pixelFormat);
    return (dstImageInfo.pixelFormat != srcImageInfo.pixelFormat || dstImageInfo.alphaType != srcImageInfo.alphaType);
}

void PostProc::SetScanlineCropAndConvert(const Rect &cropRect, ImageInfo &dstImageInfo, ImageInfo &srcImageInfo,
                                         ScanlineFilter &scanlineFilter, bool hasPixelConvert)
{
    if (hasPixelConvert) {
        scanlineFilter.SetPixelConvert(srcImageInfo, dstImageInfo);
    }

    Rect srcRect = cropRect;
    if (IsHasCrop(cropRect)) {
        dstImageInfo.size.width = cropRect.width;
        dstImageInfo.size.height = cropRect.height;
    } else {
        srcRect = { 0, 0, srcImageInfo.size.width, srcImageInfo.size.height };
        dstImageInfo.size = srcImageInfo.size;
    }
    scanlineFilter.SetSrcRegion(srcRect);
}
} // namespace Media
} // namespace OHOS
