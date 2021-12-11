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

#include "mmi_code_utils.h"

#include <regex>
#include "cellular_call_supplement.h"

#include "securec.h"

#include "standardize_utils.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
MMICodeUtils::MMICodeUtils()
{
    TELEPHONY_LOGD("MMICodeUtils entry.");
}

constexpr unsigned long long operator"" _hash(char const *p, size_t s)
{
    return StandardizeUtils::HashCompileTime(p);
}

bool MMICodeUtils::IsNeedExecuteMmi(const std::string &analyseString)
{
    if (analyseString.empty()) {
        TELEPHONY_LOGE("IsNeedExecuteMmi return, analyseString is empty.");
        return false;
    }
    return RegexMatchMmi(analyseString);
}

bool MMICodeUtils::ExecuteMmiCode()
{
    CellularCallSupplement supplement;

    /**
     * "30" Processing caller ID
     * "31" Processing calling number display
     * "21" Deal with unconditional transfer
     * "61" Handling no answer transfer
     * "62" Handling no signal transfer
     * "67" Deal with busy transfer
     * "002" Process all transfers
     * "004" Handle transfer under all conditions
     * "33" Processing limits all outgoing calls
     * "330" Processing all restrictions
     * "331" Processing limits all international calls
     * "332" Handling international outgoing calls belonging to foreign countries when roaming is
     * restricted
     * "35" Processing limits all incoming calls
     * "351" Handle all incoming calls when roaming is restricted
     * "03" Processing network password
     * "43" Handling call waiting
     */
    if (!mmiData_.serviceCode.empty()) {
        switch (StandardizeUtils::Hash_(mmiData_.serviceCode.c_str())) {
            // 3GPP TS 22.030 V4.0.0 (2001-03) 6.5.6.2  Calling Line Identification Presentation (CLIP)
            // 3GPP TS 22.030 V4.0.0 (2001-03) Annex B (normative):Codes for defined Supplementary Services
            case "30"_hash:
                supplement.GetClip(mmiData_);
                return true;

            // 3GPP TS 22.081 V4.0.0 (2001-03) 2 Calling Line Identification Restriction (CLIR)
            // 3GPP TS 22.030 V4.0.0 (2001-03) Annex B (normative):Codes for defined Supplementary Services
            case "31"_hash:
                supplement.GetClir(mmiData_);
                return true;

            // 3GPP TS 22.081 V4.0.0 (2001-03)
            // 3GPP TS 22.030 V4.0.0 (2001-03) Annex B (normative):Codes for defined Supplementary Services
            // 3GPP TS 24.082 V4.0.0 (2001-03) 1 Call Forwarding Unconditional (CFU)
            // 3GPP TS 24.082 V4.0.0 (2001-03) 2 Call Forwarding on mobile subscriber Busy (CFB)
            // 3GPP TS 24.082 V4.0.0 (2001-03) 3 Call Forwarding on No Reply (CFNRy)
            // 3GPP TS 24.082 V4.0.0 (2001-03) 4 Call Forwarding on mobile subscriber Not Reachable (CFNRc)
            case "21"_hash:
            case "61"_hash:
            case "62"_hash:
            case "67"_hash:
            case "002"_hash:
            case "004"_hash:
                supplement.DealCallTransfer(mmiData_);
                return true;

            // 27007-430_2001 7.4	Facility lock +CLCK
            // 3GPP TS 22.088 [6] 1	Barring of outgoing calls
            // 3GPP TS 22.088 [6] 2	Barring of incoming calls
            /* 3GPP TS 22.030 V4.0.0 (2001-03) Annex B (normative):Codes for defined Supplementary Services
             * BAOC	                33
             * BAOIC	            331
             * BAOIC exc home	    332
             * BAIC	                35
             * BAIC roaming	        351
             * all Barring Serv.	330
             * Outg. Barr. Serv.	333
             * Inc. Barr. Serv.	    353
             */
            case "33"_hash:
            case "330"_hash:
            case "331"_hash:
            case "332"_hash:
            case "35"_hash:
            case "351"_hash:
                supplement.DealCallRestriction(mmiData_);
                return true;

            // 3GPP TS 22.030 V4.0.0 (2001-03)  6.5.4 Registration of new password
            case "03"_hash:
                return true;

            // 27007-430_2001 7.12	Call waiting +CCWA
            // 3GPP TS 22.083 [5] 1	Call waiting (CW)
            // 3GPP TS 22.030 V4.0.0 (2001-03) Annex B (normative):Codes for defined Supplementary Services
            case "43"_hash:
                supplement.DealCallWaiting(mmiData_);
                return true;
            default:
                TELEPHONY_LOGI("ExecuteMmiCode return, default case, need check serviceCode.");
                break;
        }
    }
    return true;
}

bool MMICodeUtils::RegexMatchMmi(const std::string &analyseString)
{
    std::regex pattern(
        "((\\*|#|\\*#|\\*\\*|##)(\\d{2,3})(\\*([^*#]*)(\\*([^*#]*)(\\*([^*#]*)(\\*([^*#]*))?)?)?)?#)(.*)");
    std::smatch results;
    if (regex_match(analyseString, results, pattern)) {
        TELEPHONY_LOGI("MMICodeUtils::RegexMatchMmi, regex_match ture");

        /**
         * The following sequence of functions shall be used for the control of Supplementary Services:
         *  SELECT:	Entry of the procedure information (may be a digit or a sequence of characters).
         *  SEND: Transmission of the information to the network.
         *  INDICATION:	Call progress indications.
         */
        int32_t fullString = 1;
        int32_t action = 2;
        // 3GPP TS 22.030 V4.0.0 (2001-03)  6.5.2 Structure of the MMI
        // This structure consists of the following parts:
        //     Service Code, SC( (2 or 3 digits)
        //     Supplementary Information, SI (variable length).
        int32_t serviceCode = 3;
        int32_t sia = 5;
        int32_t sib = 7;
        int32_t sic = 9;
        int32_t pwdConfirm = 11;
        int32_t dialingNumber = 12;
        mmiData_.fullString = results.str(fullString);
        mmiData_.actionString = results.str(action);
        mmiData_.serviceCode = results.str(serviceCode);
        mmiData_.serviceInfoA = results.str(sia);
        mmiData_.serviceInfoB = results.str(sib);
        mmiData_.serviceInfoC = results.str(sic);
        mmiData_.pwdString = results.str(pwdConfirm);
        mmiData_.dialString = results.str(dialingNumber);

        /* 3GPP TS 22.030 V4.0.0 (2001-03)  6.5.2 Structure of the MMI
         * The procedure always starts with *, #, **, ## or *# and is finished by #.
         * Each part within the procedure is separated by *.
         */
        if (!mmiData_.dialString.empty() && mmiData_.dialString.back() == '#' && analyseString.back() == '#') {
            mmiData_.fullString = analyseString;
        }
        return true;
    }
    return false;
}
} // namespace Telephony
} // namespace OHOS
