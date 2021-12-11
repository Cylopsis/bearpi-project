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

#ifndef ABS_IMAGE_ENCODER_H
#define ABS_IMAGE_ENCODER_H

#include "pixel_map.h"
#include "image_plugin_type.h"
#include "output_data_stream.h"
#include "plugin_service.h"

namespace OHOS {
namespace ImagePlugin {
struct PlEncodeOptions {
    uint8_t quality = 100;
    uint32_t numberHint = 1;
};

class AbsImageEncoder {
public:
    AbsImageEncoder() = default;
    virtual ~AbsImageEncoder() = default;
    virtual uint32_t StartEncode(OutputDataStream &outputStream, PlEncodeOptions &option) = 0;
    virtual uint32_t AddImage(Media::PixelMap &pixelMap) = 0;
    virtual uint32_t FinalizeEncode() = 0;

    // define multiple subservices for this interface
    static constexpr uint16_t SERVICE_DEFAULT = 0;
};
} // namespace ImagePlugin
} // namespace OHOS

DECLARE_INTERFACE(OHOS::ImagePlugin::AbsImageEncoder, IMAGE_ENCODER_IID)

#endif // ABS_IMAGE_ENCODER_H
