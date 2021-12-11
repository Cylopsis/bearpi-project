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

#include "webp_decoder.h"
#include "media_errors.h"
#include "multimedia_templates.h"
#include "securec.h"

namespace OHOS {
namespace ImagePlugin {
using namespace OHOS::HiviewDFX;
using namespace MultimediaPlugin;
using namespace Media;
using namespace MultiMedia;

namespace {
constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "WebpDecoder" };
constexpr int32_t WEBP_IMAGE_NUM = 1;
constexpr int32_t EXTERNAL_MEMORY = 1;
constexpr size_t DECODE_VP8CHUNK_MIN_SIZE = 4096;
} // namespace

WebpDecoder::WebpDecoder()
{}

WebpDecoder::~WebpDecoder()
{
    Reset();
}

void WebpDecoder::SetSource(InputDataStream &sourceStream)
{
    stream_ = &sourceStream;
    state_ = WebpDecodingState::SOURCE_INITED;
}

uint32_t WebpDecoder::GetImageSize(uint32_t index, PlSize &size)
{
    if (index >= WEBP_IMAGE_NUM) {
        HiLog::Error(LABEL, "image size:invalid index, index:%{public}u, range:%{public}u.", index, WEBP_IMAGE_NUM);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    if (state_ < WebpDecodingState::SOURCE_INITED) {
        HiLog::Error(LABEL, "get image size failed for state %{public}d.", state_);
        return ERR_MEDIA_INVALID_OPERATION;
    }
    if (state_ >= WebpDecodingState::BASE_INFO_PARSED) {
        size = webpSize_;
        return SUCCESS;
    }

    uint32_t ret = DecodeHeader();
    if (ret != SUCCESS) {
        HiLog::Debug(LABEL, "decode header error on get image ret:%{public}u.", ret);
        return ret;
    }
    size = webpSize_;
    return SUCCESS;
}

uint32_t WebpDecoder::SetDecodeOptions(uint32_t index, const PixelDecodeOptions &opts, PlImageInfo &info)
{
    if (index >= WEBP_IMAGE_NUM) {
        HiLog::Error(LABEL, "set option:invalid index, index:%{public}u, range:%{public}u.", index, WEBP_IMAGE_NUM);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    if (state_ < WebpDecodingState::SOURCE_INITED) {
        HiLog::Error(LABEL, "set decode option failed for state %{public}d.", state_);
        return ERR_MEDIA_INVALID_OPERATION;
    }
    if (state_ >= WebpDecodingState::IMAGE_DECODING) {
        FinishOldDecompress();
        state_ = WebpDecodingState::SOURCE_INITED;
    }
    if (state_ < WebpDecodingState::BASE_INFO_PARSED) {
        uint32_t ret = DecodeHeader();
        if (ret != SUCCESS) {
            HiLog::Error(LABEL, "decode header error on set decode options:%{public}u.", ret);
            state_ = WebpDecodingState::BASE_INFO_PARSING;
            return ret;
        }
        state_ = WebpDecodingState::BASE_INFO_PARSED;
    }

    bool hasAlpha = true;
    if (opts.desiredPixelFormat == PlPixelFormat::RGB_565) {
        hasAlpha = false;
        info.alphaType = PlAlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    } else {
        info.alphaType = opts.desireAlphaType;
    }
    webpMode_ = GetWebpDecodeMode(opts.desiredPixelFormat,
                                  hasAlpha && (opts.desireAlphaType == PlAlphaType::IMAGE_ALPHA_TYPE_PREMUL));
    info.size = webpSize_;
    info.pixelFormat = outputFormat_;
    opts_ = opts;

    state_ = WebpDecodingState::IMAGE_DECODING;
    return SUCCESS;
}

uint32_t WebpDecoder::Decode(uint32_t index, DecodeContext &context)
{
    if (index >= WEBP_IMAGE_NUM) {
        HiLog::Error(LABEL, "decode:invalid index, index:%{public}u, range:%{public}u.", index, WEBP_IMAGE_NUM);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    if (state_ < WebpDecodingState::IMAGE_DECODING) {
        HiLog::Error(LABEL, "set decode option failed for state %{public}d.", state_);
        return ERR_MEDIA_INVALID_OPERATION;
    }
    if (state_ > WebpDecodingState::IMAGE_DECODING) {
        FinishOldDecompress();
        uint32_t ret = DecodeHeader();
        if (ret != SUCCESS) {
            HiLog::Error(LABEL, "decode header error on set decode options:%{public}u.", ret);
            state_ = WebpDecodingState::BASE_INFO_PARSING;
            return ret;
        }
        bool hasAlpha = true;
        if (opts_.desiredPixelFormat == PlPixelFormat::RGB_565) {
            hasAlpha = false;
        }
        webpMode_ =
            GetWebpDecodeMode(opts_.desiredPixelFormat,
                              hasAlpha && opts_.desireAlphaType == PlAlphaType::IMAGE_ALPHA_TYPE_PREMUL);
        state_ = WebpDecodingState::IMAGE_DECODING;
    }

    return DoCommonDecode(context);
}

uint32_t WebpDecoder::PromoteIncrementalDecode(uint32_t index, ProgDecodeContext &context)
{
    context.totalProcessProgress = 0;
    if (index >= WEBP_IMAGE_NUM) {
        HiLog::Error(LABEL, "incremental:invalid index, index:%{public}u, range:%{public}u.", index, WEBP_IMAGE_NUM);
        return ERR_IMAGE_INVALID_PARAMETER;
    }

    if (state_ != WebpDecodingState::IMAGE_DECODING) {
        HiLog::Error(LABEL, "incremental decode failed for state %{public}d.", state_);
        return ERR_MEDIA_INVALID_OPERATION;
    }

    if (!IsDataEnough()) {
        HiLog::Debug(LABEL, "increment data not enough, need next data.");
        return ERR_IMAGE_SOURCE_DATA_INCOMPLETE;
    }
    return DoIncrementalDecode(context);
}

uint32_t WebpDecoder::DecodeHeader()
{
    uint32_t ret = ReadIncrementalHead();
    if (ret != SUCCESS) {
        if (ret == ERR_IMAGE_SOURCE_DATA_INCOMPLETE) {
            state_ = WebpDecodingState::BASE_INFO_PARSING;
        } else {
            state_ = WebpDecodingState::SOURCE_INITED;
            HiLog::Error(LABEL, "decode image head, ret:%{public}u.", ret);
        }
        return ret;
    }
    state_ = WebpDecodingState::BASE_INFO_PARSED;
    return SUCCESS;
}

uint32_t WebpDecoder::ReadIncrementalHead()
{
    size_t stremSize = stream_->GetStreamSize();
    if (stremSize >= DECODE_VP8CHUNK_MIN_SIZE || stream_->IsStreamCompleted()) {
        stream_->Seek(0);
        if (!stream_->Read(stream_->GetStreamSize(), dataBuffer_)) {
            HiLog::Error(LABEL, "read data fail.");
            return ERR_IMAGE_SOURCE_DATA_INCOMPLETE;
        }
        if (dataBuffer_.inputStreamBuffer == nullptr || dataBuffer_.dataSize == 0) {
            HiLog::Error(LABEL, "inputStreamBuffer is null or data size is %{public}u.", dataBuffer_.dataSize);
            return ERR_IMAGE_GET_DATA_ABNORMAL;
        }

        int32_t width = 0;
        int32_t height = 0;
        int32_t ret = WebPGetInfo(dataBuffer_.inputStreamBuffer, dataBuffer_.bufferSize, &width, &height);
        if (ret == 0 || (width == 0 && height == 0)) {
            // may be incomplete data
            HiLog::Error(LABEL, "get width and height fail.");
            return ERR_IMAGE_SOURCE_DATA_INCOMPLETE;
        }

        if (width < 0 || height < 0) {
            HiLog::Error(LABEL, "width and height invalid, width:%{public}d, height:%{public}d.", width, height);
            return ERR_IMAGE_INVALID_PARAMETER;
        }
        webpSize_.width = static_cast<uint32_t>(width);
        webpSize_.height = static_cast<uint32_t>(height);
        incrementSize_ = stremSize;
        lastDecodeSize_ = stremSize;
        return SUCCESS;
    }

    return ERR_IMAGE_SOURCE_DATA_INCOMPLETE;
}

bool WebpDecoder::IsDataEnough()
{
    size_t streamSize = stream_->GetStreamSize();
    if (incrementSize_ < DECODE_VP8CHUNK_MIN_SIZE && !stream_->IsStreamCompleted()) {
        incrementSize_ += streamSize - lastDecodeSize_;
        lastDecodeSize_ = streamSize;
        return false;
    }
    incrementSize_ = streamSize - lastDecodeSize_;
    lastDecodeSize_ = streamSize;
    return true;
}

WEBP_CSP_MODE WebpDecoder::GetWebpDecodeMode(const PlPixelFormat &pixelFormat, bool premul)
{
    WEBP_CSP_MODE webpMode = MODE_RGBA;
    outputFormat_ = pixelFormat;
    switch (pixelFormat) {
        case PlPixelFormat::BGRA_8888:
            webpMode = premul ? MODE_bgrA : MODE_BGRA;
            break;
        case PlPixelFormat::RGBA_8888:
            webpMode = premul ? MODE_rgbA : MODE_RGBA;
            break;
        case PlPixelFormat::RGB_565:
            bytesPerPixel_ = 2;  // RGB_565 2 bytes each pixel
            webpMode = MODE_RGB_565;
            break;
        case PlPixelFormat::UNKNOWN:
        default:
            outputFormat_ = PlPixelFormat::RGBA_8888;
            webpMode = premul ? MODE_rgbA : MODE_RGBA;
            break;
    }
    return webpMode;
}

void WebpDecoder::FinishOldDecompress()
{
    if (state_ < WebpDecodingState::IMAGE_DECODING) {
        return;
    }
    Reset();
}

uint32_t WebpDecoder::DoCommonDecode(DecodeContext &context)
{
    WebPDecoderConfig config;
    if (!PreDecodeProc(context, config, false)) {
        HiLog::Error(LABEL, "prepare common decode failed.");
        state_ = WebpDecodingState::IMAGE_ERROR;
        return ERR_IMAGE_MALLOC_ABNORMAL;
    }

    TAutoCallProc<WebPDecBuffer, WebPFreeDecBuffer> webpOutput(&config.output);
    TAutoCallProc<WebPIDecoder, WebPIDelete> idec(WebPINewDecoder(&config.output));
    if (idec == nullptr) {
        HiLog::Error(LABEL, "common decode:idec is null.");
        state_ = WebpDecodingState::IMAGE_ERROR;
        return ERR_IMAGE_DECODE_FAILED;
    }

    VP8StatusCode status = WebPIUpdate(idec, dataBuffer_.inputStreamBuffer, static_cast<size_t>(dataBuffer_.dataSize));
    if (status == VP8_STATUS_OK) {
        state_ = WebpDecodingState::IMAGE_DECODED;
        return SUCCESS;
    }
    if (status == VP8_STATUS_SUSPENDED && opts_.allowPartialImage) {
        state_ = WebpDecodingState::IMAGE_PARTIAL;
        context.ifPartialOutput = true;
        HiLog::Error(LABEL, "this is partial image data to decode.");
        return SUCCESS;
    }

    HiLog::Error(LABEL, "decode image data failed, status:%{public}d.", status);
    state_ = WebpDecodingState::IMAGE_ERROR;
    return ERR_IMAGE_DECODE_FAILED;
}

uint32_t WebpDecoder::DoIncrementalDecode(ProgDecodeContext &context)
{
    WebPDecoderConfig config;
    if (!PreDecodeProc(context.decodeContext, config, true)) {
        HiLog::Error(LABEL, "prepare increment decode failed.");
        return ERR_IMAGE_MALLOC_ABNORMAL;
    }

    TAutoCallProc<WebPDecBuffer, WebPFreeDecBuffer> webpOutput(&config.output);
    TAutoCallProc<WebPIDecoder, WebPIDelete> idec(WebPINewDecoder(&config.output));
    if (idec == nullptr) {
        HiLog::Error(LABEL, "incremental code:idec is null.");
        return ERR_IMAGE_DECODE_FAILED;
    }

    dataBuffer_ = { nullptr, 0, 0 };
    stream_->Seek(0);
    if (!stream_->Read(stream_->GetStreamSize(), dataBuffer_)) {
        HiLog::Error(LABEL, "incremental:read data failed.");
        return ERR_IMAGE_DECODE_FAILED;
    }
    if (dataBuffer_.inputStreamBuffer == nullptr || dataBuffer_.dataSize == 0) {
        HiLog::Error(LABEL, "incremental:data is null.");
        return ERR_IMAGE_DECODE_FAILED;
    }

    VP8StatusCode status = WebPIUpdate(idec, dataBuffer_.inputStreamBuffer, static_cast<size_t>(dataBuffer_.dataSize));
    if (status != VP8_STATUS_OK && status != VP8_STATUS_SUSPENDED) {
        HiLog::Error(LABEL, "incremental:webp status exception,status:%{public}d.", status);
        return ERR_IMAGE_DECODE_FAILED;
    }
    if (status == VP8_STATUS_SUSPENDED) {
        int32_t curHeight = 0;
        if (WebPIDecGetRGB(idec, &curHeight, nullptr, nullptr, nullptr) == nullptr) {
            HiLog::Debug(LABEL, "refresh image failed, current height:%{public}d.", curHeight);
        }
        if (curHeight > 0 && webpSize_.height != 0) {
            context.totalProcessProgress =
                static_cast<uint32_t>(curHeight) * ProgDecodeContext::FULL_PROGRESS / webpSize_.height;
        }
        return ERR_IMAGE_SOURCE_DATA_INCOMPLETE;
    }
    if (status == VP8_STATUS_OK) {
        context.totalProcessProgress = context.FULL_PROGRESS;
        state_ = WebpDecodingState::IMAGE_DECODED;
    }
    return SUCCESS;
}

void WebpDecoder::InitWebpOutput(const DecodeContext &context, WebPDecBuffer &output)
{
    output.is_external_memory = EXTERNAL_MEMORY;  // external allocated space
    output.u.RGBA.rgba = static_cast<uint8_t *>(context.pixelsBuffer.buffer);
    output.u.RGBA.stride = webpSize_.width * bytesPerPixel_;
    output.u.RGBA.size = context.pixelsBuffer.bufferSize;
    output.colorspace = webpMode_;
}

bool WebpDecoder::PreDecodeProc(DecodeContext &context, WebPDecoderConfig &config, bool isIncremental)
{
    if (WebPInitDecoderConfig(&config) == 0) {
        HiLog::Error(LABEL, "init config failed.");
        return false;
    }
    if (!AllocHeapBuffer(context, isIncremental)) {
        HiLog::Error(LABEL, "get pixels memory failed.");
        return false;
    }

    InitWebpOutput(context, config.output);
    return true;
}

void WebpDecoder::Reset()
{
    stream_->Seek(0);
    dataBuffer_ = { nullptr, 0, 0 };
    webpSize_ = { 0, 0 };
}

bool WebpDecoder::AllocHeapBuffer(DecodeContext &context, bool isIncremental)
{
    if (isIncremental) {
        if (context.pixelsBuffer.buffer != nullptr && context.allocatorType == AllocatorType::HEAP_ALLOC) {
            free(context.pixelsBuffer.buffer);
            context.pixelsBuffer.buffer = nullptr;
        }
    }

    if (context.pixelsBuffer.buffer == nullptr) {
        uint64_t byteCount = static_cast<uint64_t>(webpSize_.width) * webpSize_.height * bytesPerPixel_;
        if (context.allocatorType == Media::AllocatorType::SHARE_MEM_ALLOC) {
#ifndef _WIN32
            int fd = AshmemCreate("WEBP RawData", byteCount);
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
                HiLog::Error(LABEL, "malloc fdBuffer fail");
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
#endif
        } else {
            void *outputBuffer = malloc(byteCount);
            if (outputBuffer == nullptr) {
                HiLog::Error(LABEL, "alloc output buffer size:[%{public}llu] error.",
                             static_cast<unsigned long long>(byteCount));
                return false;
            }
#ifdef _WIN32
            memset(outputBuffer, 0, byteCount);
#else
            if (memset_s(outputBuffer, byteCount, 0, byteCount) != EOK) {
                HiLog::Error(LABEL, "memset buffer failed.");
                free(outputBuffer);
                outputBuffer = nullptr;
                return false;
            }
#endif
            context.pixelsBuffer.buffer = outputBuffer;
            context.pixelsBuffer.bufferSize = byteCount;
            context.pixelsBuffer.context = nullptr;
            context.allocatorType = AllocatorType::HEAP_ALLOC;
            context.freeFunc = nullptr;
        }
    }
    return true;
}
} // namespace ImagePlugin
} // namespace OHOS
