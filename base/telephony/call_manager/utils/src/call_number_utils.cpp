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

#include <algorithm>

#include "call_number_utils.h"

#include "phonenumbers/phonenumber.pb.h"

#include "telephony_log_wrapper.h"

#include "call_manager_errors.h"
#include "cellular_call_ipc_interface_proxy.h"

namespace OHOS {
namespace Telephony {
CallNumberUtils::CallNumberUtils() : phoneUtils_(i18n::phonenumbers::PhoneNumberUtil::GetInstance()) {}

CallNumberUtils::~CallNumberUtils() {}

int32_t CallNumberUtils::FormatPhoneNumber(
    const std::string &phoneNumber, const std::string &countryCode, std::string &formatNumber)
{
    if (phoneNumber.empty()) {
        return CALL_MANAGER_PHONE_NUMBER_NULL;
    }
    if (phoneNumber.front() == '#' || phoneNumber.front() == '*') {
        formatNumber = phoneNumber;
        return TELEPHONY_SUCCESS;
    }
    std::string tmpCode = countryCode;
    transform(tmpCode.begin(), tmpCode.end(), tmpCode.begin(), ::toupper);
    i18n::phonenumbers::PhoneNumber parseResult;
    phoneUtils_->ParseAndKeepRawInput(phoneNumber, tmpCode, &parseResult);
    phoneUtils_->FormatInOriginalFormat(parseResult, tmpCode, &formatNumber);
    if (formatNumber.empty() || formatNumber == "0" || phoneNumber == formatNumber) {
        return CALL_MANAGER_FORMATTING_FAILED;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallNumberUtils::FormatPhoneNumberToE164(
    const std::string phoneNumber, const std::string countryCode, std::string &formatNumber)
{
    return FormatNumberBase(phoneNumber, countryCode, i18n::phonenumbers::PhoneNumberUtil::E164, formatNumber);
}

int32_t CallNumberUtils::FormatNumberBase(const std::string phoneNumber, std::string countryCode,
    const i18n::phonenumbers::PhoneNumberUtil::PhoneNumberFormat formatInfo, std::string &formatNumber)
{
    if (phoneNumber.empty()) {
        return CALL_MANAGER_PHONE_NUMBER_NULL;
    }
    transform(countryCode.begin(), countryCode.end(), countryCode.begin(), ::toupper);
    i18n::phonenumbers::PhoneNumber parseResult;
    phoneUtils_->Parse(phoneNumber, countryCode, &parseResult);
    if (phoneUtils_->IsValidNumber(parseResult)) {
        phoneUtils_->Format(parseResult, formatInfo, &formatNumber);
    }
    if (formatNumber.empty() || formatNumber == "0" || phoneNumber == formatNumber) {
        return CALL_MANAGER_FORMATTING_FAILED;
    }
    return TELEPHONY_SUCCESS;
}

bool CallNumberUtils::CheckNumberIsEmergency(
    const std::string &phoneNumber, const int32_t slotId, int32_t &errorCode)
{
    errorCode = TELEPHONY_SUCCESS;
    int isEcc = DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->IsUrgentCall(
        phoneNumber, slotId, errorCode);
    TELEPHONY_LOGD("CheckNumberIsEmergency  isEcc == %d, errorCode == %d", isEcc, errorCode);
    return (isEcc != 0);
}

bool CallNumberUtils::IsValidSlotId(int32_t slotId) const
{
    return slotId == defaultSlotId_;
}
} // namespace Telephony
} // namespace OHOS
