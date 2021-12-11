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

#ifndef PLUGIN_COMMON_TYPE_H
#define PLUGIN_COMMON_TYPE_H

#include <map>
#include <string>
#include "attr_data.h"

namespace OHOS {
namespace MultimediaPlugin {
struct ClassInfo {
    std::string packageName;
    std::string className;
    uint16_t priority;
    std::map<std::string, AttrData> capabilities;
};

constexpr uint32_t UINT16_MAX_VALUE = 0xFFFFUL;
constexpr uint32_t UINT32_MAX_VALUE = 0xFFFFFFFFUL;
} // namespace MultimediaPlugin
} // namespace OHOS

#endif // PLUGIN_COMMON_TYPE_H
