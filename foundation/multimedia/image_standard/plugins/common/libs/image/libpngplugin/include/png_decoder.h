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

#ifndef PNG_DECODER_H
#define PNG_DECODER_H

#include "abs_image_decoder.h"
#include "hilog/log.h"
#include "input_data_stream.h"
#include "log_tags.h"
#include "nine_patch_listener.h"
#include "plugin_class_base.h"
#include "png.h"

namespace OHOS {
namespace ImagePlugin {
enum class PngDecodingState : int32_t {
    UNDECIDED = 0,
    SOURCE_INITED = 1,
    BASE_INFO_PARSING = 2,
    BASE_INFO_PARSED = 3,
    IMAGE_DECODING = 4,
    IMAGE_ERROR = 5,
    IMAGE_PARTIAL = 6,
    IMAGE_DECODED = 7
};

struct PngImageInfo {
    uint32_t width = 0;
    uint32_t height = 0;
    uint8_t bitDepth = 0;
    uint32_t rowDataSize = 0;
    int32_t numberPasses = 0;  // interlace is 7 otherwise is 1.
};

class PngDecoder : public AbsImageDecoder, public OHOS::MultimediaPlugin::PluginClassBase {
public:
    PngDecoder();
    ~PngDecoder() override;
    PngDecoder(const PngDecoder &) = delete;
    PngDecoder &operator=(const PngDecoder &) = delete;
    void SetSource(InputDataStream &sourceStream) override;
    void Reset() override;
    uint32_t SetDecodeOptions(uint32_t index, const PixelDecodeOptions &opts, PlImageInfo &info) override;
    uint32_t Decode(uint32_t index, DecodeContext &context) override;
    uint32_t PromoteIncrementalDecode(uint32_t index, ProgDecodeContext &context) override;
    uint32_t GetImageSize(uint32_t index, PlSize &size) override;
    bool HasProperty(std::string key) override;

private:
    uint32_t GetDecodeFormat(PlPixelFormat format, PlPixelFormat &outputFormat, PlAlphaType &alphaType);
    void ChooseFormat(PlPixelFormat format, PlPixelFormat &outputFormat, png_byte destType);
    static void PngErrorExit(png_structp pngPtr, png_const_charp message);
    static void PngWarning(png_structp pngPtr, png_const_charp message);
    static void PngWarningMessage(png_structp pngPtr, png_const_charp message);
    static void PngErrorMessage(png_structp pngPtr, png_const_charp message);
    // incremental private interface
    uint32_t PushCurrentToDecode(InputDataStream *stream);
    uint32_t IncrementalReadRows(InputDataStream *stream);
    uint32_t PushAllToDecode(InputDataStream *stream, size_t bufferSize, size_t length);
    static void GetAllRows(png_structp pngPtr, png_bytep row, png_uint_32 rowNum, int pass);
    static void GetInterlacedRows(png_structp pngPtr, png_bytep row, png_uint_32 rowNum, int pass);
    static int32_t ReadUserChunk(png_structp png_ptr, png_unknown_chunkp chunk);
    void SaveRows(png_bytep row, png_uint_32 rowNum);
    void SaveInterlacedRows(png_bytep row, png_uint_32 rowNum, int pass);
    uint32_t ReadIncrementalHead(InputDataStream *stream, PngImageInfo &info);
    bool GetImageInfo(PngImageInfo &info);
    bool IsChunk(const png_byte *chunk, const char *flag);
    uint32_t ProcessData(png_structp pngPtr, png_infop infoPtr, InputDataStream *sourceStream,
                         DataStreamBuffer streamData, size_t bufferSize, size_t totalSize);
    bool ConvertOriginalFormat(png_byte source, png_byte &destination);
    uint8_t *AllocOutputHeapBuffer(DecodeContext &context);
    uint32_t IncrementalRead(InputDataStream *stream, uint32_t desiredSize, DataStreamBuffer &outData);
    uint32_t DecodeHeader();
    uint32_t ConfigInfo(const PixelDecodeOptions &opts);
    uint32_t DoOneTimeDecode(DecodeContext &context);
    bool FinishOldDecompress();
    bool InitPnglib();
    uint32_t GetImageIdatSize(InputDataStream *stream);
    void DealNinePatch(const PixelDecodeOptions &opts);
    // local private parameter
    const std::string NINE_PATCH = "ninepatch";
    png_structp pngStructPtr_ = nullptr;
    png_infop pngInfoPtr_ = nullptr;
    InputDataStream *inputStreamPtr_ = nullptr;
    PngImageInfo pngImageInfo_;
    bool decodedIdat_ = false;
    size_t idatLength_ = 0;
    size_t incrementalLength_ = 0;
    uint8_t *pixelsData_ = nullptr;
    uint32_t outputRowsNum_ = 0;
    PngDecodingState state_ = PngDecodingState::UNDECIDED;
    uint32_t streamPosition_ = 0;  // may be changed by other decoders, record it and restore if needed.
    PlPixelFormat outputFormat_ = PlPixelFormat::UNKNOWN;
    PlAlphaType alphaType_ = PlAlphaType::IMAGE_ALPHA_TYPE_UNKNOWN;
    PixelDecodeOptions opts_;
    bool decodeHeadFlag_ = false;
    uint32_t firstRow_ = 0;
    uint32_t lastRow_ = 0;
    bool interlacedComplete_ = false;
    NinePatchListener ninePatch_;
};
} // namespace ImagePlugin
} // namespace OHOS

#endif // PNG_DECODER_H
