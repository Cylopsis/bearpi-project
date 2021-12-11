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

#include "cellular_call_supplement.h"

#include <securec.h>
#include "cellular_call_register.h"
#include "cellular_call_types.h"
#include "module_service_utils.h"
#include "standardize_utils.h"
#include "supplement_request.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
constexpr unsigned long long operator"" _hash(char const *p, size_t s)
{
    return StandardizeUtils::HashCompileTime(p);
}

void CellularCallSupplement::GetClip(const MMIData &mmiData)
{
    const std::string interrogate = "*#";
    if (!mmiData.actionString.empty() && mmiData.actionString == interrogate) {
        SupplementRequest supplementRequest;
        supplementRequest.InquireClipRequest();
    }
}

void CellularCallSupplement::GetClir(const MMIData &mmiData)
{
    const std::string interrogate = "*#";
    const std::string activate = "*";
    const std::string deactivate = "#";
    if (mmiData.actionString.empty()) {
        TELEPHONY_LOGW("GetClir return, actionString is empty!");
        return;
    }
    if (mmiData.actionString == activate) {
        SupplementRequest supplementRequest;
        supplementRequest.SetClirActivateRequest(ACTIVATE_ACTION);
    } else if (mmiData.actionString == deactivate) {
        SupplementRequest supplementRequest;
        supplementRequest.SetClirDeactivateRequest(DEACTIVATE_ACTION);
    } else if (mmiData.actionString == interrogate) {
        SupplementRequest supplementRequest;
        supplementRequest.InquireClirRequest();
    }
}

void CellularCallSupplement::DealCallTransfer(const MMIData &mmiData)
{
    const std::string interrogate = "*#";
    SupplementRequest supplementRequest;
    int serviceCode = ObtainServiceCode(mmiData.serviceInfoB);
    int cause = ObtainCause(mmiData.serviceCode);
    if (!mmiData.actionString.empty() && mmiData.actionString == interrogate) {
        supplementRequest.InquireCallTransferRequest(cause);
        return;
    }
    const std::string activate = "*";
    const std::string deactivate = "#";
    const std::string regis = "**";
    const std::string erasure = "##";
    std::string phoneNumber = mmiData.serviceInfoA;
    int callForwardAction;
    if (mmiData.actionString.empty()) {
        TELEPHONY_LOGW("DealCallTransfer return, actionString is empty!");
        return;
    }

    // 3GPP TS 24.082 V4.0.0 (2001-03) 1 Call Forwarding Unconditional (CFU)
    // 3GPP TS 24.082 V4.0.0 (2001-03) 2 Call Forwarding on mobile subscriber Busy (CFB)
    // 3GPP TS 24.082 V4.0.0 (2001-03) 3 Call Forwarding on No Reply (CFNRy)
    // 3GPP TS 24.082 V4.0.0 (2001-03) 4 Call Forwarding on mobile subscriber Not Reachable (CFNRc)
    if (mmiData.actionString == activate) {
        if (phoneNumber.empty()) {
            callForwardAction = CallTransferSettingType::ENABLE;
        } else {
            callForwardAction = CallTransferSettingType::REGISTRATION;
        }
    } else if (mmiData.actionString == deactivate) {
        callForwardAction = CallTransferSettingType::DISABLE;
    } else if (mmiData.actionString == regis) {
        callForwardAction = CallTransferSettingType::REGISTRATION;
    } else if (mmiData.actionString == erasure) {
        callForwardAction = CallTransferSettingType::ERASURE;
    } else {
        callForwardAction = TELEPHONY_ERROR;
    }
    supplementRequest.SetCallTransferRequest(callForwardAction, cause, phoneNumber, serviceCode);
}

