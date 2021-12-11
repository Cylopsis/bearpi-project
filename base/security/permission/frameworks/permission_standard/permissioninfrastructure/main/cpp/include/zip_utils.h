/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef ZIP_UTILS_H
#define ZIP_UTILS_H

#include <string>

namespace OHOS {
namespace Security {
namespace Permission {
class ZipUtils {
public:
    static constexpr int32_t OK = 0;

    static constexpr int32_t ERROR = 1;

    static int32_t CompressString(const std::string& inputStr, std::string& outputStr);

    static int32_t DecompressString(const std::string& inputStr, std::string& outputStr);

private:
    static constexpr int32_t BUFFER_SIZE = 8192;
};
} // namespace Permission
} // namespace Security
} // namespace OHOS
#endif // ZIP_UTILS_H
