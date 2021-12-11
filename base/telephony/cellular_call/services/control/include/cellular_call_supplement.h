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

#ifndef CELLULAR_CALL_SUPPLEMENT_H
#define CELLULAR_CALL_SUPPLEMENT_H

#include <string>
#include <hril_types.h>
#include <hril_call_parcel.h>
#include "cellular_call_data_struct.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
class CellularCallSupplement {
public:
    /**
     * Deal Clip
     *
     * 3GPP TS 22.030 V4.0.0 (2001-03)  6.5.6.2	Calling Line Identification Presentation (CLIP)
     * The CLIP Supplementary Service is defined in GSM 02.81[12]
     * 3GPP TS 22.030 V4.0.0 (2001-03) Annex B (normative):Codes for defined Supplementary Services
     *
     * @param MMIData
     */
    void GetClip(const MMIData &mmiData);

    /**
     * Deal Clir
     *
     * 3GPP TS 22.081 V4.0.0 (2001-03) 2 Calling Line Identification Restriction (CLIR)
     * 3GPP TS 22.030 V4.0.0 (2001-03) Annex B (normative):Codes for defined Supplementary Services
     *
     * @param MMIData
     */
    void GetClir(const MMIData &mmiData);

    /**
     * Deal Call Transfer
     *
     * 3GPP TS 22.030 V4.0.0 (2001-03)
     * 3GPP TS 22.030 V4.0.0 (2001-03) Annex B (normative):Codes for defined Supplementary Services
     *
     * @param MMIData
     */
    void DealCallTransfer(const MMIData &mmiData);

    /**
     * Set Call Transfer
     *
     * 27007-430_2001 7.11 Call forwarding number and conditions +CCFC
     * 3GPP TS 22.082 [4]
     * Control of the call forwarding supplementary service
     *
     * @param CallTransferInfo
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t SetCallTransfer(const CallTransferInfo &cTInfo);

    /**
     * Inquire Call Transfer
     *
     * 27007-430_2001 7.11 Call forwarding number and conditions +CCFC
     * 3GPP TS 22.082 [4]
     * Control of the call forwarding supplementary service
     *
     * @param CallTransferType
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t InquireCallTransfer(CallTransferType type);

    /**
     * Deal Call Restriction
     *
     * 3GPP TS 22.088 [6] 1	Barring of outgoing calls
     * 3GPP TS 22.088 [6] 2	Barring of incoming calls
     *
     * @param MMIData
     */
    void DealCallRestriction(const MMIData &mmiData);

    /**
     * Set Call Restriction
     *
     * 27007-430_2001 7.4	Facility lock +CLCK
     * 3GPP TS 22.088 [6] 1	Barring of outgoing calls
     * 3GPP TS 22.088 [6] 2	Barring of incoming calls
     *
     * Control of the call barring supplementary service
     *
     * @param CallRestrictionInfo
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t SetCallRestriction(const CallRestrictionInfo &cRInfo);

    /**
     * Inquire Call Restriction
     *
     * 27007-430_2001 7.4	Facility lock +CLCK
     * 3GPP TS 22.088 [6] 1	Barring of outgoing calls
     * 3GPP TS 22.088 [6] 2	Barring of incoming calls
     *
     * Control of the call barring supplementary service
     *
     * @param CallRestrictionType
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t InquireCallRestriction(CallRestrictionType facType);

    /**
     * Deal Call Waiting
     *
     * 27007-430_2001 7.12	Call waiting +CCWA
     * 3GPP TS 22.083 [5] 1	Call waiting (CW)
     *
     * Deal Call Waiting
     * @param MMIData
     */
    void DealCallWaiting(const MMIData &mmiData);