int CellularCallSupplement::ObtainServiceCode(const std::string &serviceInfoB)
{
    if (serviceInfoB.empty()) {
        TELEPHONY_LOGW("ObtainServiceCode return, serviceInfoB is empty!");
        return NONE;
    }
    int intServiceInfoB = atoi(serviceInfoB.c_str());
    switch (intServiceInfoB) {
        case ALL_TELE_SERVICES:
            return SHORT_MESSAGE_SERVICE + FAX + VOICE;
        case TELE_SERVICES:
            return VOICE;
        case ALL_DATA_TELE_SERVICES:
            return SHORT_MESSAGE_SERVICE + FAX;
        case FACSIMILE_SERVICES:
            return FAX;
        case SHORT_MESSAGE_SERVICES:
            return SHORT_MESSAGE_SERVICE;
        case ALL_TELE_SERVICES_EXCEPT_SMS:
            return FAX + VOICE;
        case ALL_BEARER_SERVICES:
            return DATA_CIRCUIT_ASYNC + DATA_CIRCUIT_SYNC;
        case ALL_ASYNC_SERVICES:
            return DEDICATED_PAD_ACCESS + DATA_CIRCUIT_ASYNC;
        case ALL_SYNC_SERVICES:
            return DEDICATED_PACKET_ACCESS + DATA_CIRCUIT_SYNC;
        case ALL_DATA_CIRCUIT_SYNC:
            return DATA_CIRCUIT_SYNC;
        case ALL_DATA_CIRCUIT_ASYNC:
            return DATA_CIRCUIT_ASYNC;
        case ALL_GPRS_BEARER_SERVICES:
            return DEDICATED_PACKET_ACCESS;
        default:
            return NONE;
    }
}

int CellularCallSupplement::ObtainCause(const std::string &actionStr)
{
    if (actionStr.empty()) {
        TELEPHONY_LOGW("ObtainCause return, actionStr is empty!");
        return TELEPHONY_ERROR;
    }

    /*
     * 3GPP TS 22.030 V4.0.0 (2001-03) Annex B (normative): Codes for defined Supplementary Services
     * CFU	                21
     * CF Busy	            67
     * CF No Reply	        61
     * CF Not Reachable 	62
     * all CF		        002
     * all conditional CF	004
     */
    switch (StandardizeUtils::Hash_(actionStr.c_str())) {
        case "21"_hash:
            return UNCONDITIONAL;
        case "67"_hash:
            return MOBILE_BUSY;
        case "62"_hash:
            return NOT_REACHABLE;
        case "61"_hash:
            return NO_REPLY;
        default:
            return TELEPHONY_ERROR;
    }
}

void CellularCallSupplement::DealCallRestriction(const MMIData &mmiData)
{
    std::string infoA = mmiData.serviceInfoA;
    std::string facType = ObtainBarringInstallation(mmiData.serviceInfoC);
    const std::string interrogate = "*#";
    const std::string activate = "*";
    const std::string deactivate = "#";
    if (mmiData.actionString.empty()) {
        TELEPHONY_LOGW("DealCallRestriction return, actionString is empty!");
        return;
    }
    SupplementRequest supplementRequest;
    if (mmiData.actionString == interrogate) {
        supplementRequest.InquireCallRestriction(facType);
    } else if (mmiData.actionString == activate || mmiData.actionString == deactivate) {
        supplementRequest.SetCallRestriction(facType, mmiData.actionString == activate, infoA);
    }
}

std::string CellularCallSupplement::ObtainBarringInstallation(const std::string &serviceInfoC)
{
    if (serviceInfoC.empty()) {
        TELEPHONY_LOGW("ObtainBarringInstallation return, serviceInfoC is empty!");
        return std::string();
    }

    // 27007-430_2001 7.4	Facility lock +CLCK
    switch (StandardizeUtils::Hash_(serviceInfoC.c_str())) {
        case "33"_hash:
            // "AO"	BAOC (Barr All Outgoing Calls) (refer 3GPP TS 22.088 [6] clause 1)
            return "AO";
        case "331"_hash:
            // "OI"	BOIC (Barr Outgoing International Calls) (refer 3GPP TS 22.088 [6] clause 1)
            return "OI";
        case "332"_hash:
            // "OX"	BOIC exHC (Barr Outgoing International Calls except to Home Country)
            // (refer 3GPP TS 22.088 [6] clause 1)
            return "OX";
        case "351"_hash:
            // "IR"	BIC Roam (Barr Incoming Calls when Roaming outside the home country)
            // (refer 3GPP TS 22.088 [6] clause 2)
            return "IR";
        case "35"_hash:
            // "AI"	BAIC (Barr All Incoming Calls) (refer 3GPP TS 22.088 [6] clause 2)
            return "AI";
        case "330"_hash:
            // "AB"	All Barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
            return "AB";
        case "333"_hash:
            // "AG"	All outGoing barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
            return "AG";
        case "353"_hash:
            // "AC"	All inComing barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
            return "AC";
        default:
            return std::string();
    }
}

