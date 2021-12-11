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

#ifndef JPEG_UTILS_H
#define JPEG_UTILS_H

#include <setjmp.h>
#include <stdio.h>
#include <cstdint>
#include "hilog/log.h"
#include "input_data_stream.h"
#include "jerror.h"
#include "jpeglib.h"
#include "log_tags.h"
#include "output_data_stream.h"

namespace OHOS {
namespace ImagePlugin {
static constexpr uint8_t SET_JUMP_VALUE = 1;
static constexpr uint8_t RW_LINE_NUM = 1;
static constexpr uint16_t JPEG_BUFFER_SIZE = 1024;
static constexpr uint32_t JPEG_IMAGE_NUM = 1;

// redefine jpeg error manager struct.
struct ErrorMgr : jpeg_error_mgr {
    struct jpeg_error_mgr pub;      // public fields

#ifdef _WIN32
    jmp_buf setjmp_buffer = {{0}};  // for return to caller
#else
    jmp_buf setjmp_buffer;  // for return to caller
#endif
};

// redefine jpeg source manager struct.
struct JpegSrcMgr : jpeg_source_mgr {
    explicit JpegSrcMgr(InputDataStream *stream);

    InputDataStream *inputStream = nullptr;
    uint16_t bufferSize = JPEG_BUFFER_SIZE;
    ImagePlugin::DataStreamBuffer streamData;
};

// redefine jpeg destination manager struct.
struct JpegDstMgr : jpeg_destination_mgr {
    explicit JpegDstMgr(OutputDataStream *stream);

    OutputDataStream *outputStream = nullptr;
    uint16_t bufferSize = JPEG_BUFFER_SIZE;
    uint8_t buffer[JPEG_BUFFER_SIZE] = { 0 };
};

// for jpeg error manager
void ErrorExit(j_common_ptr cinfo);
void OutputErrorMessage(j_common_ptr cinfo);
// for jpeg source manager
void InitSrcStream(j_decompress_ptr dinfo);
boolean FillInputBuffer(j_decompress_ptr dinfo);
void SkipInputData(j_decompress_ptr dinfo, long numBytes);
void TermSrcStream(j_decompress_ptr dinfo);
// for jpeg destination manager
void InitDstStream(j_compress_ptr cinfo);
boolean EmptyOutputBuffer(j_compress_ptr cinfo);
void TermDstStream(j_compress_ptr cinfo);
} // namespace ImagePlugin
} // namespace OHOS

#endif // JPEG_UTILS_H
