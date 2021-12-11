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

#include "emergency_utils.h"

#include <algorithm>
#include <sstream>

#include "shortnumberinfo.h"

#include "mmi_code_utils.h"
#include "module_service_utils.h"
#include "standardize_utils.h"

namespace OHOS {
namespace Telephony {
EmergencyUtils::EmergencyUtils() {}

bool EmergencyUtils::IsEmergencyCall(const std::string &phoneNum, int32_t slotId)
{
    if (phoneNum.empty()) {
        TELEPHONY_LOGI("IsEmergencyCall return, phoneNum is empty.");
        return false;
    }

    if (phoneNum.find('@') != std::string::npos || phoneNum.find("%40") != std::string::npos) {
        TELEPHONY_LOGI("IsEmergencyCall return, phoneNum is not emergency.");
        return false;
    }

    if (phoneNum.front() == '+') {
        TELEPHONY_LOGI("IsEmergencyCall return, phoneNum is not emergency.");
        return false;
    }
    return IsEmergencyCallProcessing(phoneNum, slotId);
}

bool EmergencyUtils::IsEmergencyCallProcessing(const std::string &formatString, int32_t slotId)
{
    std::vector<std::string> emergencyNumList = {
        "110", "120", "119", "122", "12395", "114", "112", "000", "911", "08", "118", "999",
    };
    if (std::find(emergencyNumList.begin(), emergencyNumList.end(), formatString) != emergencyNumList.end()) {
        TELEPHONY_LOGD("IsEmergencyCallProcessing, Complies with local configuration data.");
        return true;
    }
    ModuleServiceUtils dependDataObtain;
    std::string countryIsoCode = dependDataObtain.GetNetworkCountryCode(slotId);
    if (!countryIsoCode.empty()) {
        TELEPHONY_LOGD("IsEmergencyCallProcessing countryIsoCode is not empty");
        i18n::phonenumbers::ShortNumberInfo shortNumberInfo;
        transform(countryIsoCode.begin(), countryIsoCode.end(), countryIsoCode.begin(), ::toupper);
        return shortNumberInfo.IsEmergencyNumber(formatString, countryIsoCode);
    }
    return false;
}
} // namespace Telephony
} // namespace OHOS