void CellularCallSupplement::DealCallWaiting(const MMIData &mmiData)
{
    if (mmiData.actionString.empty()) {
        TELEPHONY_LOGW("DealCallWaiting return, actionString is empty!");
        return;
    }
    const std::string activate = "*";
    const std::string deactivate = "#";
    const std::string interrogate = "*#";
    SupplementRequest supplementRequest;
    if (mmiData.actionString == activate || mmiData.actionString == deactivate) {
        supplementRequest.SetCallWaitingRequest(mmiData.actionString == activate);
    } else if (mmiData.actionString == interrogate) {
        supplementRequest.InquireCallWaitingRequest();
    }
}

void CellularCallSupplement::EventInquireCallWait(CallWaitResult &waitingInfo)
{
    CallWaitResponse callWaitResponse;
    callWaitResponse.result = waitingInfo.result;

    /**
     * <n> (sets/shows the result code presentation status in the TA):
     * 0	disable
     * 1	enable
     */
    callWaitResponse.status = waitingInfo.status;
    callWaitResponse.classCw = waitingInfo.classCw;
    if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("EventInquireCallWait return, GetInstance is nullptr.");
        return;
    }
    DelayedSingleton<CellularCallRegister>::GetInstance()->ReportGetWaitingResult(callWaitResponse);
}

void CellularCallSupplement::EventSetCallWait(HRilRadioResponseInfo &responseInfo)
{
    CallWaitResponse callWaitResponse;
    callWaitResponse.result = static_cast<int32_t>(responseInfo.error);
    if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("EventSetCallWait return, GetInstance is nullptr.");
        return;
    }
    DelayedSingleton<CellularCallRegister>::GetInstance()->ReportSetWaitingResult(callWaitResponse);
}

void CellularCallSupplement::EventInquireCallTransfer(CallForwardQueryResult &queryResult)
{
    if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("EventInquireCallTransfer return, GetInstance is nullptr.");
        return;
    }

    // 3GPP TS 27.007 V3.9.0 (2001-06) 7.11	Call forwarding number and conditions +CCFC
    CallTransferResponse callTransferResponse;
    callTransferResponse.result = queryResult.result;

    /*
     * <status>:0	not active    1	  active
     * */
    callTransferResponse.status = queryResult.status;

    /*
     * <classx> is a sum of integers each representing a class of information (default 7):
     * 1	voice (telephony)
     * 2	data (refers to all bearer services)
     * 4	fax (facsimile services)
     * 8	short message service
     * 16	data circuit sync
     * 32	data circuit async
     * 64	dedicated packet access
     * 128	dedicated PAD access
     */
    callTransferResponse.classx = queryResult.classx;
    // <number>: string type phone number of forwarding address in format specified by <type>
    callTransferResponse.number = queryResult.number;
    // <type>: type of address octet in integer format (refer GSM 04.08 [8] subclause 10.5.4.7);
    // default 145 when dialling string includes international access code character "+", otherwise 129
    callTransferResponse.type = queryResult.type;

    DelayedSingleton<CellularCallRegister>::GetInstance()->ReportGetTransferResult(callTransferResponse);
}

void CellularCallSupplement::EventSetCallTransfer(HRilRadioResponseInfo &responseInfo)
{
    if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("EventSetCallTransfer return, GetInstance is nullptr.");
        return;
    }
    DelayedSingleton<CellularCallRegister>::GetInstance()->ReportSetTransferResult((int32_t)responseInfo.error);
}

void CellularCallSupplement::EventGetCallRestriction(const CallRestrictionResult &result)
{
    if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("EventGetCallRestriction return, GetInstance is nullptr.");
        return;
    }
    CallRestrictionResponse callRestrictionResponse;
    callRestrictionResponse.result = result.result;

    /*
     * <status>:0	not active    1	  active
     */
    callRestrictionResponse.status = result.status;
    callRestrictionResponse.classCw = result.classCw;
    DelayedSingleton<CellularCallRegister>::GetInstance()->ReportGetRestrictionResult(callRestrictionResponse);
}

void CellularCallSupplement::EventSetCallRestriction(HRilRadioResponseInfo &info)
{
    if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("EventSetCallRestriction return, GetInstance is nullptr.");
        return;
    }
    DelayedSingleton<CellularCallRegister>::GetInstance()->ReportSetRestrictionResult((int32_t)info.error);
}

