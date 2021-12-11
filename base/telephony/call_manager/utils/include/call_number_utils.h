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

#ifndef CALL_NUMBER_UTILS_H
#define CALL_NUMBER_UTILS_H

#include "pac_map.h"
#include "singleton.h"
#include "phonenumberutil.h"

#include "common_type.h"

namespace OHOS {
namespace Telephony {
class CallNumberUtils {
    DECLARE_DELAYED_SINGLETON(CallNumberUtils)
public:
    int32_t FormatPhoneNumber(
        const std::string &phoneNumber, const std::string &countryCode, std::string &formatNumber);
    int32_t FormatPhoneNumberToE164(
        const std::string phoneNumber, const std::string countryCode, std::string &formatNumber);
    int32_t FormatNumberBase(const std::string phoneNumber, std::string countryCode,
        const i18n::phonenumbers::PhoneNumberUtil::PhoneNumberFormat formatInfo, std::string &formatNumber);
    bool CheckNumberIsEmergency(const std::string &phoneNumber, const int32_t slotId, int32_t &errorCode);
    bool IsValidSlotId(int32_t slotId) const;

private:
    i18n::phonenumbers::PhoneNumberUtil *phoneUtils_;
    int32_t defaultSlotId_ = 0;
};
} // namespace Telephony
} // namespace OHOS

#endif // CALL_NUMBER_UTILS_H