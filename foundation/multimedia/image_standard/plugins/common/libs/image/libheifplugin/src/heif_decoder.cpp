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

#include "heif_decoder.h"
#include "media_errors.h"
#include "securec.h"

namespace OHOS {
namespace ImagePlugin {
using namespace OHOS::HiviewDFX;
using namespace MultimediaPlugin;
using namespace Media;

constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "HeifDecoder" };
constexpr uint32_t HEIF_IMAGE_NUM = 1;

void HeifDecoder::SetSource(InputDataStream &sourceStream)
{
    heifDecoderInterface_ = HeifDecoderWrapper::CreateHeifDecoderInterface(sourceStream);
}

void HeifDecoder::Reset()
{
    heifDecoderInterface_ = nullptr;
    heifSize_.width = 0;
    heifSize_.height = 0;
    bytesPerPixel_ = 0;
}

uint32_t HeifDecoder::SetDecodeOptions(uint32_t index, const PixelDecodeOptions &opts, PlImageInfo &info)
{
    uint32_t ret = GetImageSize(index, info.size);
    if (ret != SUCCESS) {
        HiLog::Error(LABEL, "get image size failed, ret=%{public}u", ret);
        return ret;
    }
    heifSize_ = info.size;

    if (heifDecoderInterface_->ConversionSupported(opts.desiredPixelFormat, bytesPerPixel_)) {
        info.pixelFormat = opts.desiredPixelFormat;
        if (info.pixelFormat == PlPixelFormat::UNKNOWN) {
            info.pixelFormat = PlPixelFormat::BGRA_8888;
        }
    } else {
        return ERR_IMAGE_COLOR_CONVERT;
    }
    heifDecoderInterface_->SetAllowPartial(opts.allowPartialImage);
    bool hasAlpha = (info.pixelFormat == PlPixelFormat::RGB_565 || info.pixelFormat == PlPixelFormat::RGB_888 ||
                     info.pixelFormat == PlPixelFormat::ALPHA_8);
    if (hasAlpha) {
        info.alphaType = PlAlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    } else {
        info.alphaType = PlAlphaType::IMAGE_ALPHA_TYPE_UNPREMUL;
    }
    return SUCCESS;
}
uint32_t HeifDecoder::Decode(uint32_t index, DecodeContext &context)
{
    if (heifDecoderInterface_ == nullptr) {
        HiLog::Error(LABEL, "create heif interface object failed!");
        return ERR_IMAGE_INIT_ABNORMAL;
    }

    if (index >= HEIF_IMAGE_NUM) {
        HiLog::Error(LABEL, "decode image out of range, index:%{public}u, range:%{public}d.", index, HEIF_IMAGE_NUM);
        return ERR_IMAGE_INVALID_PARAMETER;
    }

    if (!AllocHeapBuffer(context)) {
        HiLog::Error(LABEL, "get pixels memory fail.");
        return ERR_IMAGE_MALLOC_ABNORMAL;
    }
    return heifDecoderInterface_->OnGetPixels(heifSize_, heifSize_.width * bytesPerPixel_, context);
}

uint32_t HeifDecoder::GetImageSize(uint32_t index, PlSize &size)
{
    if (index >= HEIF_IMAGE_NUM) {
        HiLog::Error(LABEL, "decode image out of range, index:%{public}u, range:%{public}d.", index, HEIF_IMAGE_NUM);
        return ERR_IMAGE_INVALID_PARAMETER;
    }

    if (heifDecoderInterface_ == nullptr) {
        HiLog::Error(LABEL, "create heif interface object failed!");
        return ERR_IMAGE_INIT_ABNORMAL;
    }

    heifDecoderInterface_->GetHeifSize(size);
    if (size.width == 0 || size.height == 0) {
        HiLog::Error(LABEL, "get width and height fail, height:%{public}u, width:%{public}u.", size.height,
                     size.height);
        return ERR_IMAGE_GET_DATA_ABNORMAL;
    }
    return SUCCESS;
}

uint32_t HeifDecoder::PromoteIncrementalDecode(uint32_t index, ProgDecodeContext &context)
{
    // currently not support increment decode
    return ERR_IMAGE_DATA_UNSUPPORT;
}
uint32_t HeifDecoder::GetTopLevelImageNum(uint32_t &num)
{
    // currently only supports single frame
    num = HEIF_IMAGE_NUM;
    return SUCCESS;
}

bool HeifDecoder::AllocHeapBuffer(DecodeContext &context)
{
    if (context.pixelsBuffer.buffer == nullptr) {
        if (!IsHeifImageParaValid(heifSize_, bytesPerPixel_)) {
            HiLog::Error(LABEL, "check heif image para fail");
            return false;
        }
        uint64_t byteCount = static_cast<uint64_t>(heifSize_.width) * heifSize_.height * bytesPerPixel_;
        if (context.allocatorType == Media::AllocatorType::SHARE_MEM_ALLOC) {
            int fd = AshmemCreate("HEIF RawData", byteCount);
            if (fd < 0) {
                return false;
            }
            int result = AshmemSetProt(fd, PROT_READ | PROT_WRITE);
            if (result < 0) {
                ::close(fd);
                return false;
            }
            void* ptr = ::mmap(nullptr, byteCount, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (ptr == MAP_FAILED) {
                ::close(fd);
                return false;
            }
            context.pixelsBuffer.buffer = ptr;
            void *fdBuffer = new int32_t();
            if (fdBuffer == nullptr) {
                HiLog::Error(LABEL, "new fdBuffer fail");
                ::munmap(ptr, byteCount);
                ::close(fd);
                context.pixelsBuffer.buffer = nullptr;
                return false;
            }
            *static_cast<int32_t *>(fdBuffer) = fd;
            context.pixelsBuffer.context = fdBuffer;
            context.pixelsBuffer.bufferSize = byteCount;
            context.allocatorType = AllocatorType::SHARE_MEM_ALLOC;
            context.freeFunc = nullptr;
        } else {
            void *outputBuffer = malloc(byteCount);
            if (outputBuffer == nullptr) {
                HiLog::Error(LABEL, "alloc output buffer size:[%{public}llu] error.",
                             static_cast<unsigned long long>(byteCount));
                return false;
            }
            if (memset_s(outputBuffer, byteCount, 0, byteCount) != EOK) {
                HiLog::Error(LABEL, "memset buffer failed.");
                free(outputBuffer);
                outputBuffer = nullptr;
                return false;
            }
            context.pixelsBuffer.buffer = outputBuffer;
            context.pixelsBuffer.bufferSize = byteCount;
            context.pixelsBuffer.context = nullptr;
            context.allocatorType = AllocatorType::HEAP_ALLOC;
            context.freeFunc = nullptr;
        }
    }
    return true;
}

bool HeifDecoder::IsHeifImageParaValid(PlSize heifSize, uint32_t bytesPerPixel)
{
    if (heifSize.width == 0 || heifSize.height == 0 || bytesPerPixel == 0) {
        HiLog::Error(LABEL, "heif image para is 0");
        return false;
    }
    uint64_t area = static_cast<uint64_t>(heifSize.width) * heifSize.height;
    if ((area / heifSize.width) != heifSize.height) {
        HiLog::Error(LABEL, "compute width*height overflow!");
        return false;
    }
    uint64_t size = area * bytesPerPixel;
    if ((size / bytesPerPixel) != area) {
        HiLog::Error(LABEL, "compute area*bytesPerPixel overflow!");
        return false;
    }
    if (size > UINT32_MAX) {
        HiLog::Error(LABEL, "size is too large!");
        return false;
    }
    return true;
}
} // namespace ImagePlugin
} // namespace OHOS