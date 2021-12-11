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

#ifndef ABS_IMAGE_DECOMPRESS_H
#define ABS_IMAGE_DECOMPRESS_H

#include "image_plugin_type.h"
#include "input_data_stream.h"
#include "plugin_service.h"
#include "media_errors.h"
#include "pixel_map.h"

namespace OHOS {
namespace ImagePlugin {
class AbsImageDecompressComponent {
public:

    AbsImageDecompressComponent() = default;
    virtual ~AbsImageDecompressComponent() = default;
    virtual uint32_t Decompress(void *decompressInfo, InputDataStream *stream, DecodeContext &context) = 0;
    // define multiple subservices for this interface
    static constexpr uint16_t SERVICE_DEFAULT = 0;
};
} // namespace ImagePlugin
} // namespace OHOS

DECLARE_INTERFACE(OHOS::ImagePlugin::AbsImageDecompressComponent, IMAGE_DECOMPRESS_COMP_IID)

#endif // ABS_IMAGE_DECOMPRESS_H
