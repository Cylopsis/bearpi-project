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

#ifndef PLUGIN_ERRORS_H
#define PLUGIN_ERRORS_H

#include "errors.h"
#include "modules.h"

namespace OHOS {
namespace MultimediaPlugin {
constexpr uint32_t BASE_PLUGIN_ERR_OFFSET = ErrCodeOffset(SUBSYS_MULTIMEDIA, MODULE_PLUGIN);

constexpr uint32_t SUCCESS = 0;                                           // Operation succeed
constexpr uint32_t ERR_GENERAL = BASE_PLUGIN_ERR_OFFSET;                  // General error
constexpr uint32_t ERR_INTERNAL = BASE_PLUGIN_ERR_OFFSET + 1;
constexpr uint32_t ERR_INVALID_PARAMETER = BASE_PLUGIN_ERR_OFFSET + 2;
constexpr uint32_t ERR_UNSUPPORTED = BASE_PLUGIN_ERR_OFFSET + 3;
constexpr uint32_t ERR_MATCHING_PLUGIN = BASE_PLUGIN_ERR_OFFSET + 4;
constexpr uint32_t ERR_INSTANCE_LIMIT = BASE_PLUGIN_ERR_OFFSET + 5;
constexpr uint32_t ERR_COMP_EQUAL = BASE_PLUGIN_ERR_OFFSET + 6;
constexpr uint32_t ERR_COMP_ERROR = BASE_PLUGIN_ERR_OFFSET + 7;
constexpr uint32_t ERR_COMP_LOWER = BASE_PLUGIN_ERR_OFFSET + 8;
constexpr uint32_t ERR_COMP_HIGHER = BASE_PLUGIN_ERR_OFFSET + 9;
constexpr uint32_t ERR_NO_TARGET = BASE_PLUGIN_ERR_OFFSET + 10;
constexpr uint32_t ERR_DATA_TYPE = BASE_PLUGIN_ERR_OFFSET + 11;
} // namespace MultimediaPlugin
} // namespace OHOS

#endif // PLUGIN_ERRORS_H
