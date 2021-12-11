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

#include "basic_transformer.h"
#include <iostream>
#include <new>
#include <unistd.h>
#include "image_utils.h"
#include "pixel_convert.h"
#include "pixel_map.h"
#ifndef _WIN32
#include "securec.h"
#else
#include "memory.h"
#endif

#if !defined(_WIN32) && !defined(_APPLE)
#include "ashmem.h"
#include <sys/mman.h>
#endif

namespace OHOS {
namespace Media {
using namespace std;
void BasicTransformer::ResetParam()
{
    matrix_ = Matrix();
    minX_ = 0.0f;
    minY_ = 0.0f;
}

void BasicTransformer::SetScaleParam(const float sx, const float sy)
{
    Matrix m;
    m.SetScale(sx, sy);
    matrix_.SetConcat(m);
}

void BasicTransformer::SetTranslateParam(const float tx, const float ty)
{
    Matrix m;
    m.SetTranslate(tx, ty);
    matrix_.SetConcat(m);
}

void BasicTransformer::SetRotateParam(const float degrees, const float px, const float py)
{
    Matrix m;
    m.SetRotate(degrees, px, py);
    matrix_.SetConcat(m);
}

void BasicTransformer::GetDstDimension(const Size &srcSize, Size &dstSize)
{
    Matrix::OperType operType = matrix_.GetOperType();
    if ((static_cast<uint8_t>(operType) & Matrix::SCALE) == Matrix::SCALE) {
        dstSize.width = static_cast<int32_t>(srcSize.width * matrix_.GetScaleX() + FHALF);
        dstSize.height = static_cast<int32_t>(srcSize.height * matrix_.GetScaleY() + FHALF);
    }

    if ((static_cast<uint8_t>(operType) & Matrix::ROTATEORSKEW) == Matrix::ROTATEORSKEW) {
        Matrix::CalcXYProc fInvProc = Matrix::GetXYProc(operType);
        GetRotateDimension(fInvProc, srcSize, dstSize);
    }
}

bool BasicTransformer::CheckAllocateBuffer(PixmapInfo &outPixmap, AllocateMem allocate,
                                           int &fd, uint64_t &bufferSize, Size &dstSize)
{
    if (allocate == nullptr) {
        outPixmap.data = static_cast<uint8_t *>(malloc(bufferSize));
    } else {
        outPixmap.data = allocate(dstSize, bufferSize, fd);
    }
    if (outPixmap.data == nullptr) {
        IMAGE_LOGE("[BasicTransformer]apply heap memory failed");
        return false;
    }
    return true;
}

void BasicTransformer::ReleaseBuffer(AllocatorType allocatorType, int fd, int dataSize, uint8_t *buffer)
{
#if !defined(_WIN32) && !defined(_APPLE)
    if (allocatorType == AllocatorType::SHARE_MEM_ALLOC) {
        if (buffer != nullptr) {
            ::munmap(buffer, dataSize);
            ::close(fd);
        }
        return;
    }
#endif

    if (allocatorType == AllocatorType::HEAP_ALLOC) {
        if (buffer != nullptr) {
            free(buffer);
        }
        return;
    }
}
uint32_t BasicTransformer::TransformPixmap(const PixmapInfo &inPixmap, PixmapInfo &outPixmap, AllocateMem allocate)
{
    if (inPixmap.data == nullptr) {
        IMAGE_LOGE("[BasicTransformer]input data is null.");
        return ERR_IMAGE_GENERAL_ERROR;
    }
    int32_t pixelBytes = ImageUtils::GetPixelBytes(inPixmap.imageInfo.pixelFormat);
    if (pixelBytes == 0) {
        IMAGE_LOGE("[BasicTransformer]input pixel is invalid.");
        return ERR_IMAGE_INVALID_PIXEL;
    }

    Size dstSize = inPixmap.imageInfo.size;
    GetDstDimension(inPixmap.imageInfo.size, dstSize);
    outPixmap.imageInfo.size = dstSize;
    if (dstSize.width <= 0 || dstSize.height <= 0) {
        IMAGE_LOGE("[BasicTransformer]buffer size is invalid.");
        return ERR_IMAGE_ALLOC_MEMORY_FAILED;
    }

    uint64_t bufferSize = static_cast<uint64_t>(dstSize.width) * dstSize.height * pixelBytes;
    if (bufferSize > PIXEL_MAP_MAX_RAM_SIZE) {
        IMAGE_LOGE("[BasicTransformer] buffer size:%{public}llu out of range.",
                   static_cast<unsigned long long>(bufferSize));
        return ERR_IMAGE_ALLOC_MEMORY_FAILED;
    }
    int fd = 0;
    if (CheckAllocateBuffer(outPixmap, allocate, fd, bufferSize, dstSize) != true) {
        return ERR_IMAGE_ALLOC_MEMORY_FAILED;
    }
    outPixmap.bufferSize = bufferSize;
    outPixmap.imageInfo.pixelFormat = inPixmap.imageInfo.pixelFormat;
    outPixmap.imageInfo.colorSpace = inPixmap.imageInfo.colorSpace;
    outPixmap.imageInfo.alphaType = inPixmap.imageInfo.alphaType;
    outPixmap.imageInfo.baseDensity = inPixmap.imageInfo.baseDensity;

#ifdef _WIN32
    memset(outPixmap.data, COLOR_DEFAULT, bufferSize * sizeof(uint8_t));
#else
    if (memset_s(outPixmap.data, bufferSize * sizeof(uint8_t), COLOR_DEFAULT, bufferSize * sizeof(uint8_t)) != EOK) {
        IMAGE_LOGE("[BasicTransformer]apply heap memory failed.");
        ReleaseBuffer((allocate == nullptr) ? AllocatorType::HEAP_ALLOC : AllocatorType::SHARE_MEM_ALLOC,
            fd, bufferSize, outPixmap.data);
        return ERR_IMAGE_GENERAL_ERROR;
    }
#endif

    if (!DrawPixelmap(inPixmap, pixelBytes, dstSize, outPixmap.data)) {
        IMAGE_LOGE("[BasicTransformer] the matrix can not invert.");
        ReleaseBuffer((allocate == nullptr) ? AllocatorType::HEAP_ALLOC : AllocatorType::SHARE_MEM_ALLOC,
            fd, bufferSize, outPixmap.data);
        return ERR_IMAGE_MATRIX_NOT_INVERT;
    }
    return IMAGE_SUCCESS;
}

bool BasicTransformer::DrawPixelmap(const PixmapInfo &pixmapInfo, const int32_t pixelBytes, const Size &size,
                                    uint8_t *data)
{
    Matrix invertMatrix;
    bool isInvert = matrix_.Invert(invertMatrix);
    if (!isInvert) {
        return false;
    }

    uint32_t rb = pixmapInfo.imageInfo.size.width * pixelBytes;
    Matrix::OperType operType = matrix_.GetOperType();
    Matrix::CalcXYProc fInvProc = Matrix::GetXYProc(operType);

    for (int32_t y = 0; y < size.height; ++y) {
        for (int32_t x = 0; x < size.width; ++x) {
            Point srcPoint;
            // Center coordinate alignment, need to add 0.5, so the boundary can also be considered
            fInvProc(invertMatrix, static_cast<float>(x) + minX_ + FHALF, static_cast<float>(y) + minY_ + FHALF,
                     srcPoint);
            if (CheckOutOfRange(srcPoint, pixmapInfo.imageInfo.size)) {
                continue;
            }
            uint32_t shiftBytes = (y * size.width + x) * pixelBytes;
            BilinearProc(srcPoint, pixmapInfo, rb, shiftBytes, data);
        }
    }

    return true;
}

void BasicTransformer::GetRotateDimension(Matrix::CalcXYProc fInvProc, const Size &srcSize, Size &dstSize)
{
    Point dstP1;
    Point dstP2;
    Point dstP3;
    Point dstP4;

    float fx = static_cast<float>(srcSize.width);
    float fy = static_cast<float>(srcSize.height);
    fInvProc(matrix_, 0.0f, 0.0f, dstP1);
    fInvProc(matrix_, fx, 0.0f, dstP2);
    fInvProc(matrix_, 0.0f, fy, dstP3);
    fInvProc(matrix_, fx, fy, dstP4);

    // For rotation, the width and height will change, so you need to take the maximum of the two diagonals.
    dstSize.width = static_cast<int32_t>(fmaxf(fabsf(dstP4.x - dstP1.x), fabsf(dstP3.x - dstP2.x)) + FHALF);
    dstSize.height = static_cast<int32_t>(fmaxf(fabsf(dstP4.y - dstP1.y), fabsf(dstP3.y - dstP2.y)) + FHALF);

    float min14X = std::min(dstP1.x, dstP4.x);
    float min23X = std::min(dstP2.x, dstP3.x);
    minX_ = std::min(min14X, min23X);

    float min14Y = std::min(dstP1.y, dstP4.y);
    float min23Y = std::min(dstP2.y, dstP3.y);
    minY_ = std::min(min14Y, min23Y);
}

void BasicTransformer::BilinearProc(const Point &pt, const PixmapInfo &pixmapInfo, const uint32_t rb,
                                    const int32_t shiftBytes, uint8_t *data)
{
    uint32_t srcX = (pt.x * MULTI_65536) - HALF_BASIC;
    uint32_t srcY = (pt.y * MULTI_65536) - HALF_BASIC;

    AroundPos aroundPos;
    aroundPos.x0 = RightShift16Bit(srcX, pixmapInfo.imageInfo.size.width - 1);
    aroundPos.x1 = RightShift16Bit(srcX + BASIC, pixmapInfo.imageInfo.size.width - 1);
    uint32_t subx = GetSubValue(srcX);

    aroundPos.y0 = RightShift16Bit(srcY, pixmapInfo.imageInfo.size.height - 1);
    aroundPos.y1 = RightShift16Bit(srcY + BASIC, pixmapInfo.imageInfo.size.height - 1);
    uint32_t suby = GetSubValue(srcY);

    AroundPixels aroundPixels;
    switch (pixmapInfo.imageInfo.pixelFormat) {
        case PixelFormat::RGBA_8888:
        case PixelFormat::ARGB_8888:
        case PixelFormat::BGRA_8888:
            GetAroundPixelRGBA(aroundPos, pixmapInfo.data, rb, aroundPixels);
            break;
        case PixelFormat::RGB_565:
            GetAroundPixelRGB565(aroundPos, pixmapInfo.data, rb, aroundPixels);
            break;
        case PixelFormat::RGB_888:
            GetAroundPixelRGB888(aroundPos, pixmapInfo.data, rb, aroundPixels);
            break;
        case PixelFormat::ALPHA_8:
            GetAroundPixelALPHA8(aroundPos, pixmapInfo.data, rb, aroundPixels);
            break;
        default:
            IMAGE_LOGE("[BasicTransformer] pixel format not supported, format:%d", pixmapInfo.imageInfo.pixelFormat);
            return;
    }

    uint32_t *tmp = reinterpret_cast<uint32_t *>(data + shiftBytes);
    *tmp = FilterProc(subx, suby, aroundPixels);
}

void BasicTransformer::GetAroundPixelRGB565(const AroundPos aroundPos, uint8_t *data, uint32_t rb,
                                            AroundPixels &aroundPixels)
{
    const uint16_t *row0 = reinterpret_cast<uint16_t *>(data + aroundPos.y0 * rb);
    const uint16_t *row1 = reinterpret_cast<uint16_t *>(data + aroundPos.y1 * rb);
    aroundPixels.color00 = row0[aroundPos.x0];
    aroundPixels.color01 = row0[aroundPos.x1];
    aroundPixels.color10 = row1[aroundPos.x0];
    aroundPixels.color11 = row1[aroundPos.x1];
}

void BasicTransformer::GetAroundPixelRGB888(const AroundPos aroundPos, uint8_t *data, uint32_t rb,
                                            AroundPixels &aroundPixels)
{
    const uint8_t *row0 = data + aroundPos.y0 * rb;
    const uint8_t *row1 = data + aroundPos.y1 * rb;
    uint32_t current0 = aroundPos.x0 * RGB888_BYTE;
    uint32_t current1 = aroundPos.x1 * RGB888_BYTE;
    // The RGB888 format occupies 3 bytes, and an int integer is formed by OR operation.
    aroundPixels.color00 =
        (row0[current0] << SHIFT_16_BIT) | (row0[current0 + 1] << SHIFT_8_BIT) | (row0[current0 + 2]);
    aroundPixels.color01 =
        (row0[current1] << SHIFT_16_BIT) | (row0[current1 + 1] << SHIFT_8_BIT) | (row0[current1 + 2]);
    aroundPixels.color10 =
        (row1[current0] << SHIFT_16_BIT) | (row1[current0 + 1] << SHIFT_8_BIT) | (row1[current0 + 2]);
    aroundPixels.color11 =
        (row1[current1] << SHIFT_16_BIT) | (row1[current1 + 1] << SHIFT_8_BIT) | (row1[current1 + 2]);
}

void BasicTransformer::GetAroundPixelRGBA(const AroundPos aroundPos, uint8_t *data,
                                          uint32_t rb, AroundPixels &aroundPixels)
{
    const uint32_t *row0 = reinterpret_cast<uint32_t *>(data + aroundPos.y0 * rb);
    const uint32_t *row1 = reinterpret_cast<uint32_t *>(data + aroundPos.y1 * rb);
    aroundPixels.color00 = row0[aroundPos.x0];
    aroundPixels.color01 = row0[aroundPos.x1];
    aroundPixels.color10 = row1[aroundPos.x0];
    aroundPixels.color11 = row1[aroundPos.x1];
}

void BasicTransformer::GetAroundPixelALPHA8(const AroundPos aroundPos, uint8_t *data, uint32_t rb,
                                            AroundPixels &aroundPixels)
{
    const uint8_t *row0 = data + aroundPos.y0 * rb;
    const uint8_t *row1 = data + aroundPos.y1 * rb;
    aroundPixels.color00 = row0[aroundPos.x0];
    aroundPixels.color01 = row0[aroundPos.x1];
    aroundPixels.color10 = row1[aroundPos.x0];
    aroundPixels.color11 = row1[aroundPos.x1];
}

uint32_t BasicTransformer::RightShift16Bit(uint32_t num, int32_t maxNum)
{
    /*
     * When the original image coordinates are obtained,
     * the first 16 bits are shifted to the left, so the right shift is 16 bits here.
     */
    return ClampMax(num >> 16, maxNum);
}

uint32_t BasicTransformer::FilterProc(const uint32_t subx, const uint32_t suby, const AroundPixels &aroundPixels)
{
    int32_t xy = subx * suby;
    // Mask 0xFF00FF ensures that high and low 16 bits can be calculated simultaneously
    const uint32_t mask = 0xFF00FF;

    /* All values are first magnified 16 times (left shift 4bit) and then divide 256 (right shift 8bit).
     * Reference formula f(i+u,j+v) = (1-u)(1-v)f(i,j) + (1-u)vf(i,j+1) + u(1-v)f(i+1,j) + uvf(i+1,j+1),
     * The subx is u, the suby is y,
     * color00 is f(i,j), color 01 is f(i,j+1), color 10 is f(i+1,j), color11 is f(i+1,j+1).
     */
    int32_t scale = 256 - 16 * suby - 16 * subx + xy;
    uint32_t lo = (aroundPixels.color00 & mask) * scale;
    uint32_t hi = ((aroundPixels.color00 >> 8) & mask) * scale;

    scale = 16 * subx - xy;
    lo += (aroundPixels.color01 & mask) * scale;
    hi += ((aroundPixels.color01 >> 8) & mask) * scale;

    scale = 16 * suby - xy;
    lo += (aroundPixels.color10 & mask) * scale;
    hi += ((aroundPixels.color10 >> 8) & mask) * scale;

    lo += (aroundPixels.color11 & mask) * xy;
    hi += ((aroundPixels.color11 >> 8) & mask) * xy;

    return ((lo >> 8) & mask) | (hi & ~mask);
}
} // namespace Media
} // namespace OHOS