    /**
     * Set Call Waiting
     *
     * 27007-430_2001 7.12	Call waiting +CCWA
     * 3GPP TS 22.083 [5] 1	Call waiting (CW)
     *
     * Control of the Call Waiting supplementary service
     *
     * @param activate
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t SetCallWaiting(bool activate);

    /**
     * Inquire Call Waiting
     *
     * 27007-430_2001 7.12	Call waiting +CCWA
     * 3GPP TS 22.083 [5] 1	Call waiting (CW)
     *
     * Control of the Call Waiting supplementary service
     *
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t InquireCallWaiting();

    /**
     * Event Deal clip Mmi
     *
     * 3GPP TS 27.007 V3.9.0 (2001-06) 7.6	Calling line identification presentation +CLIP
     *
     * @param GetClipResult
     */
    void EventGetClip(GetClipResult &getClipResult);

    /**
     * Event Deal Clir Mmi
     *
     * 3GPP TS 27.007 V3.9.0 (2001-06) 7.7	Calling line identification restriction +CLIR
     *
     * @param GetClirResult
     */
    void EventGetClir(GetClirResult &getClirResult);

    /**
     * Event set Clir Mmi
     *
     * 3GPP TS 27.007 V3.9.0 (2001-06) 7.7	Calling line identification restriction +CLIR
     *
     * @param HRilRadioResponseInfo
     */
    void EventSetClir(HRilRadioResponseInfo &info);

    /**
     * GetCallRestriction result
     *
     * 3GPP TS 27.007 V3.9.0 (2001-06) 7.4	Facility lock +CLCK
     *
     * @param CallRestrictionResult
     */
    void EventGetCallRestriction(const CallRestrictionResult &result);

    /**
     * SetCallRestriction result
     *
     * 3GPP TS 27.007 V3.9.0 (2001-06) 7.4	Facility lock +CLCK
     *
     * @param HRilRadioResponseInfo
     */
    void EventSetCallRestriction(HRilRadioResponseInfo &info);

    /**
     * Call Waiting result
     *
     * 3GPP TS 27.007 V3.9.0 (2001-06) 7.12	Call waiting +CCWA
     * status: 0	not active  1	active
     *
     * @param CallWaitResult waitingInfo
     */
    void EventInquireCallWait(CallWaitResult &waitingInfo);

    /**
     * Event Set CallWait
     *
     * 3GPP TS 27.007 V3.9.0 (2001-06) 7.12	Call waiting +CCWA
     *
     * @param HRilRadioResponseInfo
     */
    void EventSetCallWait(HRilRadioResponseInfo &responseInfo);

    /**
     * Event Inquire Call Transfer
     *
     * 3GPP TS 27.007 V3.9.0 (2001-06) 7.11	Call forwarding number and conditions +CCFC
     *
     * @param CallForwardQueryResult
     */
    void EventInquireCallTransfer(CallForwardQueryResult &queryResult);

    /**
     * Event Set Call Transfer
     *
     * 3GPP TS 27.007 V3.9.0 (2001-06) 7.11	Call forwarding number and conditions +CCFC
     *
     * @param HRilRadioResponseInfo
     */
    void EventSetCallTransfer(HRilRadioResponseInfo &responseInfo);

private:
    /**
     * Obtain ServiceCode
     *
     * @param serviceInfoB
     * @return
     */
    int ObtainServiceCode(const std::string &serviceInfoB);

    /**
     * Obtain Cause
     *
     * 3GPP TS 22.030 V4.0.0 (2001-03) Annex B (normative): Codes for defined Supplementary Services
     *
     * @param basicString
     * @return
     */
    int ObtainCause(const std::string &serviceInfoC);

    /**
     * Obtain Barring Installation
     *
     * 27007-430_2001 7.4	Facility lock +CLCK
     * 3GPP TS 22.088 [6] 1	Barring of outgoing calls
     * 3GPP TS 22.088 [6] 2	Barring of incoming calls
     *
     * @param serviceInfoC
     * @return Installation
     */
    std::string ObtainBarringInstallation(const std::string &serviceInfoC);

    /**
     * Is Phone Gsm Type
     * @return bool
     */
    bool PhoneTypeGsmOrNot();

private:
    const int32_t ACTIVATE_ACTION = 1;
    const int32_t DEACTIVATE_ACTION = 2;
    static constexpr HiviewDFX::HiLogLabel LOG_LABEL = {LOG_CORE, LOG_DOMAIN, "CellularCallSupplement"};
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_CALL_SUPPLEMENT_H