int32_t CellularCallSupplement::SetCallTransfer(const CallTransferInfo &cfInfo)
{
    if (!PhoneTypeGsmOrNot()) {
        TELEPHONY_LOGE("SetCallTransfer return, network type is not supported!");
        return ERR_NETWORK_TYPE;
    }
    if (strlen(cfInfo.transferNum) == 0) {
        TELEPHONY_LOGE("SetCallTransfer return, transferNum is empty!");
        return ERR_PHONE_NUMBER_EMPTY;
    }

    /**
     * <reason>:
     * 0   unconditional
     * 1   mobile busy
     * 2   no reply
     * 3   not reachable
     * 4   all call forwarding (refer 3GPP TS 22.030 [19])
     * 5   all conditional call forwarding (refer 3GPP TS 22.030 [19])
     * <mode>:
     * 0   disable
     * 1   enable
     * 2   query status
     * 3   registration
     * 4   erasure
     */
    if (cfInfo.type > CallTransferType::NOT_REACHABLE || cfInfo.type < CallTransferType::UNCONDITIONAL ||
        cfInfo.settingType > CallTransferSettingType::ERASURE ||
        cfInfo.settingType < CallTransferSettingType::REGISTRATION) {
        TELEPHONY_LOGE("SetCallTransfer return, parameter out of range!");
        return ERR_PARAMETER_OUT_OF_RANGE;
    }
    SupplementRequest supplementRequest;
    std::string dialString(cfInfo.transferNum);
    return supplementRequest.SetCallTransferRequest(cfInfo.settingType, cfInfo.type, dialString, 1);
}

int32_t CellularCallSupplement::InquireCallTransfer(CallTransferType type)
{
    if (!PhoneTypeGsmOrNot()) {
        TELEPHONY_LOGE("InquireCallTransfer return, network type is not supported!");
        return ERR_NETWORK_TYPE;
    }

    /**
     * When querying the status of a network service (<mode>=2) the response line for 'not active' case
     * (<status>=0) should be returned only if service is not active for any <class>
     */
    SupplementRequest supplementRequest;
    return supplementRequest.InquireCallTransferRequest(type);
}

bool CellularCallSupplement::PhoneTypeGsmOrNot()
{
    ModuleServiceUtils moduleServiceUtils;
    int32_t slot = CoreManager::DEFAULT_SLOT_ID;
    return moduleServiceUtils.GetNetworkStatus(slot) == RadioTech::RADIO_TECHNOLOGY_GSM;
}

int32_t CellularCallSupplement::SetCallWaiting(bool activate)
{
    /**
     * <n> (sets/shows the result code presentation status in the TA):
     * 0	disable
     * 1	enable
     */
    if (!PhoneTypeGsmOrNot()) {
        TELEPHONY_LOGE("SetCallWaiting return, network type is not supported!");
        return ERR_NETWORK_TYPE;
    }
    SupplementRequest supplementRequest;
    return supplementRequest.SetCallWaitingRequest(activate);
}

int32_t CellularCallSupplement::InquireCallWaiting()
{
    if (!PhoneTypeGsmOrNot()) {
        TELEPHONY_LOGE("InquireCallWaiting return, network type is not supported!");
        return ERR_NETWORK_TYPE;
    }
    SupplementRequest supplementRequest;
    return supplementRequest.InquireCallWaitingRequest();
}

