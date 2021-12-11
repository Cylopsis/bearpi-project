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

#ifndef PRIORITY_SCHEME_H
#define PRIORITY_SCHEME_H

#include <string>

namespace OHOS {
namespace MultimediaPlugin {
enum class PriorityType : int32_t {
    PRIORITY_TYPE_NULL,
    PRIORITY_ORDER_BY_ATTR_ASCENDING,
    PRIORITY_ORDER_BY_ATTR_DESCENDING
};

class PriorityScheme final {
public:
    PriorityScheme() : type_(PriorityType::PRIORITY_TYPE_NULL) {}
    PriorityScheme(PriorityType type, const std::string &attrKey) : type_(type), attrKey_(attrKey) {}
    PriorityScheme(PriorityType type, std::string &&attrKey) : type_(type), attrKey_(std::move(attrKey)) {}
    ~PriorityScheme() = default;

    PriorityType GetPriorityType() const
    {
        return type_;
    }

    const std::string &GetAttrKey() const
    {
        return attrKey_;
    }

private:
    PriorityType type_;
    std::string attrKey_;
};
} // namespace MultimediaPlugin
} // namespace OHOS

#endif // PRIORITY_SCHEME_H
