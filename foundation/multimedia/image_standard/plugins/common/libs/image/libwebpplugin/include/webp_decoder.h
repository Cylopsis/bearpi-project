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

#ifndef WEBP_DECODER_H
#define WEBP_DECODER_H

#include "abs_image_decoder.h"
#include "hilog/log.h"
#include "input_data_stream.h"
#include "log_tags.h"
#include "plugin_class_base.h"
#include "webp/decode.h"
#include "webp/demux.h"

namespace OHOS {
namespace ImagePlugin {
enum class WebpDecodingState : int32_t {
    UNDECIDED = 0,
    SOURCE_INITED = 1,
    BASE_INFO_PARSING = 2,
    BASE_INFO_PARSED = 3,
    IMAGE_DECODING = 4,
    IMAGE_ERROR = 5,
    IMAGE_PARTIAL = 6,
    IMAGE_DECODED = 7
};

class WebpDecoder : public AbsImageDecoder, public OHOS::MultimediaPlugin::PluginClassBase {
public:
    WebpDecoder();
    ~WebpDecoder() override;
    void SetSource(InputDataStream &sourceStream) override;
    void Reset() override;
    uint32_t SetDecodeOptions(uint32_t index, const PixelDecodeOptions &opts, PlImageInfo &info) override;
    uint32_t Decode(uint32_t index, DecodeContext &context) override;
    uint32_t PromoteIncrementalDecode(uint32_t index, ProgDecodeContext &context) override;
    uint32_t GetImageSize(uint32_t index, PlSize &size) override;

private:
    // private function
    DISALLOW_COPY_AND_MOVE(WebpDecoder);
    WEBP_CSP_MODE GetWebpDecodeMode(const PlPixelFormat &pixelFormat, bool premul);
    uint32_t ReadIncrementalHead();
    uint32_t DecodeHeader();
    bool AllocHeapBuffer(DecodeContext &context, bool isIncremental);
    void InitWebpOutput(const DecodeContext &context, WebPDecBuffer &output);
    bool PreDecodeProc(DecodeContext &context, WebPDecoderConfig &config, bool isIncremental);
    uint32_t DoCommonDecode(DecodeContext &context);
    uint32_t DoIncrementalDecode(ProgDecodeContext &context);
    void FinishOldDecompress();
    bool IsDataEnough();
    // private members
    InputDataStream *stream_ = nullptr;
    DataStreamBuffer dataBuffer_;
    PlSize webpSize_;
    size_t incrementSize_ = 0;   // current incremental data size
    size_t lastDecodeSize_ = 0;  // last decoded data size
    int32_t bytesPerPixel_ = 4;  // default four bytes for each pixel
    WEBP_CSP_MODE webpMode_ = MODE_RGBA;
    WebpDecodingState state_ = WebpDecodingState::UNDECIDED;
    PixelDecodeOptions opts_;
    PlPixelFormat outputFormat_ = PlPixelFormat::UNKNOWN;
};
} // namespace ImagePlugin
} // namespace OHOS

#endif // WEBP_DECODER_H