int32_t CellularCallSupplement::SetCallRestriction(const CallRestrictionInfo &cRInfo)
{
    /**
     * <fac> values reserved by the present document:
     * "SC"	SIM (lock SIM/UICC card) (SIM/UICC asks password in ME power up and when this lock command issued)
     * "AO"	BAOC (Barr All Outgoing Calls) (refer 3GPP TS 22.088 [6] clause 1)
     * "OI"	BOIC (Barr Outgoing International Calls) (refer 3GPP TS 22.088 [6] clause 1)
     * "OX"	BOIC exHC (Barr Outgoing International Calls except to Home Country) (refer 3GPP TS 22.088 [6] clause 1)
     * "AI"	BAIC (Barr All Incoming Calls) (refer 3GPP TS 22.088 [6] clause 2)
     * "IR"	BIC Roam (Barr Incoming Calls when Roaming outside the home country) (refer 3GPP TS 22.088 [6] clause 2)
     * "AB"	All Barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
     * "AG"	All outGoing barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
     * "AC"	All inComing barring services (refer 3GPP TS 22.030 [19]) (applicable only for <mode>=0)
     */
    if (!PhoneTypeGsmOrNot()) {
        TELEPHONY_LOGE("SetCallRestriction return, network type is not supported!");
        return ERR_NETWORK_TYPE;
    }

    std::string fac;
    switch (cRInfo.fac) {
        case RESTRICTION_TYPE_ALL_OUTGOING:
            fac = "AO";
            break;
        case RESTRICTION_TYPE_INTERNATIONAL:
            fac = "OI";
            break;
        case RESTRICTION_TYPE_INTERNATIONAL_EXCLUDING_HOME:
            fac = "OX";
            break;
        case RESTRICTION_TYPE_ALL_INCOMING:
            fac = "AI";
            break;
        case RESTRICTION_TYPE_ROAMING_INCOMING:
            fac = "IR";
            break;
        default:
            TELEPHONY_LOGE("SetCallRestriction return, fac parameter out of range!");
            return ERR_PARAMETER_OUT_OF_RANGE;
    }
    if (cRInfo.mode < CallRestrictionMode::RESTRICTION_MODE_DEACTIVATION ||
        cRInfo.mode > CallRestrictionMode::RESTRICTION_MODE_ACTIVATION) {
        TELEPHONY_LOGE("SetCallRestriction return, mode parameter out of range!");
        return ERR_PARAMETER_OUT_OF_RANGE;
    }
    SupplementRequest supplementRequest;
    std::string info(cRInfo.password);
    return supplementRequest.SetCallRestriction(fac, cRInfo.mode, info);
}

int32_t CellularCallSupplement::InquireCallRestriction(CallRestrictionType facType)
{
    if (!PhoneTypeGsmOrNot()) {
        TELEPHONY_LOGE("InquireCallRestriction return, network type is not supported!");
        return ERR_NETWORK_TYPE;
    }
    std::string fac;
    switch (facType) {
        case RESTRICTION_TYPE_ALL_OUTGOING:
            fac = "AO";
            break;
        case RESTRICTION_TYPE_INTERNATIONAL:
            fac = "OI";
            break;
        case RESTRICTION_TYPE_INTERNATIONAL_EXCLUDING_HOME:
            fac = "OX";
            break;
        case RESTRICTION_TYPE_ALL_INCOMING:
            fac = "AI";
            break;
        case RESTRICTION_TYPE_ROAMING_INCOMING:
            fac = "IR";
            break;
        default:
            return ERR_PARAMETER_OUT_OF_RANGE;
    }
    SupplementRequest supplementRequest;
    return supplementRequest.InquireCallRestriction(fac);
}

void CellularCallSupplement::EventGetClip(GetClipResult &getClipResult)
{
    ClipResponse clipResponse;
    clipResponse.result = getClipResult.result;
    clipResponse.action = getClipResult.action;
    clipResponse.clipStat = getClipResult.clipStat;
    if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("EventGetClip return, GetInstance is nullptr.");
        return;
    }
    DelayedSingleton<CellularCallRegister>::GetInstance()->ReportGetClipResult(clipResponse);
}

void CellularCallSupplement::EventGetClir(GetClirResult &getClirResult)
{
    ClirResponse clirResponse;
    // 3GPP TS 27.007 V3.9.0 (2001-06) 7.7	Calling line identification restriction +CLIR
    clirResponse.result = getClirResult.result;
    clirResponse.action = getClirResult.action;
    clirResponse.clirStat = getClirResult.clirStat;
    if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("EventGetClir return, GetInstance is nullptr.");
        return;
    }
    DelayedSingleton<CellularCallRegister>::GetInstance()->ReportGetClirResult(clirResponse);
}

void CellularCallSupplement::EventSetClir(HRilRadioResponseInfo &info)
{
    ClirResponse clirResponse;
    clirResponse.result = static_cast<int32_t>(info.error);
    if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("EventSetClir return, GetInstance is nullptr.");
        return;
    }
    DelayedSingleton<CellularCallRegister>::GetInstance()->ReportSetClirResult(clirResponse);
}
} // namespace Telephony
} // namespace OHOS
