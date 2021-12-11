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

#ifndef JPEG_ENCODER_H
#define JPEG_ENCODER_H

#include <vector>
#include "abs_image_encoder.h"
#include "hilog/log.h"
#include "jpeg_utils.h"
#include "jpeglib.h"
#include "log_tags.h"
#include "plugin_class_base.h"

namespace OHOS {
namespace ImagePlugin {
class JpegEncoder : public AbsImageEncoder, public OHOS::MultimediaPlugin::PluginClassBase {
public:
    JpegEncoder();
    ~JpegEncoder() override;
    uint32_t StartEncode(OutputDataStream &outputStream, PlEncodeOptions &option) override;
    uint32_t AddImage(Media::PixelMap &pixelMap) override;
    uint32_t FinalizeEncode() override;

private:
    DISALLOW_COPY_AND_MOVE(JpegEncoder);
    J_COLOR_SPACE GetEncodeFormat(Media::PixelFormat format, int32_t &componentsNum);
    void Deinterweave(uint8_t *uvPlane, uint8_t *uPlane, uint8_t *vPlane, uint32_t curRow, uint32_t width,
                      uint32_t height);
    uint32_t SetCommonConfig();
    void SetYuv420spExtraConfig();
    uint32_t SequenceEncoder(const uint8_t *data);
    uint32_t Yuv420spEncoder(const uint8_t *data);
    static constexpr OHOS::HiviewDFX::HiLogLabel LABEL = { LOG_CORE, LOG_TAG_DOMAIN_ID_PLUGIN, "JpegEncoder" };
    jpeg_compress_struct encodeInfo_;
    JpegDstMgr dstMgr_;
    ErrorMgr jerr_;
    std::vector<Media::PixelMap *> pixelMaps_;
    PlEncodeOptions encodeOpts_;
};
} // namespace ImagePlugin
} // namespace OHOS

#endif // JPEG_ENCODER_H