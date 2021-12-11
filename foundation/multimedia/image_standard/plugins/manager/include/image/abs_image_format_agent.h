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

#ifndef ABS_IMAGE_FORMAT_AGENT_H
#define ABS_IMAGE_FORMAT_AGENT_H

#include <string>
#include "image_plugin_type.h"
#include "plugin_service.h"

namespace OHOS {
namespace ImagePlugin {
class AbsImageFormatAgent {
public:
    // get the image encoded format supported by this class.
    virtual std::string GetFormatType() = 0;

    // get the header size of the encoded format.
    // it is used to determine the size of the image data that
    // needs to be passed to the function of CheckFormat().
    virtual uint32_t GetHeaderSize() = 0;

    // check if the image is in this encoded format, if it returns true.
    virtual bool CheckFormat(const void *headerData, uint32_t dataSize) = 0;

    // define multiple subservices for this interface.
    static constexpr uint16_t SERVICE_DEFAULT = 0;
};
} // namespace ImagePlugin
} // namespace OHOS

DECLARE_INTERFACE(OHOS::ImagePlugin::AbsImageFormatAgent, IMAGE_FORMAT_AGENT_IID)

#endif // ABS_IMAGE_FORMAT_AGENT_H
