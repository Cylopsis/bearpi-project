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

#include "jerror.h"
#include "jpeg_encoder.h"
#include "media_errors.h"

namespace OHOS {
namespace ImagePlugin {
using namespace OHOS::HiviewDFX;
using namespace MultimediaPlugin;
using namespace Media;

constexpr uint32_t COMPONENT_NUM_ARGB = 4;
constexpr uint32_t COMPONENT_NUM_RGBA = 4;
constexpr uint32_t COMPONENT_NUM_BGRA = 4;
constexpr uint32_t COMPONENT_NUM_RGB = 3;
constexpr uint32_t COMPONENT_NUM_GRAY = 1;
// yuv format
constexpr uint8_t COMPONENT_NUM_YUV420SP = 3;
constexpr uint8_t Y_SAMPLE_ROW = 16;
constexpr uint8_t UV_SAMPLE_ROW = 8;
constexpr uint8_t SAMPLE_FACTOR_ONE = 1;
constexpr uint8_t SAMPLE_FACTOR_TWO = 2;
constexpr uint8_t INDEX_ZERO = 0;
constexpr uint8_t INDEX_ONE = 1;
constexpr uint8_t INDEX_TWO = 2;
constexpr uint8_t SHIFT_MASK = 1;

JpegDstMgr::JpegDstMgr(OutputDataStream *stream) : outputStream(stream)
{
    init_destination = InitDstStream;
    empty_output_buffer = EmptyOutputBuffer;
    term_destination = TermDstStream;
}

JpegEncoder::JpegEncoder() : dstMgr_(nullptr)
{
    // create decompress struct
    jpeg_create_compress(&encodeInfo_);

    // set error output
    encodeInfo_.err = jpeg_std_error(&jerr_);
    jerr_.error_exit = ErrorExit;
    if (encodeInfo_.err == nullptr) {
        HiLog::Error(LABEL, "create jpeg encoder failed.");
        return;
    }
    encodeInfo_.err->output_message = &OutputErrorMessage;
}

uint32_t JpegEncoder::StartEncode(OutputDataStream &outputStream, PlEncodeOptions &option)
{
    pixelMaps_.clear();
    dstMgr_.outputStream = &outputStream;
    encodeInfo_.dest = &dstMgr_;
    encodeOpts_ = option;
    return SUCCESS;
}

J_COLOR_SPACE JpegEncoder::GetEncodeFormat(PixelFormat format, int32_t &componentsNum)
{
    J_COLOR_SPACE colorSpace = JCS_UNKNOWN;
    int32_t components = 0;
    switch (format) {
        case PixelFormat::RGBA_8888: {
            colorSpace = JCS_EXT_RGBA;
            components = COMPONENT_NUM_RGBA;
            break;
        }
        case PixelFormat::BGRA_8888: {
            colorSpace = JCS_EXT_BGRA;
            components = COMPONENT_NUM_BGRA;
            break;
        }
        case PixelFormat::ARGB_8888: {
            colorSpace = JCS_EXT_ARGB;
            components = COMPONENT_NUM_ARGB;
            break;
        }
        case PixelFormat::ALPHA_8: {
            colorSpace = JCS_GRAYSCALE;
            components = COMPONENT_NUM_GRAY;
            break;
        }
        case PixelFormat::RGB_888: {
            colorSpace = JCS_RGB;
            components = COMPONENT_NUM_RGB;
            break;
        }
        case PixelFormat::NV12:
        case PixelFormat::NV21: {
            colorSpace = JCS_YCbCr;
            components = COMPONENT_NUM_YUV420SP;
            break;
        }
        case PixelFormat::CMYK: {
            colorSpace = JCS_CMYK;
            components = COMPONENT_NUM_RGBA;
            break;
        }
        default: {
            HiLog::Error(LABEL, "encode format:[%{public}d] is unsupported!", format);
            break;
        }
    }
    componentsNum = components;
    return colorSpace;
}

uint32_t JpegEncoder::AddImage(Media::PixelMap &pixelMap)
{
    if (pixelMaps_.size() >= JPEG_IMAGE_NUM) {
        HiLog::Error(LABEL, "add pixel map out of range:[%{public}u].", JPEG_IMAGE_NUM);
        return ERR_IMAGE_ADD_PIXEL_MAP_FAILED;
    }
    pixelMaps_.push_back(&pixelMap);
    return SUCCESS;
}

uint32_t JpegEncoder::FinalizeEncode()
{
    uint32_t errorCode = SetCommonConfig();
    if (errorCode != SUCCESS) {
        HiLog::Error(LABEL, "set jpeg compress struct failed:%{public}u.", errorCode);
        return errorCode;
    }
    const uint8_t *data = pixelMaps_[0]->GetPixels();
    if (data == nullptr) {
        HiLog::Error(LABEL, "encode image buffer is null.");
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    PixelFormat pixelFormat = pixelMaps_[0]->GetPixelFormat();
    if (pixelFormat == PixelFormat::NV21 || pixelFormat == PixelFormat::NV12) {
        errorCode = Yuv420spEncoder(data);
    } else {
        errorCode = SequenceEncoder(data);
    }
    if (errorCode != SUCCESS) {
        HiLog::Error(LABEL, "encode jpeg failed:%{public}u.", errorCode);
    }
    return errorCode;
}

uint32_t JpegEncoder::SetCommonConfig()
{
    if (pixelMaps_.empty()) {
        HiLog::Error(LABEL, "encode image failed, no pixel map input.");
        return ERR_IMAGE_INVALID_PARAMETER;
    }
    if (setjmp(jerr_.setjmp_buffer)) {
        HiLog::Error(LABEL, "encode image error, set config failed.");
        return ERR_IMAGE_ENCODE_FAILED;
    }
    encodeInfo_.image_width = pixelMaps_[0]->GetWidth();
    encodeInfo_.image_height = pixelMaps_[0]->GetHeight();
    PixelFormat pixelFormat = pixelMaps_[0]->GetPixelFormat();
    encodeInfo_.in_color_space = GetEncodeFormat(pixelFormat, encodeInfo_.input_components);
    if (encodeInfo_.in_color_space == JCS_UNKNOWN) {
        HiLog::Error(LABEL, "set input jpeg color space invalid.");
        return ERR_IMAGE_UNKNOWN_FORMAT;
    }
    HiLog::Debug(LABEL, "width=%{public}u, height=%{public}u, colorspace=%{public}d, components=%{public}d.",
                 encodeInfo_.image_width, encodeInfo_.image_height, encodeInfo_.in_color_space,
                 encodeInfo_.input_components);
    jpeg_set_defaults(&encodeInfo_);
    int32_t quality = encodeOpts_.quality;
    jpeg_set_quality(&encodeInfo_, quality, TRUE);
    return SUCCESS;
}

uint32_t JpegEncoder::SequenceEncoder(const uint8_t *data)
{
    if (setjmp(jerr_.setjmp_buffer)) {
        HiLog::Error(LABEL, "encode image error.");
        return ERR_IMAGE_ENCODE_FAILED;
    }
    jpeg_start_compress(&encodeInfo_, TRUE);
    uint8_t *base = const_cast<uint8_t *>(data);
    uint32_t rowStride = encodeInfo_.image_width * encodeInfo_.input_components;
    uint8_t *buffer = nullptr;
    while (encodeInfo_.next_scanline < encodeInfo_.image_height) {
        buffer = base + encodeInfo_.next_scanline * rowStride;
        jpeg_write_scanlines(&encodeInfo_, &buffer, RW_LINE_NUM);
    }
    jpeg_finish_compress(&encodeInfo_);
    return SUCCESS;
}

void JpegEncoder::SetYuv420spExtraConfig()
{
    encodeInfo_.raw_data_in = TRUE;
    encodeInfo_.dct_method = JDCT_IFAST;
    encodeInfo_.comp_info[INDEX_ZERO].h_samp_factor = SAMPLE_FACTOR_TWO;
    encodeInfo_.comp_info[INDEX_ZERO].v_samp_factor = SAMPLE_FACTOR_TWO;
    encodeInfo_.comp_info[INDEX_ONE].h_samp_factor = SAMPLE_FACTOR_ONE;
    encodeInfo_.comp_info[INDEX_ONE].v_samp_factor = SAMPLE_FACTOR_ONE;
    encodeInfo_.comp_info[INDEX_TWO].h_samp_factor = SAMPLE_FACTOR_ONE;
    encodeInfo_.comp_info[INDEX_TWO].v_samp_factor = SAMPLE_FACTOR_ONE;
}

uint32_t JpegEncoder::Yuv420spEncoder(const uint8_t *data)
{
    SetYuv420spExtraConfig();
    jpeg_start_compress(&encodeInfo_, TRUE);
    JSAMPROW y[Y_SAMPLE_ROW];
    JSAMPROW u[UV_SAMPLE_ROW];
    JSAMPROW v[UV_SAMPLE_ROW];
    JSAMPARRAY planes[COMPONENT_NUM_YUV420SP]{ y, u, v };
    uint32_t width = encodeInfo_.image_width;
    uint32_t height = encodeInfo_.image_height;
    uint32_t yPlaneSize = width * height;
    uint8_t *yPlane = const_cast<uint8_t *>(data);
    uint8_t *uvPlane = const_cast<uint8_t *>(data + yPlaneSize);
    auto uPlane = std::make_unique<uint8_t[]>((width >> SHIFT_MASK) * UV_SAMPLE_ROW);
    if (uPlane == nullptr) {
        HiLog::Error(LABEL, "allocate uPlane memory failed.");
        return ERR_IMAGE_MALLOC_ABNORMAL;
    }
    auto vPlane = std::make_unique<uint8_t[]>((width >> SHIFT_MASK) * UV_SAMPLE_ROW);
    if (vPlane == nullptr) {
        HiLog::Error(LABEL, "allocate vPlane memory failed.");
        return ERR_IMAGE_MALLOC_ABNORMAL;
    }
    while (encodeInfo_.next_scanline < height) {
        Deinterweave(uvPlane, uPlane.get(), vPlane.get(), encodeInfo_.next_scanline, width, height);
        for (uint32_t i = 0; i < Y_SAMPLE_ROW; i++) {
            y[i] = yPlane + (encodeInfo_.next_scanline + i) * width;
            if ((i & SHIFT_MASK) == 0) {
                uint32_t offset = (i >> SHIFT_MASK) * (width >> SHIFT_MASK);
                u[i >> SHIFT_MASK] = uPlane.get() + offset;
                v[i >> SHIFT_MASK] = vPlane.get() + offset;
            }
        }
        jpeg_write_raw_data(&encodeInfo_, planes, Y_SAMPLE_ROW);
    }
    jpeg_finish_compress(&encodeInfo_);
    return SUCCESS;
}

void JpegEncoder::Deinterweave(uint8_t *uvPlane, uint8_t *uPlane, uint8_t *vPlane, uint32_t curRow, uint32_t width,
                               uint32_t height)
{
    PixelFormat pixelFormat = pixelMaps_[0]->GetPixelFormat();
    uint32_t rowNum = (height - curRow) >> SHIFT_MASK;
    if (rowNum > UV_SAMPLE_ROW) {
        rowNum = UV_SAMPLE_ROW;
    }
    uint8_t indexZero = INDEX_ZERO;
    uint8_t indexOne = INDEX_ONE;
    if (pixelFormat != PixelFormat::NV12) {
        std::swap(indexZero, indexOne);
    }
    
    for (uint32_t row = 0; row < rowNum; row++) {
        uint32_t offset = ((curRow >> SHIFT_MASK) + row) * width;
        uint8_t *uv = uvPlane + offset;
        uint32_t col = width >> SHIFT_MASK;
        for (uint32_t i = 0; i < col; i++) {
            uint32_t index = row * col + i;
            uPlane[index] = uv[indexZero];
            vPlane[index] = uv[indexOne];
            uv += INDEX_TWO;
        }
    }
}

JpegEncoder::~JpegEncoder()
{
    jpeg_destroy_compress(&encodeInfo_);
    pixelMaps_.clear();
}
} // namespace ImagePlugin
} // namespace OHOS
