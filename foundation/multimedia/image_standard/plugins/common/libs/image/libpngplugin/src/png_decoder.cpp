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

#include "png_decoder.h"
#include "media_errors.h"
#include "pngpriv.h"
#include "pngstruct.h"
#ifndef _WIN32
#include "securec.h"
#else
#include "memory.h"
#endif

namespace OHOS {
namespace ImagePlugin {
using namespace OHOS::HiviewDFX;
using namespace MultimediaPlugin;
using namespace Media;
static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "PngDecoder" };
static constexpr uint32_t PNG_IMAGE_NUM = 1;
static constexpr int SET_JUMP_VALUE = 1;
static constexpr int BITDEPTH_VALUE_1 = 1;
static constexpr int BITDEPTH_VALUE_2 = 2;
static constexpr int BITDEPTH_VALUE_4 = 4;
static constexpr int BITDEPTH_VALUE_8 = 8;
static constexpr int BITDEPTH_VALUE_16 = 16;
static constexpr size_t DECODE_BUFFER_SIZE = 4096;
static constexpr size_t CHUNK_SIZE = 8;
static constexpr size_t CHUNK_DATA_LEN = 4;
static constexpr int PNG_HEAD_SIZE = 100;

PngDecoder::PngDecoder()
{
    if (!InitPnglib()) {
        HiLog::Error(LABEL, "Png decoder init failed!");
    }
}

PngDecoder::~PngDecoder()
{
    Reset();
    // destroy the png decode struct
    if (pngStructPtr_) {
        png_infopp pngInfoPtr = pngInfoPtr_ ? &pngInfoPtr_ : nullptr;
        png_destroy_read_struct(&pngStructPtr_, pngInfoPtr, nullptr);
    }
}

void PngDecoder::SetSource(InputDataStream &sourceStream)
{
    inputStreamPtr_ = &sourceStream;
    state_ = PngDecodingState::SOURCE_INITED;
}

uint32_t PngDecoder::GetImageSize(uint32_t index, PlSize &size)
{
    // PNG format only supports one picture decoding, index in order to Compatible animation scene.
    if (index >= PNG_IMAGE_NUM) {
        HiLog::Error(LABEL, "decode image out of range, index:%{public}u, range:%{public}u.", index, PNG_IMAGE_NUM);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    if (pngStructPtr_ == nullptr || pngInfoPtr_ == nullptr) {
        HiLog::Error(LABEL, "create Png Struct or Png Info failed!");
        return ERR_IMAGE_INIT_ABNORMAL;
    }
    if (state_ < PngDecodingState::SOURCE_INITED) {
        HiLog::Error(LABEL, "get image size failed for state %{public}d.", state_);
        return ERR_MEDIA_INVALID_OPERATION;
    }
    if (state_ >= PngDecodingState::BASE_INFO_PARSED) {
        size.width = png_get_image_width(pngStructPtr_, pngInfoPtr_);
        size.height = png_get_image_height(pngStructPtr_, pngInfoPtr_);
        return SUCCESS;
    }
    // only state PngDecodingState::SOURCE_INITED and PngDecodingState::BASE_INFO_PARSING can go here.
    uint32_t ret = DecodeHeader();
    if (ret != SUCCESS) {
        HiLog::Debug(LABEL, "decode header error on get image ret:%{public}u.", ret);
        return ret;
    }
    size.width = png_get_image_width(pngStructPtr_, pngInfoPtr_);
    size.height = png_get_image_height(pngStructPtr_, pngInfoPtr_);
    return SUCCESS;
}

uint32_t PngDecoder::SetDecodeOptions(uint32_t index, const PixelDecodeOptions &opts, PlImageInfo &info)
{
    // PNG format only supports one picture decoding, index in order to Compatible animation scene.
    if (index >= PNG_IMAGE_NUM) {
        HiLog::Error(LABEL, "decode image out of range, index:%{public}u, range:%{public}u.", index, PNG_IMAGE_NUM);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    if (pngStructPtr_ == nullptr || pngInfoPtr_ == nullptr) {
        HiLog::Error(LABEL, "Png init fail, can't set decode option.");
        return ERR_IMAGE_INIT_ABNORMAL;
    }
    if (state_ < PngDecodingState::SOURCE_INITED) {
        HiLog::Error(LABEL, "set decode options failed for state %{public}d.", state_);
        return ERR_MEDIA_INVALID_OPERATION;
    }
    if (state_ >= PngDecodingState::IMAGE_DECODING) {
        if (!FinishOldDecompress()) {
            HiLog::Error(LABEL, "finish old decompress fail, can't set decode option.");
            return ERR_IMAGE_INIT_ABNORMAL;
        }
    }
    if (state_ < PngDecodingState::BASE_INFO_PARSED) {
        uint32_t ret = DecodeHeader();
        if (ret != SUCCESS) {
            HiLog::Error(LABEL, "decode header error on set decode options:%{public}u.", ret);
            return ret;
        }
    }

    DealNinePatch(opts);
    // only state PngDecodingState::BASE_INFO_PARSED can go here.
    uint32_t ret = ConfigInfo(opts);
    if (ret != SUCCESS) {
        HiLog::Error(LABEL, "config decoding failed on set decode options:%{public}u.", ret);
        return ret;
    }
    info.size.width = pngImageInfo_.width;
    info.size.height = pngImageInfo_.height;
    info.pixelFormat = outputFormat_;
    info.alphaType = alphaType_;
    opts_ = opts;
    state_ = PngDecodingState::IMAGE_DECODING;
    return SUCCESS;
}

bool PngDecoder::HasProperty(std::string key)
{
    if (NINE_PATCH == key) {
        return static_cast<void *>(ninePatch_.patch_) != nullptr && ninePatch_.patchSize_ != 0;
    }
    return false;
}

uint32_t PngDecoder::Decode(uint32_t index, DecodeContext &context)
{
    // PNG format only supports one picture decoding, index in order to Compatible animation scene.
    if (index >= PNG_IMAGE_NUM) {
        HiLog::Error(LABEL, "decode image out of range, index:%{public}u, range:%{public}u.", index, PNG_IMAGE_NUM);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    if (pngStructPtr_ == nullptr || pngInfoPtr_ == nullptr) {
        HiLog::Error(LABEL, "Png init failed can't begin to decode.");
        return ERR_IMAGE_INIT_ABNORMAL;
    }
    if (state_ < PngDecodingState::IMAGE_DECODING) {
        HiLog::Error(LABEL, "decode failed for state %{public}d.", state_);
        return ERR_MEDIA_INVALID_OPERATION;
    }
    if (state_ > PngDecodingState::IMAGE_DECODING) {
        if (!FinishOldDecompress()) {
            HiLog::Error(LABEL, "finish old decompress fail on decode.");
            return ERR_IMAGE_INIT_ABNORMAL;
        }
        uint32_t ret = DecodeHeader();
        if (ret != SUCCESS) {
            HiLog::Error(LABEL, "decode header error on decode:%{public}u.", ret);
            return ret;
        }
        ret = ConfigInfo(opts_);
        if (ret != SUCCESS) {
            HiLog::Error(LABEL, "config decoding info failed on decode:%{public}u.", ret);
            return ret;
        }
        state_ = PngDecodingState::IMAGE_DECODING;
    }
    // only state PngDecodingState::IMAGE_DECODING can go here.
    context.ninePatchContext.ninePatch = static_cast<void *>(ninePatch_.patch_);
    context.ninePatchContext.patchSize = ninePatch_.patchSize_;
    uint32_t ret = DoOneTimeDecode(context);
    if (ret == SUCCESS) {
        state_ = PngDecodingState::IMAGE_DECODED;
        return SUCCESS;
    }
    if (ret == ERR_IMAGE_SOURCE_DATA_INCOMPLETE && opts_.allowPartialImage) {
        state_ = PngDecodingState::IMAGE_PARTIAL;
        context.ifPartialOutput = true;
        HiLog::Error(LABEL, "this is partial image data to decode, ret:%{public}u.", ret);
        return SUCCESS;
    }
    state_ = PngDecodingState::IMAGE_ERROR;
    return ret;
}

uint8_t *PngDecoder::AllocOutputHeapBuffer(DecodeContext &context)
{
    if (context.pixelsBuffer.buffer == nullptr) {
        uint64_t byteCount = static_cast<uint64_t>(pngImageInfo_.rowDataSize) * pngImageInfo_.height;
        if (context.allocatorType == Media::AllocatorType::SHARE_MEM_ALLOC) {
#if !defined(_WIN32) && !defined(_APPLE)
            int fd = AshmemCreate("PNG RawData", byteCount);
            if (fd < 0) {
                return nullptr;
            }
            int result = AshmemSetProt(fd, PROT_READ | PROT_WRITE);
            if (result < 0) {
                ::close(fd);
                return nullptr;
            }
            void* ptr = ::mmap(nullptr, byteCount, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
            if (ptr == MAP_FAILED) {
                ::close(fd);
                return nullptr;
            }
            context.pixelsBuffer.buffer = ptr;
            void *fdBuffer = new int32_t();
            if (fdBuffer == nullptr) {
                HiLog::Error(LABEL, "new fdBuffer fail");
                ::munmap(ptr, byteCount);
                ::close(fd);
                context.pixelsBuffer.buffer = nullptr;
                return nullptr;
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
                return nullptr;
            }
#ifdef _WIN32
            memset(outputBuffer, 0, byteCount);
#else
            if (memset_s(outputBuffer, byteCount, 0, byteCount) != EOK) {
                HiLog::Error(LABEL, "init output buffer fail.");
                free(outputBuffer);
                outputBuffer = nullptr;
                return nullptr;
            }
#endif
            context.pixelsBuffer.buffer = outputBuffer;
            context.pixelsBuffer.bufferSize = byteCount;
            context.pixelsBuffer.context = nullptr;
            context.allocatorType = AllocatorType::HEAP_ALLOC;
            context.freeFunc = nullptr;
        }
    }
    return static_cast<uint8_t *>(context.pixelsBuffer.buffer);
}

uint32_t PngDecoder::PromoteIncrementalDecode(uint32_t index, ProgDecodeContext &context)
{
    // PNG format only supports one picture decoding, index in order to Compatible animation scene.
    context.totalProcessProgress = 0;
    if (index >= PNG_IMAGE_NUM) {
        HiLog::Error(LABEL, "decode image out of range, index:%{public}u, range:%{public}u.", index, PNG_IMAGE_NUM);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    if (pngStructPtr_ == nullptr || pngInfoPtr_ == nullptr) {
        HiLog::Error(LABEL, "Png init failed can't begin to decode.");
        return ERR_IMAGE_INIT_ABNORMAL;
    }
    if (state_ != PngDecodingState::IMAGE_DECODING) {
        HiLog::Error(LABEL, "incremental decode failed for state %{public}d.", state_);
        return ERR_MEDIA_INVALID_OPERATION;
    }

    pixelsData_ = AllocOutputHeapBuffer(context.decodeContext);
    if (pixelsData_ == nullptr) {
        HiLog::Error(LABEL, "get pixels memory fail.");
        return ERR_IMAGE_MALLOC_ABNORMAL;
    }
    inputStreamPtr_->Seek(streamPosition_);
    uint32_t ret = IncrementalReadRows(inputStreamPtr_);
    streamPosition_ = inputStreamPtr_->Tell();
    if (ret != SUCCESS) {
        if (ret != ERR_IMAGE_SOURCE_DATA_INCOMPLETE) {
            HiLog::Error(LABEL, "Incremental decode fail, ret:%{public}u", ret);
        }
    } else {
        if (outputRowsNum_ != pngImageInfo_.height) {
            HiLog::Debug(LABEL, "Incremental decode incomplete, outputRowsNum:%{public}u, height:%{public}u",
                         outputRowsNum_, pngImageInfo_.height);
        }
        state_ = PngDecodingState::IMAGE_DECODED;
    }
    // get promote decode progress, in percentage: 0~100.
    // DecodeHeader() has judged that pngImageInfo_.height should not be equal to 0 and returns a failure result,
    // so here pngImageInfo_.height will not be equal to 0 in the PngDecodingState::IMAGE_DECODING state.
    context.totalProcessProgress =
        outputRowsNum_ == 0 ? 0 : outputRowsNum_ * ProgDecodeContext::FULL_PROGRESS / pngImageInfo_.height;
    HiLog::Debug(LABEL, "Incremental decode progress %{public}u.", context.totalProcessProgress);
    return ret;
}

void PngDecoder::Reset()
{
    inputStreamPtr_ = nullptr;
    decodedIdat_ = false;
    idatLength_ = 0;
    incrementalLength_ = 0;
    pixelsData_ = nullptr;
    outputRowsNum_ = 0;
    decodeHeadFlag_ = false;
    firstRow_ = 0;
    lastRow_ = 0;
    interlacedComplete_ = false;
}

// private interface
bool PngDecoder::ConvertOriginalFormat(png_byte source, png_byte &destination)
{
    if (png_get_valid(pngStructPtr_, pngInfoPtr_, PNG_INFO_tRNS)) {
        png_set_tRNS_to_alpha(pngStructPtr_);
    }
    HiLog::Info(LABEL, "color type:[%{public}d]", source);
    switch (source) {
        case PNG_COLOR_TYPE_PALETTE: {  // value is 3
            png_set_palette_to_rgb(pngStructPtr_);
            destination = PNG_COLOR_TYPE_RGB;
            break;
        }
        case PNG_COLOR_TYPE_GRAY: {            // value is 0
            if (pngImageInfo_.bitDepth < 8) {  // 8 is single pixel bit depth
                png_set_expand_gray_1_2_4_to_8(pngStructPtr_);
            }
            png_set_gray_to_rgb(pngStructPtr_);
            destination = PNG_COLOR_TYPE_RGB;
            break;
        }
        case PNG_COLOR_TYPE_GRAY_ALPHA: {  // value is 4
            png_set_gray_to_rgb(pngStructPtr_);
            destination = PNG_COLOR_TYPE_RGB;
            break;
        }
        case PNG_COLOR_TYPE_RGB:
        case PNG_COLOR_TYPE_RGB_ALPHA: {  // value is 6
            destination = source;
            break;
        }
        default: {
            HiLog::Error(LABEL, "the color type:[%{public}d] libpng unsupported!", source);
            return false;
        }
    }

    return true;
}

uint32_t PngDecoder::GetDecodeFormat(PlPixelFormat format, PlPixelFormat &outputFormat, PlAlphaType &alphaType)
{
    png_byte sourceType = png_get_color_type(pngStructPtr_, pngInfoPtr_);
    if ((sourceType & PNG_COLOR_MASK_ALPHA) || png_get_valid(pngStructPtr_, pngInfoPtr_, PNG_INFO_tRNS)) {
        alphaType = PlAlphaType::IMAGE_ALPHA_TYPE_UNPREMUL;
    } else {
        alphaType = PlAlphaType::IMAGE_ALPHA_TYPE_OPAQUE;
    }
    png_byte destType = 0;
    if (!ConvertOriginalFormat(sourceType, destType)) {
        return ERR_IMAGE_DATA_UNSUPPORT;
    }
    if (format != PlPixelFormat::RGB_888 && destType == PNG_COLOR_TYPE_RGB) {
        png_set_add_alpha(pngStructPtr_, 0xff, PNG_FILLER_AFTER);  // 0xffff add the A after RGB.
    }
    // only support 8 bit depth for each pixel except for RGBA_F16
    if (format != PlPixelFormat::RGBA_F16 && pngImageInfo_.bitDepth == 16) {  // 16bit depth
        pngImageInfo_.bitDepth = 8;  // 8bit depth
        png_set_strip_16(pngStructPtr_);
    }
    ChooseFormat(format, outputFormat, destType);
    return SUCCESS;
}

void PngDecoder::ChooseFormat(PlPixelFormat format, PlPixelFormat &outputFormat,
                              png_byte destType)
{
    outputFormat = format;
    switch (format) {
        case PlPixelFormat::BGRA_8888: {
            pngImageInfo_.rowDataSize = pngImageInfo_.width * 4;  // 4 is BGRA size
            png_set_bgr(pngStructPtr_);
            break;
        }
        case PlPixelFormat::ARGB_8888: {
            png_set_swap_alpha(pngStructPtr_);
            pngImageInfo_.rowDataSize = pngImageInfo_.width * 4;  // 4 is ARGB size
            break;
        }
        case PlPixelFormat::RGB_888: {
            if (destType == PNG_COLOR_TYPE_RGBA) {
                png_set_strip_alpha(pngStructPtr_);
            }
            pngImageInfo_.rowDataSize = pngImageInfo_.width * 3;  // 3 is RGB size
            break;
        }
        case PlPixelFormat::RGBA_F16: {
            png_set_scale_16(pngStructPtr_);
            pngImageInfo_.rowDataSize = pngImageInfo_.width * 7;  // 7 is RRGGBBA size
            break;
        }
        case PlPixelFormat::UNKNOWN:
        case PlPixelFormat::RGBA_8888:
        default: {
            pngImageInfo_.rowDataSize = pngImageInfo_.width * 4;  // 4 is RGBA size
            outputFormat = PlPixelFormat::RGBA_8888;
            break;
        }
    }
}

void PngDecoder::PngErrorExit(png_structp pngPtr, png_const_charp message)
{
    if ((pngPtr == nullptr) || (message == nullptr)) {
        HiLog::Error(LABEL, "ErrorExit png_structp or error message is null.");
        return;
    }
    jmp_buf *jmpBuf = &(png_jmpbuf(pngPtr));
    if (jmpBuf == nullptr) {
        HiLog::Error(LABEL, "jmpBuf exception.");
        return;
    }
    longjmp(*jmpBuf, SET_JUMP_VALUE);
}

void PngDecoder::PngWarning(png_structp pngPtr, png_const_charp message)
{
    if (message == nullptr) {
        HiLog::Error(LABEL, "WarningExit message is null.");
        return;
    }
    HiLog::Warn(LABEL, "png warn %{public}s", message);
}

void PngDecoder::PngErrorMessage(png_structp pngPtr, png_const_charp message)
{
    if (message == nullptr) {
        HiLog::Error(LABEL, "PngErrorMessage message is null.");
        return;
    }
    HiLog::Error(LABEL, "PngErrorMessage, message:%{public}s.", message);
}

void PngDecoder::PngWarningMessage(png_structp pngPtr, png_const_charp message)
{
    if (message == nullptr) {
        HiLog::Error(LABEL, "PngWarningMessage message is null.");
        return;
    }
    HiLog::Error(LABEL, "PngWarningMessage, message:%{public}s.", message);
}

// image incremental decode Interface
uint32_t PngDecoder::ProcessData(png_structp pngStructPtr, png_infop infoStructPtr, InputDataStream *sourceStream,
                                 DataStreamBuffer streamData, size_t bufferSize, size_t totalSize)
{
    if (pngStructPtr == nullptr || infoStructPtr == nullptr || sourceStream == nullptr || totalSize == 0 ||
        bufferSize == 0) {
        HiLog::Error(LABEL, "ProcessData input error, totalSize:%{public}zu, bufferSize:%{public}zu.", totalSize,
                     bufferSize);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    while (totalSize > 0) {
        size_t readSize = (bufferSize < totalSize) ? bufferSize : totalSize;
        uint32_t ret = IncrementalRead(sourceStream, readSize, streamData);
        if (ret != SUCCESS) {
            HiLog::Error(LABEL, "ProcessData Read from source stream fail, readSize:%{public}zu,\
                        bufferSize:%{public}zu, dataSize:%{public}u, totalSize:%{public}zu.",
                         readSize, bufferSize, streamData.dataSize, totalSize);
            return ret;
        }
        png_process_data(pngStructPtr, infoStructPtr, const_cast<png_bytep>(streamData.inputStreamBuffer),
                         streamData.dataSize);
        totalSize -= streamData.dataSize;
    }
    return SUCCESS;
}

bool PngDecoder::IsChunk(const png_byte *chunk, const char *flag)
{
    if (chunk == nullptr || flag == nullptr) {
        HiLog::Error(LABEL, "IsChunk input parameter exception.");
        return false;
    }
    return memcmp(chunk + CHUNK_DATA_LEN, flag, CHUNK_DATA_LEN) == 0;
}

bool PngDecoder::GetImageInfo(PngImageInfo &info)
{
    png_uint_32 origWidth = 0;
    png_uint_32 origHeight = 0;
    int32_t bitDepth = 0;
    png_get_IHDR(pngStructPtr_, pngInfoPtr_, &origWidth, &origHeight, &bitDepth, nullptr, nullptr, nullptr, nullptr);
    if ((origWidth == 0) || (origHeight == 0) || (origWidth > PNG_UINT_31_MAX) || (origHeight > PNG_UINT_31_MAX)) {
        HiLog::Error(LABEL, "Get the png image size abnormal, width:%{public}u, height:%{public}u", origWidth,
                     origHeight);
        return false;
    }
    if (bitDepth != BITDEPTH_VALUE_1 && bitDepth != BITDEPTH_VALUE_2 && bitDepth != BITDEPTH_VALUE_4 &&
        bitDepth != BITDEPTH_VALUE_8 && bitDepth != BITDEPTH_VALUE_16) {
        HiLog::Error(LABEL, "Get the png image bit depth abnormal, bitDepth:%{public}d.", bitDepth);
        return false;
    }
    size_t rowDataSize = png_get_rowbytes(pngStructPtr_, pngInfoPtr_);
    if (rowDataSize == 0) {
        HiLog::Error(LABEL, "Get the bitmap row bytes size fail.");
        return false;
    }
    info.numberPasses = png_set_interlace_handling(pngStructPtr_);
    info.width = origWidth;
    info.height = origHeight;
    info.bitDepth = bitDepth;
    info.rowDataSize = rowDataSize;
    HiLog::Info(LABEL, "GetImageInfo:width:%{public}u,height:%{public}u,bitDepth:%{public}u,numberPasses:%{public}d.",
        origWidth, origHeight, info.bitDepth, info.numberPasses);
    return true;
}

uint32_t PngDecoder::IncrementalRead(InputDataStream *stream, uint32_t desiredSize, DataStreamBuffer &outData)
{
    uint32_t curPos = stream->Tell();
    if (!stream->Read(desiredSize, outData)) {
        HiLog::Debug(LABEL, "read data fail.");
        return ERR_IMAGE_SOURCE_DATA_INCOMPLETE;
    }
    if (outData.inputStreamBuffer == nullptr || outData.dataSize == 0) {
        HiLog::Error(LABEL, "inputStreamBuffer is null or data size is %{public}u.", outData.dataSize);
        return ERR_IMAGE_GET_DATA_ABNORMAL;
    }
    if (outData.dataSize < desiredSize) {
        stream->Seek(curPos);
        HiLog::Debug(LABEL, "read outdata size[%{public}u] < data size[%{public}u] and curpos:%{public}u",
                     outData.dataSize, desiredSize, curPos);
        return ERR_IMAGE_SOURCE_DATA_INCOMPLETE;
    }
    return SUCCESS;
}

uint32_t PngDecoder::GetImageIdatSize(InputDataStream *stream)
{
    uint32_t ret = 0;
    DataStreamBuffer readData;
    while (true) {
        uint32_t preReadPos = stream->Tell();
        ret = IncrementalRead(stream, static_cast<uint32_t>(CHUNK_SIZE), readData);
        if (ret != SUCCESS) {
            break;
        }
        png_byte *chunk = const_cast<png_byte *>(readData.inputStreamBuffer);
        const size_t length = png_get_uint_32(chunk);
        if (IsChunk(chunk, "IDAT")) {
            HiLog::Debug(LABEL, "first idat Length is %{public}zu.", length);
            idatLength_ = length;
            return SUCCESS;
        }
        uint32_t afterReadPos = stream->Tell();
        if (!stream->Seek(length + afterReadPos + CHUNK_DATA_LEN)) {
            HiLog::Debug(LABEL, "stream current pos is %{public}u, chunk size is %{public}zu.", preReadPos, length);
            stream->Seek(preReadPos);
            return ERR_IMAGE_SOURCE_DATA_INCOMPLETE;
        }
        stream->Seek(afterReadPos);
        png_process_data(pngStructPtr_, pngInfoPtr_, chunk, CHUNK_SIZE);
        ret = ProcessData(pngStructPtr_, pngInfoPtr_, stream, readData, DECODE_BUFFER_SIZE, length + CHUNK_DATA_LEN);
        if (ret != SUCCESS) {
            break;
        }
    }
    return ret;
}

uint32_t PngDecoder::ReadIncrementalHead(InputDataStream *stream, PngImageInfo &info)
{
    if (stream == nullptr) {
        HiLog::Error(LABEL, "read incremental head input data is null!");
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    uint32_t pos = stream->Tell();
    if (!stream->Seek(PNG_HEAD_SIZE)) {
        HiLog::Debug(LABEL, "don't enough the data to decode the image head.");
        return ERR_IMAGE_SOURCE_DATA_INCOMPLETE;
    }
    stream->Seek(pos);
    // set the exception handle
    jmp_buf *jmpBuf = &(png_jmpbuf(pngStructPtr_));
    if ((jmpBuf == nullptr) || setjmp(*jmpBuf)) {
        HiLog::Error(LABEL, "read incremental head PNG decode head exception.");
        return ERR_IMAGE_DECODE_HEAD_ABNORMAL;
    }

    DataStreamBuffer readData;
    if (!decodeHeadFlag_) {
        png_set_keep_unknown_chunks(pngStructPtr_, PNG_HANDLE_CHUNK_ALWAYS, (png_byte *)"", 0);
        png_set_read_user_chunk_fn(pngStructPtr_, static_cast<png_voidp>(&ninePatch_), ReadUserChunk);
        png_set_progressive_read_fn(pngStructPtr_, nullptr, nullptr, nullptr, nullptr);
        uint32_t ret = IncrementalRead(stream, static_cast<uint32_t>(CHUNK_SIZE), readData);
        if (ret != SUCCESS) {
            return ret;
        }
        png_bytep head = const_cast<png_bytep>(readData.inputStreamBuffer);
        png_process_data(pngStructPtr_, pngInfoPtr_, head, CHUNK_SIZE);
        decodeHeadFlag_ = true;
    }
    uint32_t ret = GetImageIdatSize(stream);
    if (ret != SUCCESS) {
        HiLog::Error(LABEL, "get image idat size fail, ret:%{public}u.", ret);
        return ret;
    }
    if (!GetImageInfo(info)) {
        return ERR_IMAGE_DECODE_HEAD_ABNORMAL;
    }
    return SUCCESS;
}

void PngDecoder::SaveRows(png_bytep row, png_uint_32 rowNum)
{
    if (rowNum != outputRowsNum_ || pngImageInfo_.height < rowNum) {
        HiLog::Error(LABEL,
                     "AllRowsCallback exception, rowNum:%{public}u, outputRowsNum:%{public}u, height:%{public}u.",
                     rowNum, outputRowsNum_, pngImageInfo_.height);
        return;
    }
    outputRowsNum_++;
    uint8_t *offset = pixelsData_ + rowNum * pngImageInfo_.rowDataSize;
    uint32_t offsetSize = (pngImageInfo_.height - rowNum) * pngImageInfo_.rowDataSize;
    errno_t ret = memcpy_s(offset, offsetSize, row, pngImageInfo_.rowDataSize);
    if (ret != 0) {
        HiLog::Error(LABEL, "copy data fail, ret:%{public}d, rowDataSize:%{public}u, offsetSize:%{public}u.", ret,
                     pngImageInfo_.rowDataSize, offsetSize);
        return;
    }
}

void PngDecoder::SaveInterlacedRows(png_bytep row, png_uint_32 rowNum, int pass)
{
    if (row == nullptr) {
        HiLog::Error(LABEL, "input row is null.");
        return;
    }
    if (rowNum < firstRow_ || rowNum > lastRow_ || interlacedComplete_) {
        HiLog::Error(LABEL, "ignore this row, rowNum:%{public}u,InterlacedComplete:%{public}u.", rowNum,
                     interlacedComplete_);
        return;
    }
    png_bytep oldRow = pixelsData_ + (rowNum - firstRow_) * pngImageInfo_.rowDataSize;
    uint64_t mollocByteCount = static_cast<uint64_t>(pngImageInfo_.rowDataSize) * pngImageInfo_.height;
    uint64_t needByteCount = static_cast<uint64_t>(pngStructPtr_->width) * sizeof(*oldRow);
    if (mollocByteCount < needByteCount) {
        HiLog::Error(LABEL, "malloc byte size is(%{public}llu), but actual needs (%{public}llu)",
                     static_cast<unsigned long long>(mollocByteCount), static_cast<unsigned long long>(needByteCount));
        return;
    }
    png_progressive_combine_row(pngStructPtr_, oldRow, row);
    if (pass == 0) {
        // The first pass initializes all rows.
        if (outputRowsNum_ == rowNum - firstRow_) {
            HiLog::Error(LABEL, "rowNum(%{public}u) - firstRow(%{public}u) = outputRow(%{public}u)", rowNum, firstRow_,
                         outputRowsNum_);
            return;
        }
        outputRowsNum_++;
    } else {
        if (outputRowsNum_ == lastRow_ - firstRow_ + 1) {
            HiLog::Error(LABEL, "lastRow_(%{public}u) + firstRow(%{public}u) + 1 = outputRow(%{public}u)", lastRow_,
                         firstRow_, outputRowsNum_);
            return;
        }
        if (pngImageInfo_.numberPasses - 1 == pass && rowNum == lastRow_) {
            // Last pass, and we have read all of the rows we care about.
            HiLog::Error(LABEL, "last pass:%{public}d, numberPasses:%{public}d, rowNum:%{public}d, lastRow:%{public}d.",
                         pass, pngImageInfo_.numberPasses, rowNum, lastRow_);
            interlacedComplete_ = true;
        }
    }
}

void PngDecoder::GetAllRows(png_structp pngPtr, png_bytep row, png_uint_32 rowNum, int pass)
{
    if (pngPtr == nullptr || row == nullptr) {
        HiLog::Error(LABEL, "get decode rows exception, rowNum:%{public}u.", rowNum);
        return;
    }
    PngDecoder *decoder = static_cast<PngDecoder *>(png_get_progressive_ptr(pngPtr));
    if (decoder == nullptr) {
        HiLog::Error(LABEL, "get all rows fail, get decoder is null.");
        return;
    }
    decoder->SaveRows(row, rowNum);
}

void PngDecoder::GetInterlacedRows(png_structp pngPtr, png_bytep row, png_uint_32 rowNum, int pass)
{
    if (pngPtr == nullptr || row == nullptr) {
        HiLog::Debug(LABEL, "get decode rows exception, rowNum:%{public}u.", rowNum);
        return;
    }
    PngDecoder *decoder = static_cast<PngDecoder *>(png_get_progressive_ptr(pngPtr));
    if (decoder == nullptr) {
        HiLog::Error(LABEL, "get all rows fail, get decoder is null.");
        return;
    }
    decoder->SaveInterlacedRows(row, rowNum, pass);
}

int32_t PngDecoder::ReadUserChunk(png_structp png_ptr, png_unknown_chunkp chunk)
{
    NinePatchListener *chunkReader = static_cast<NinePatchListener *>(png_get_user_chunk_ptr(png_ptr));
    if (chunkReader == nullptr) {
        HiLog::Error(LABEL, "chunk header is null.");
        return ERR_IMAGE_DECODE_ABNORMAL;
    }
    return chunkReader->ReadChunk(reinterpret_cast<const char *>(chunk->name), chunk->data, chunk->size)
               ? SUCCESS
               : ERR_IMAGE_DECODE_ABNORMAL;
}

uint32_t PngDecoder::PushAllToDecode(InputDataStream *stream, size_t bufferSize, size_t length)
{
    if (stream == nullptr || bufferSize == 0 || length == 0) {
        HiLog::Error(LABEL, "iend process input exception, bufferSize:%{public}zu, length:%{public}zu.", bufferSize,
                     length);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    DataStreamBuffer ReadData;
    if (ProcessData(pngStructPtr_, pngInfoPtr_, stream, ReadData, bufferSize, length) != SUCCESS) {
        HiLog::Error(LABEL, "ProcessData return false, bufferSize:%{public}zu, length:%{public}zu.", bufferSize,
                     length);
        return ERR_IMAGE_DECODE_ABNORMAL;
    }
    bool iend = false;
    uint32_t ret = 0;
    while (true) {
        // Parse chunk length and type.
        ret = IncrementalRead(stream, CHUNK_SIZE, ReadData);
        if (ret != SUCCESS) {
            HiLog::Error(LABEL, "set iend mode Read chunk fail,ret:%{public}u", ret);
            break;
        }
        png_byte *chunk = const_cast<png_byte *>(ReadData.inputStreamBuffer);
        png_process_data(pngStructPtr_, pngInfoPtr_, chunk, CHUNK_SIZE);
        if (IsChunk(chunk, "IEND")) {
            iend = true;
        }
        size_t chunkLength = png_get_uint_32(chunk);
        // Process the full chunk + CRC
        ret = ProcessData(pngStructPtr_, pngInfoPtr_, stream, ReadData, bufferSize, chunkLength + CHUNK_DATA_LEN);
        if (ret != SUCCESS || iend) {
            break;
        }
    }
    return ret;
}

uint32_t PngDecoder::IncrementalReadRows(InputDataStream *stream)
{
    if (stream == nullptr) {
        HiLog::Error(LABEL, "input data is null!");
        return ERR_IMAGE_GET_DATA_ABNORMAL;
    }
    if (idatLength_ < incrementalLength_) {
        HiLog::Error(LABEL, "incremental len:%{public}zu > idat len:%{public}zu.", incrementalLength_, idatLength_);
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    // set the exception handle
    jmp_buf *jmpBuf = &(png_jmpbuf(pngStructPtr_));
    if ((jmpBuf == nullptr) || setjmp(*jmpBuf)) {
        HiLog::Error(LABEL, "[IncrementalReadRows]PNG decode exception.");
        return ERR_IMAGE_DECODE_ABNORMAL;
    }
    // set process decode state to IDAT mode.
    if (!decodedIdat_) {
        if (pngImageInfo_.numberPasses == 1) {
            png_set_progressive_read_fn(pngStructPtr_, this, nullptr, GetAllRows, nullptr);
        } else {
            png_set_progressive_read_fn(pngStructPtr_, this, nullptr, GetInterlacedRows, nullptr);
            lastRow_ = pngImageInfo_.height - 1;  // decode begin to 0
        }
        png_byte idat[] = { 0, 0, 0, 0, 'I', 'D', 'A', 'T' };
        png_save_uint_32(idat, idatLength_);
        png_process_data(pngStructPtr_, pngInfoPtr_, idat, CHUNK_SIZE);
        decodedIdat_ = true;
        idatLength_ += CHUNK_DATA_LEN;
    }
    if (stream->IsStreamCompleted()) {
        uint32_t ret = PushAllToDecode(stream, DECODE_BUFFER_SIZE, idatLength_ - incrementalLength_);
        if (ret != SUCCESS) {
            HiLog::Error(LABEL, "iend set fail, ret:%{public}u, idatLen:%{public}zu, incrementalLen:%{public}zu.", ret,
                         idatLength_, incrementalLength_);
            return ret;
        }
        return SUCCESS;
    }
    uint32_t ret = PushCurrentToDecode(stream);
    if (ret != SUCCESS) {
        HiLog::Error(LABEL,
                     "push stream to decode fail, ret:%{public}u, idatLen:%{public}zu, incrementalLen:%{public}zu.",
                     ret, idatLength_, incrementalLength_);
        return ret;
    }
    return SUCCESS;
}

uint32_t PngDecoder::PushCurrentToDecode(InputDataStream *stream)
{
    if (stream == nullptr) {
        HiLog::Error(LABEL, "push current stream to decode input data is null!");
        return ERR_IMAGE_GET_DATA_ABNORMAL;
    }
    if (idatLength_ == 0) {
        HiLog::Error(LABEL, "idat Length is zero.");
        return ERR_IMAGE_DECODE_ABNORMAL;
    }

    DataStreamBuffer ReadData;
    uint32_t ret = 0;
    while (incrementalLength_ < idatLength_) {
        const size_t targetSize = std::min(DECODE_BUFFER_SIZE, idatLength_ - incrementalLength_);
        ret = IncrementalRead(stream, targetSize, ReadData);
        if (ret != SUCCESS) {
            HiLog::Debug(LABEL, "push current stream read fail, ret:%{public}u", ret);
            return ret;
        }
        incrementalLength_ += ReadData.dataSize;
        png_process_data(pngStructPtr_, pngInfoPtr_, (png_bytep)ReadData.inputStreamBuffer, ReadData.dataSize);
    }

    while (true) {
        ret = IncrementalRead(stream, CHUNK_SIZE, ReadData);
        if (ret != SUCCESS) {
            HiLog::Debug(LABEL, "set iend mode Read chunk fail,ret:%{public}u", ret);
            break;
        }
        png_byte *chunk = const_cast<png_byte *>(ReadData.inputStreamBuffer);
        png_process_data(pngStructPtr_, pngInfoPtr_, chunk, CHUNK_SIZE);
        idatLength_ = png_get_uint_32(chunk) + CHUNK_DATA_LEN;
        incrementalLength_ = 0;
        while (incrementalLength_ < idatLength_) {
            const size_t targetSize = std::min(DECODE_BUFFER_SIZE, idatLength_ - incrementalLength_);
            ret = IncrementalRead(stream, targetSize, ReadData);
            if (ret != SUCCESS) {
                HiLog::Debug(LABEL, "push current stream read fail, ret:%{public}u", ret);
                return ret;
            }
            incrementalLength_ += ReadData.dataSize;
            png_process_data(pngStructPtr_, pngInfoPtr_, (png_bytep)ReadData.inputStreamBuffer, ReadData.dataSize);
        }
    }
    return ret;
}

uint32_t PngDecoder::DecodeHeader()
{
    // only state PngDecodingState::SOURCE_INITED and PngDecodingState::BASE_INFO_PARSING can go in this function.
    if (inputStreamPtr_->IsStreamCompleted()) {
        // decode the png image header
        inputStreamPtr_->Seek(0);
    }
    // incremental decode the png image header
    if (state_ == PngDecodingState::SOURCE_INITED) {
        inputStreamPtr_->Seek(0);
    } else {
        inputStreamPtr_->Seek(streamPosition_);
    }
    uint32_t ret = ReadIncrementalHead(inputStreamPtr_, pngImageInfo_);
    if (ret != SUCCESS) {
        if (ret == ERR_IMAGE_SOURCE_DATA_INCOMPLETE) {
            streamPosition_ = inputStreamPtr_->Tell();
            state_ = PngDecodingState::BASE_INFO_PARSING;
        } else {
            state_ = PngDecodingState::SOURCE_INITED;
            HiLog::Error(LABEL, "decode image head, ret:%{public}u.", ret);
        }
        return ret;
    }
    if (pngImageInfo_.width == 0 || pngImageInfo_.height == 0) {
        HiLog::Error(LABEL, "get width and height fail, height:%{public}u, width:%{public}u.", pngImageInfo_.height,
                     pngImageInfo_.width);
        state_ = PngDecodingState::SOURCE_INITED;
        return ERR_IMAGE_GET_DATA_ABNORMAL;
    }
    streamPosition_ = inputStreamPtr_->Tell();
    state_ = PngDecodingState::BASE_INFO_PARSED;
    return SUCCESS;
}

uint32_t PngDecoder::ConfigInfo(const PixelDecodeOptions &opts)
{
    uint32_t ret = SUCCESS;
    bool isComeNinePatchRGB565 = false;
    if (ninePatch_.patch_ != nullptr) {
        // Do not allow ninepatch decodes to 565,use RGBA_8888;
        if (opts.desiredPixelFormat == PlPixelFormat::RGB_565) {
            ret = GetDecodeFormat(PlPixelFormat::RGBA_8888, outputFormat_, alphaType_);
            isComeNinePatchRGB565 = true;
        }
    }
    if (!isComeNinePatchRGB565) {
        ret = GetDecodeFormat(opts.desiredPixelFormat, outputFormat_, alphaType_);
    }
    if (ret != SUCCESS) {
        HiLog::Error(LABEL, "get the color type fail.");
        return ERR_IMAGE_DATA_ABNORMAL;
    }

    // get the libpng interface exception.
    jmp_buf *jmpBuf = &(png_jmpbuf(pngStructPtr_));
    if ((jmpBuf == nullptr) || setjmp(*jmpBuf)) {
        HiLog::Error(LABEL, "config decoding info fail.");
        return ERR_IMAGE_DATA_ABNORMAL;
    }
    png_read_update_info(pngStructPtr_, pngInfoPtr_);
    return SUCCESS;
}

uint32_t PngDecoder::DoOneTimeDecode(DecodeContext &context)
{
    if (idatLength_ <= 0) {
        HiLog::Error(LABEL, "normal decode the image source incomplete.");
        return ERR_IMAGE_SOURCE_DATA_INCOMPLETE;
    }
    jmp_buf *jmpBuf = &(png_jmpbuf(pngStructPtr_));
    if ((jmpBuf == nullptr) || setjmp(*jmpBuf)) {
        HiLog::Error(LABEL, "decode the image fail.");
        return ERR_IMAGE_DECODE_ABNORMAL;
    }
    pixelsData_ = AllocOutputHeapBuffer(context);
    if (pixelsData_ == nullptr) {
        HiLog::Error(LABEL, "get pixels memory fail.");
        return ERR_IMAGE_MALLOC_ABNORMAL;
    }
    inputStreamPtr_->Seek(streamPosition_);
    uint32_t ret = IncrementalReadRows(inputStreamPtr_);
    if (ret != SUCCESS) {
        HiLog::Error(LABEL, "normal decode the image fail, ret:%{public}u", ret);
        return ret;
    }
    streamPosition_ = inputStreamPtr_->Tell();
    return SUCCESS;
}

bool PngDecoder::FinishOldDecompress()
{
    if (state_ < PngDecodingState::IMAGE_DECODING) {
        return true;
    }

    InputDataStream *temp = inputStreamPtr_;
    Reset();
    inputStreamPtr_ = temp;
    // destroy the png decode struct
    if (pngStructPtr_ != nullptr) {
        png_infopp pngInfoPtr = pngInfoPtr_ ? &pngInfoPtr_ : nullptr;
        png_destroy_read_struct(&pngStructPtr_, pngInfoPtr, nullptr);
        HiLog::Debug(LABEL, "FinishOldDecompress png_destroy_read_struct");
    }
    state_ = PngDecodingState::SOURCE_INITED;
    if (InitPnglib()) {
        return true;
    }
    return false;
}

bool PngDecoder::InitPnglib()
{
    // create the png decode struct
    pngStructPtr_ = png_create_read_struct(PNG_LIBPNG_VER_STRING, nullptr, PngErrorExit, PngWarning);
    pngInfoPtr_ = png_create_info_struct(pngStructPtr_);
    // set the libpng exception message callback function
    png_set_error_fn(pngStructPtr_, nullptr, PngErrorMessage, PngWarningMessage);
    if (pngStructPtr_ == nullptr || pngInfoPtr_ == nullptr) {
        HiLog::Error(LABEL, "Png lib init fail.");
        return false;
    }
    return true;
}

void PngDecoder::DealNinePatch(const PixelDecodeOptions &opts)
{
    if (ninePatch_.patch_ != nullptr) {
        if (opts.desiredSize.width > 0 && opts.desiredSize.height > 0) {
            const float scaleX = static_cast<float>(opts.desiredSize.width) / pngImageInfo_.width;
            const float scaleY = static_cast<float>(opts.desiredSize.height) / pngImageInfo_.height;
            ninePatch_.Scale(scaleX, scaleY, opts.desiredSize.width, opts.desiredSize.height);
        }
    }
}
} // namespace ImagePlugin
} // namespace OHOS
