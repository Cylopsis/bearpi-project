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

#ifndef HEIF_DECODER_H
#define HEIF_DECODER_H

#include <memory>
#include "abs_image_decoder.h"
#include "heif_decoder_wrapper.h"
#include "hilog/log.h"
#include "log_tags.h"
#include "nocopyable.h"
#include "plugin_class_base.h"

namespace OHOS {
namespace ImagePlugin {
class HeifDecoder : public AbsImageDecoder, public MultimediaPlugin::PluginClassBase {
public:
    HeifDecoder() = default;
    virtual ~HeifDecoder() override{};
    virtual void SetSource(InputDataStream &sourceStream) override;
    virtual void Reset() override;
    virtual uint32_t SetDecodeOptions(uint32_t index, const PixelDecodeOptions &opts, PlImageInfo &info) override;
    virtual uint32_t Decode(uint32_t index, DecodeContext &context) override;
    virtual uint32_t PromoteIncrementalDecode(uint32_t index, ProgDecodeContext &context) override;
    virtual uint32_t GetTopLevelImageNum(uint32_t &num) override;
    virtual uint32_t GetImageSize(uint32_t index, PlSize &size) override;

private:
    DISALLOW_COPY_AND_MOVE(HeifDecoder);
    bool AllocHeapBuffer(DecodeContext &context);
    bool IsHeifImageParaValid(PlSize heifSize, uint32_t bytesPerPixel);
    std::unique_ptr<HeifDecoderInterface> heifDecoderInterface_ = nullptr;
    PlSize heifSize_;
    int32_t bytesPerPixel_ = 0;
};
} // namespace ImagePlugin
} // namespace OHOS

#endif // HEIF_DECODER_H
