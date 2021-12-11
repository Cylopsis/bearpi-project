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

#ifndef TELEPHONY_CELLULAR_CALL_MMI_CODE_UTILS_H
#define TELEPHONY_CELLULAR_CALL_MMI_CODE_UTILS_H

#include <string>
#include "cellular_call_data_struct.h"

#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
// Man-Machine Interface
class MMICodeUtils {
public:
    /**
     * MMICodeUtils constructor
     */
    MMICodeUtils();

    /**
     * ~MMICodeUtils destructor
     */
    ~MMICodeUtils() = default;

    /**
     * IsNeedExecuteMmi
     *
     * 3GPP TS 22.030 V4.0.0 (2001-03)  6.5.2 Structure of the MMI
     * TS 24.080 [10]
     *
     * @param analyseString
     * @return Whether to execute the MMI
     */
    bool IsNeedExecuteMmi(const std::string &analyseString);

    /**
     * ExecuteMmiCode
     * @return bool
     */
    bool ExecuteMmiCode();

public:
    MMIData mmiData_;

private:
    /**
     * RegexMatchMmi
     *
     * 3GPP TS 22.030 V4.0.0 (2001-03)  6.5.2 Structure of the MMI
     * TS 24.080 [10]
     *
     * Regex Match Mmi Code
     *
     * @param analyseString
     * @return bool
     */
    bool RegexMatchMmi(const std::string &analyseString);

private:
    static constexpr HiviewDFX::HiLogLabel LOG_LABEL = {LOG_CORE, LOG_DOMAIN, "MMICodeUtils"};
};
} // namespace Telephony
} // namespace OHOS

#endif // TELEPHONY_CELLULAR_CALL_MMI_CODE_UTILS_H
