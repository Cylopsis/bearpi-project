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

#include "jpeg_utils.h"

namespace OHOS {
namespace ImagePlugin {
using namespace OHOS::HiviewDFX;

static constexpr HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "JpegUtils" };

// these functions are called by libjpeg-turbo third_party library, no need check input parameter.
// for error manager
void ErrorExit(j_common_ptr dinfo)
{
    if ((dinfo == nullptr) || (dinfo->err == nullptr)) {
        return;
    }
    // dinfo->err really points to a ErrorMgr struct, so coerce pointer.
    ErrorMgr *err = static_cast<ErrorMgr *>(dinfo->err);
    (*dinfo->err->output_message)(dinfo);
    // return control to the setjmp point.
    longjmp(err->setjmp_buffer, SET_JUMP_VALUE);
}

void OutputErrorMessage(j_common_ptr dinfo)
{
    if ((dinfo == nullptr) || (dinfo->err == nullptr)) {
        return;
    }
    char buffer[JMSG_LENGTH_MAX] = { 0 };
    dinfo->err->format_message(dinfo, buffer);
    HiLog::Error(LABEL, "libjpeg error %{public}d <%{public}s>.", dinfo->err->msg_code, buffer);
}

// for source manager
// this is called by jpeg_read_header() before any data is actually read.
void InitSrcStream(j_decompress_ptr dinfo)
{
    if ((dinfo == nullptr) || (dinfo->src == nullptr)) {
        HiLog::Error(LABEL, "init source stream error.");
        return;
    }
    JpegSrcMgr *src = static_cast<JpegSrcMgr *>(dinfo->src);
    src->next_input_byte = src->streamData.inputStreamBuffer;
    src->bytes_in_buffer = 0;
}

// this is called whenever bytes_in_buffer has reached zero and more data is wanted.
boolean FillInputBuffer(j_decompress_ptr dinfo)
{
    if (dinfo == nullptr) {
        HiLog::Error(LABEL, "fill input buffer error, decompress struct is null.");
        return FALSE;
    }
    JpegSrcMgr *src = static_cast<JpegSrcMgr *>(dinfo->src);
    if ((src == nullptr) || (src->inputStream == nullptr)) {
        HiLog::Error(LABEL, "fill input buffer error, source stream is null.");
        ERREXIT(dinfo, JERR_FILE_READ);
        return FALSE;
    }

    uint32_t preReadPos = src->inputStream->Tell();
    if (!src->inputStream->IsStreamCompleted() && !src->inputStream->Seek(preReadPos + JPEG_BUFFER_SIZE)) {
        return FALSE;
    }
    src->inputStream->Seek(preReadPos);
    if (!src->inputStream->Read(src->bufferSize, src->streamData)) {
        HiLog::Error(LABEL, "fill input buffer error, read source stream failed.");
        return FALSE;
    }
    if (!src->inputStream->IsStreamCompleted() && src->streamData.dataSize < JPEG_BUFFER_SIZE) {
        uint32_t curr = src->inputStream->Tell();
        src->inputStream->Seek(curr - src->streamData.dataSize);
        HiLog::Debug(LABEL, "fill input buffer seekTo=%{public}u, rewindSize=%{public}u.",
                     curr - src->streamData.dataSize, src->streamData.dataSize);
        return FALSE;
    }
    src->next_input_byte = src->streamData.inputStreamBuffer;
    src->bytes_in_buffer = src->streamData.dataSize;
    return TRUE;
}

// skip num_bytes worth of data.
void SkipInputData(j_decompress_ptr dinfo, long numBytes)
{
    if (dinfo == nullptr) {
        HiLog::Error(LABEL, "skip input buffer error, decompress struct is null.");
        return;
    }
    JpegSrcMgr *src = static_cast<JpegSrcMgr *>(dinfo->src);
    if ((src == nullptr) || (src->inputStream == nullptr)) {
        HiLog::Error(LABEL, "skip input buffer error, source stream is null.");
        ERREXIT(dinfo, JERR_FILE_READ);
        return;
    }
    size_t bytes = static_cast<size_t>(numBytes);
    if (bytes > src->bytes_in_buffer) {
        size_t bytesToSkip = bytes - src->bytes_in_buffer;
        uint32_t nowOffset = src->inputStream->Tell();
        if (bytesToSkip > src->inputStream->GetStreamSize() - nowOffset) {
            HiLog::Error(LABEL, "skip data:%{public}zu larger than current offset:%{public}u.", bytesToSkip, nowOffset);
            return;
        }
        if (!src->inputStream->Seek(nowOffset + bytesToSkip)) {
            HiLog::Error(LABEL, "skip data:%{public}zu fail, current offset:%{public}u.", bytesToSkip, nowOffset);
            ERREXIT(dinfo, JERR_FILE_READ);
            return;
        }
        src->next_input_byte = src->streamData.inputStreamBuffer;
        src->bytes_in_buffer = 0;
    } else {
        src->next_input_byte += numBytes;
        src->bytes_in_buffer -= numBytes;
    }
}

// this is called by jpeg_finish_decompress() after all data has been read. Often a no-op.
void TermSrcStream(j_decompress_ptr dinfo)
{}

// for destination manager
// this is called by jpeg_start_compress() before any data is actually written.
void InitDstStream(j_compress_ptr cinfo)
{
    if ((cinfo == nullptr) || (cinfo->dest == nullptr)) {
        HiLog::Error(LABEL, "init destination stream error.");
        return;
    }
    JpegDstMgr *dest = static_cast<JpegDstMgr *>(cinfo->dest);
    dest->next_output_byte = dest->buffer;
    dest->free_in_buffer = dest->bufferSize;
}

// this is called whenever the buffer has filled (free_in_buffer reaches zero).
boolean EmptyOutputBuffer(j_compress_ptr cinfo)
{
    if (cinfo == nullptr) {
        HiLog::Error(LABEL, "write output buffer error, compress struct is null.");
        return FALSE;
    }
    JpegDstMgr *dest = static_cast<JpegDstMgr *>(cinfo->dest);
    if ((dest == nullptr) || (dest->outputStream == nullptr)) {
        HiLog::Error(LABEL, "write output buffer error, dest stream is null.");
        ERREXIT(cinfo, JERR_FILE_WRITE);
        return FALSE;
    }
    if (!dest->outputStream->Write(dest->buffer, dest->bufferSize)) {
        HiLog::Error(LABEL, "write output buffer error, write dest stream failed.");
        ERREXIT(cinfo, JERR_FILE_WRITE);
        return FALSE;
    }
    dest->next_output_byte = dest->buffer;
    dest->free_in_buffer = dest->bufferSize;
    return TRUE;
}

// this is called by jpeg_finish_compress() after all data has been written.
void TermDstStream(j_compress_ptr cinfo)
{
    if (cinfo == nullptr) {
        HiLog::Error(LABEL, "term output buffer error, compress struct is null.");
        return;
    }
    JpegDstMgr *dest = static_cast<JpegDstMgr *>(cinfo->dest);
    if ((dest == nullptr) || (dest->outputStream == nullptr)) {
        HiLog::Error(LABEL, "term output buffer error, dest stream is null.");
        ERREXIT(cinfo, JERR_FILE_WRITE);
        return;
    }
    size_t size = dest->bufferSize - dest->free_in_buffer;
    if (size > 0) {
        if (!dest->outputStream->Write(dest->buffer, size)) {
            HiLog::Error(LABEL, "term output buffer error, write dest stream size:%{public}zu failed.", size);
            ERREXIT(cinfo, JERR_FILE_WRITE);
            return;
        }
    }
    dest->outputStream->Flush();
}
} // namespace ImagePlugin
} // namespace OHOS
