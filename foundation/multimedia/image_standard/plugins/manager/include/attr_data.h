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

#ifndef ATTR_DATA_H
#define ATTR_DATA_H

#include <set>
#include <string>
#include "plugin_errors.h"

namespace OHOS {
namespace MultimediaPlugin {
enum class AttrDataType : int32_t {
    ATTR_DATA_NULL = 0,
    ATTR_DATA_BOOL,
    ATTR_DATA_UINT32,
    ATTR_DATA_STRING,
    ATTR_DATA_UINT32_SET,
    ATTR_DATA_STRING_SET,
    ATTR_DATA_UINT32_RANGE,
    ATTR_DATA_TYPE_INVALID
};

class AttrData final {
public:
    AttrData();
    explicit AttrData(bool value);
    explicit AttrData(uint32_t value);
    explicit AttrData(const std::string &value);
    explicit AttrData(std::string &&value);
    AttrData(uint32_t lowerBound, uint32_t upperBound);
    AttrData(const AttrData &data);
    AttrData(AttrData &&data) noexcept;
    ~AttrData();

    AttrData &operator=(const AttrData &data);
    AttrData &operator=(AttrData &&data) noexcept;

    void SetData(bool value);
    void SetData(uint32_t value);
    uint32_t SetData(const std::string &value);
    uint32_t SetData(std::string &&value);
    uint32_t SetData(uint32_t lowerBound, uint32_t upperBound);
    void ClearData();

    uint32_t InsertSet(uint32_t value);
    uint32_t InsertSet(const std::string &value);
    uint32_t InsertSet(std::string &&value);

    bool InRange(bool value) const;
    bool InRange(uint32_t value) const;
    bool InRange(const std::string &value) const;
    bool InRange(const AttrData &data) const;

    AttrDataType GetType() const;
    uint32_t GetMinValue(uint32_t &value) const;
    uint32_t GetMaxValue(uint32_t &value) const;
    uint32_t GetMinValue(const std::string *&value) const;
    uint32_t GetMaxValue(const std::string *&value) const;

    uint32_t GetValue(bool &value) const;
    uint32_t GetValue(uint32_t &value) const;
    uint32_t GetValue(std::string &value) const;
    uint32_t GetValue(const std::string *&value) const;

    static constexpr uint8_t RANGE_ARRAY_SIZE = 2;
    static constexpr uint8_t LOWER_BOUND_INDEX = 0;
    static constexpr uint8_t UPPER_BOUND_INDEX = 1;

private:
    uint32_t InitStringAttrData(const AttrData &data);
    uint32_t InitUint32SetAttrData(const AttrData &data);
    uint32_t InitStringSetAttrData(const AttrData &data);
    bool InRangeUint32Range(uint32_t value) const;
    bool InRange(const std::set<uint32_t> &uint32Set) const;
    bool InRange(const std::set<std::string> &stringSet) const;
    bool InRange(const uint32_t (&uint32Rang)[RANGE_ARRAY_SIZE]) const;

    AttrDataType type_;
    union AttrDataUnion {
        bool boolValue;
        uint32_t uint32Value;
        uint32_t uint32Rang[RANGE_ARRAY_SIZE];
        std::set<uint32_t> *uint32Set;
        std::string *stringValue;
        std::set<std::string> *stringSet;
    };
    AttrDataUnion value_;
};
} // namespace MultimediaPlugin
} // namespace OHOS

#endif // ATTR_DATA_H
