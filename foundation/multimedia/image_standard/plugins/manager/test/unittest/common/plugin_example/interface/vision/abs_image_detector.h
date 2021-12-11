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
#ifndef ABS_IMAGE_DETECT_H
#define ABS_IMAGE_DETECT_H

#include <vector>
#include <string>
#include "plugin_service.h"

namespace OHOS {
namespace PluginExample {
class AbsImageDetector {
public:
    virtual bool Prepare() = 0;
    virtual std::string Process() = 0;
    virtual ~AbsImageDetector() {}

    // define multiple subservices for this interface
    static constexpr uint16_t SERVICE_LABEL = 0;
    static constexpr uint16_t SERVICE_LANDMARK = 1;
    static constexpr uint16_t SERVICE_FACE = 2;
    static constexpr uint16_t SERVICE_TEXT = 3;
    static constexpr uint16_t SERVICE_FLOWER = 4;
};
} // namespace PluginExample
} // namespace OHOS

DECLARE_INTERFACE(OHOS::PluginExample::AbsImageDetector, PLUGIN_EXAMPLE_IID)

#endif // ABS_IMAGE_DETECT_H
