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

#include "standardize_utils.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
StandardizeUtils::StandardizeUtils() {}

std::string StandardizeUtils::RemoveSeparatorsPhoneNumber(const std::string &phoneString)
{
    std::string newString("");

    if (phoneString.empty()) {
        TELEPHONY_LOGE("RemoveSeparatorsPhoneNumber return, phoneStr is empty.");
        return newString;
    }

    for (uint32_t i = 0; i < phoneString.length(); i++) {
        char c = phoneString.at(i);
        if ((c >= '0' && c <= '9') || c == '*' || c == '#' || c == '+' || c == 'N' || c == ',' || c == ';') {
            newString = newString + c;
        }
    }

    return newString;
}
} // namespace Telephony
} // namespace OHOS
