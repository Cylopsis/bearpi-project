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

#ifndef JSON_HELPER_H
#define JSON_HELPER_H

#include <string>
#include "json.hpp"

namespace OHOS {
namespace MultimediaPlugin {
class JsonHelper final {
public:
    static uint32_t CheckElementExistence(const nlohmann::json &jsonObject, const std::string &key);
    static uint32_t GetStringValue(const nlohmann::json &jsonString, std::string &value);
    static uint32_t GetStringValue(const nlohmann::json &jsonObject, const std::string &key, std::string &value);
    static uint32_t GetUint32Value(const nlohmann::json &jsonNum, uint32_t &value);
    static uint32_t GetUint32Value(const nlohmann::json &jsonObject, const std::string &key, uint32_t &value);
    static uint32_t GetUint16Value(const nlohmann::json &jsonObject, const std::string &key, uint16_t &value);
    static uint32_t GetArraySize(const nlohmann::json &jsonObject, const std::string &key, size_t &size);

private:
    static const nlohmann::json &GetJsonElement(const nlohmann::json &jsonObject,
                                                const std::string &key,
                                                uint32_t &errorCode);
    static void PrintElementMissingLog(const std::string &identifier, const std::string &key, uint32_t errorCode);
    static nlohmann::json nullJson_;
};
} // namespace MultimediaPlugin
} // namespace OHOS

#endif // JSON_HELPER_H