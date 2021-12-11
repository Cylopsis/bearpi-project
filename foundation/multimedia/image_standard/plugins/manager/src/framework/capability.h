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

#ifndef CAPABILITY_H
#define CAPABILITY_H

#include <map>
#include <string>
#include "json.hpp"
#include "attr_data.h"
#include "plugin_errors.h"

namespace OHOS {
namespace MultimediaPlugin {
class Capability final {
public:
    Capability() = default;
    explicit Capability(const std::map<std::string, AttrData> &caps);
    explicit Capability(std::map<std::string, AttrData> &&caps);
    ~Capability() = default;
    uint32_t SetCapability(const nlohmann::json &capsInfo);
    bool IsCompatible(const std::map<std::string, AttrData> &caps) const;
    const AttrData *GetCapability(const std::string &key) const;
    const std::map<std::string, AttrData> &GetCapability() const;

private:
    uint32_t AnalyzeAttrData(const nlohmann::json &capInfo, AttrData &attrData);
    uint32_t AnalyzeBool(const nlohmann::json &capInfo, AttrData &attrData);
    uint32_t AnalyzeUint32(const nlohmann::json &capInfo, AttrData &attrData);
    uint32_t AnalyzeString(const nlohmann::json &capInfo, AttrData &attrData);
    uint32_t AnalyzeUint32Set(const nlohmann::json &capInfo, AttrData &attrData);
    uint32_t AnalyzeUint32Range(const nlohmann::json &capInfo, AttrData &attrData);
    uint32_t AnalyzeStringSet(const nlohmann::json &capInfo, AttrData &attrData);

    static constexpr uint32_t SET_MIN_VALUE_NUM = 1;
    static const std::string CAPABILITY_BOOL_TRUE;
    static const std::string CAPABILITY_BOOL_FALSE;
    static std::map<std::string, AttrDataType> typeMap_;
    using CapsMap = std::map<std::string, AttrData>;
    CapsMap caps_;
};
} // namespace MultimediaPlugin
} // namespace OHOS

#endif // CAPABILITY_H
