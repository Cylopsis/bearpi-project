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

#include "cs_control.h"

#include <regex>

#include "securec.h"

#include "standardize_utils.h"
#include "mmi_code_utils.h"
#include "module_service_utils.h"
#include "cellular_call_register.h"

namespace OHOS {
namespace Telephony {
CSControl::CSControl()
{
    Init();
}

bool CSControl::Init()
{
    TELEPHONY_LOGD("CSControl Init");
    eventLoop_ = AppExecFwk::EventRunner::Create("CSControl");
    if (eventLoop_ == nullptr) {
        TELEPHONY_LOGE("failed to create EventRunner");
        return false;
    }
    handler_ = std::make_shared<MMIHandler>(eventLoop_);
    if (handler_ == nullptr) {
        TELEPHONY_LOGE("failed to make_shared handler_");
        return false;
    }
    return true;
}

CSControl::~CSControl()
{
    ReleaseAllConnection();
}

int32_t CSControl::Dial(const CellularCallInfo &callInfo)
{
    std::string dialString(callInfo.phoneNum);
    if (dialString.empty()) {
        TELEPHONY_LOGE("Dial return, dialString is empty.");
        return CALL_MANAGER_PHONE_NUMBER_NULL;
    }

    ModuleServiceUtils moduleServiceUtils;
    moduleServiceUtils.GetRadioStatus(slotId_);

    RadioTech netType = moduleServiceUtils.GetNetworkStatus(slotId_);
    if (netType == RadioTech::RADIO_TECHNOLOGY_GSM) {
        return DialMMI(callInfo, dialString);
    }
    if (netType == RadioTech::RADIO_TECHNOLOGY_WCDMA) {
        return DialCdma(callInfo, dialString);
    }
    TELEPHONY_LOGE("Dial return, net type error.");
    return ERR_NETWORK_TYPE;
}

int32_t CSControl::DialCdma(const CellularCallInfo &callInfo, const std::string &dialString)
{
    if (!CanCall()) {
        TELEPHONY_LOGE("CSControl::DialCdma return, error type: call state error.");
        return ERR_CALL_NUMBER_LIMIT;
    }
    // When a terminal wanders between countries (i.e. networks), need get get the country and operator name
    StandardizeUtils standardizeUtils;
    // Remove the phone number separator
    std::string newPhoneNum = standardizeUtils.RemoveSeparatorsPhoneNumber(dialString);

    if (IsInState(CALL_STATUS_ACTIVE)) {
        TELEPHONY_LOGD("DialCdma, CDMA is have connection in active state.");
        CallReportInfo cellularCallReportInfo;
        if (memset_s(&cellularCallReportInfo, sizeof(cellularCallReportInfo), 0, sizeof(cellularCallReportInfo)) !=
            EOK) {
            TELEPHONY_LOGE("HandleOnMultiCall return, memset_s fail.");
            return ERR_SYSTEM_INVOKE;
        }
        if (strcpy_s(cellularCallReportInfo.accountNum, strlen(callInfo.phoneNum) + 1, callInfo.phoneNum) != EOK) {
            TELEPHONY_LOGE("HandleOnMultiCall return, strcpy_s fail.");
            return ERR_SYSTEM_INVOKE;
        }

        cellularCallReportInfo.state = CALL_STATUS_DIALING;
        cellularCallReportInfo.callType = CallType::TYPE_CS;
        cellularCallReportInfo.callMode = VideoStateType::TYPE_VOICE;
        CellularCallConnectionCS csConnection;
        csConnection.SetOrUpdateCallReportInfo(cellularCallReportInfo);

        if (!SetConnectionData(callInfo.phoneNum, csConnection)) {
            TELEPHONY_LOGE("HandleOnMultiCall cdma return, SetConnectionData fail.");
            return ERR_CALL_ALREADY_EXISTS;
        }
        return csConnection.SendCDMAThreeWayDialRequest(slotId_);
    }
    CLIRMode clirMode = CLIRMode::DEFAULT;
    return EncapsulateDialCommon(newPhoneNum, clirMode);
}

int32_t CSControl::DialMMI(const CellularCallInfo &callInfo, const std::string &dialString)
{
    StandardizeUtils standardizeUtils;
    // Remove the phone number separator
    std::string newPhoneNum = standardizeUtils.RemoveSeparatorsPhoneNumber(dialString);

    // Also supplementary services may be controlled using dial command according to 3GPP TS 22.030 [19].
    // An example of call forwarding on no reply for telephony with the adjustment of the
    // no reply condition timer on 25 seconds:
    // look at the Network portion
    // Parse the MMI code from the string
    std::unique_ptr<MMICodeUtils> mmiCodeUtils = std::make_unique<MMICodeUtils>();
    if (mmiCodeUtils == nullptr) {
        TELEPHONY_LOGE("DialMMI failed to make_unique mmiCodeUtils");
        return ERR_NULL_POINTER;
    }
    CLIRMode clirMode = CLIRMode::DEFAULT;
    if (!mmiCodeUtils->IsNeedExecuteMmi(newPhoneNum)) {
        TELEPHONY_LOGD("CSControl::DialMMI dail DEFAULT number");
        return DialGsm(callInfo, newPhoneNum, clirMode);
    }
    if (mmiCodeUtils->mmiData_.serviceCode == "30" && !mmiCodeUtils->mmiData_.dialString.empty()) {
        if (mmiCodeUtils->mmiData_.actionString == "*") {
            clirMode = CLIRMode::TRANSFER;
        } else if (mmiCodeUtils->mmiData_.actionString == "#") {
            clirMode = CLIRMode::INHIBITION;
        }
        if (clirMode == CLIRMode::TRANSFER || clirMode == CLIRMode::INHIBITION) {
            TELEPHONY_LOGD("CSControl::DialMMI dail CLIR mode number");
            return DialGsm(callInfo, mmiCodeUtils->mmiData_.dialString, clirMode);
        }
    }
    if (handler_ != nullptr) {
        TELEPHONY_LOGD("CSControl::DialMMI handler_ is not nullptr.");
        handler_->SendEvent(MMIHandlerId::EVENT_MMI_Id, mmiCodeUtils);
    }
    TELEPHONY_LOGD("CSControl::DialMMI return, mmi code type.");
    return ERR_MMI_TYPE;
}

int32_t CSControl::DialGsm(const CellularCallInfo &callInfo, const std::string &phoneNum, CLIRMode &clirMode)
{
    if (!CanCall()) {
        TELEPHONY_LOGE("DialGsm return, error type: call state error.");
        return ERR_CALL_NUMBER_LIMIT;
    }

    // Calls can be put on hold, recovered, released, added to conversation,
    // and transferred similarly as defined in 3GPP TS 22.030 [19].
    if (IsInState(CALL_STATUS_ACTIVE)) {
        // New calls must be active, so other calls need to be hold
        TELEPHONY_LOGD("DialGsm, GSM is have connection in active state.");
        CellularCallConnectionCS pConnection;

        // Delay dialing to prevent failure to add a new call while making a multi-party call
        // Will it block the main thread or other threads? Will the reception of messages be blocked during sleep?
        // - a call can be temporarily disconnected from the ME but the connection is retained by the network
        pConnection.SwapRequest(slotId_);
    }
    return EncapsulateDialCommon(phoneNum, clirMode);
}

int32_t CSControl::EncapsulateDialCommon(const std::string &phoneNum, CLIRMode &clirMode)
{
    CallReportInfo cellularCallReportInfo;
    if (memset_s(&cellularCallReportInfo, sizeof(cellularCallReportInfo), 0, sizeof(cellularCallReportInfo)) !=
        EOK) {
        TELEPHONY_LOGE("EncapsulateDialCommon return, memset_s fail.");
        return ERR_SYSTEM_INVOKE;
    }
    if (strcpy_s(cellularCallReportInfo.accountNum, strlen(phoneNum.c_str()) + 1, phoneNum.c_str()) != EOK) {
        TELEPHONY_LOGE("EncapsulateDialCommon return, strcpy_s fail.");
        return ERR_SYSTEM_INVOKE;
    }
    cellularCallReportInfo.state = CALL_STATUS_DIALING;
    cellularCallReportInfo.callType = CallType::TYPE_CS;
    cellularCallReportInfo.callMode = VideoStateType::TYPE_VOICE;
    CellularCallConnectionCS csConnection;
    csConnection.SetOrUpdateCallReportInfo(cellularCallReportInfo);

    if (!SetConnectionData(phoneNum, csConnection)) {
        TELEPHONY_LOGE("EncapsulateDialCommon return, SetConnectionData fail.");
        return ERR_CALL_ALREADY_EXISTS;
    }

    DialRequestStruct dialRequest;

    /**
     * <idx>: integer type;
     * call identification number as described in 3GPP TS 22.030 [19] subclause 4.5.5.1
     * this number can be used in +CHLD command operations
     * <dir>:
     */
    dialRequest.phoneNum = phoneNum;

    /**
     * <n> (parameter sets the adjustment for outgoing calls):
     *  0	presentation indicator is used according to the subscription of the CLIR service
     *  1	CLIR invocation
     *  2	CLIR suppression
     */
    dialRequest.clirMode = clirMode;

    /**
     * An example of voice group call service request usage:
     * ATD*17*753#500; (originate voice group call with the priority level 3)
     * OK (voice group call setup was successful)
     */
    return csConnection.DialRequest(dialRequest, slotId_);
}

int32_t CSControl::End(const CellularCallInfo &callInfo)
{
    // Match the session connection according to the phone number string
    CellularCallConnectionCS *pConnection = GetConnectionData(callInfo.phoneNum);
    if (pConnection == nullptr) {
        TELEPHONY_LOGI("End: connection cannot be matched, use index directly");
        pConnection = FindConnectionByIndex(callInfo.index);
    }
    if (pConnection == nullptr) {
        TELEPHONY_LOGE("CSControl::End, error type: connection is null");
        return ERR_CONNECTION;
    }

    if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("CSControl::End, error type: GetInstance() is nullptr");
        return ERR_NULL_POINTER;
    }
    DelayedSingleton<CellularCallRegister>::GetInstance()->ReportSingleCallInfo(
        *pConnection, CALL_STATUS_DISCONNECTING);

    /**
     * The "directory number" case shall be handled with dial command D,
     * and the END case with hangup command H (or +CHUP).
     * (e.g. +CHLD: (0,1,1x,2,2x,3)).
     * NOTE: Call Hold, MultiParty and Explicit Call Transfer are only applicable to teleservice 11.
     */
    return pConnection->EndRequest(slotId_);
}

int32_t CSControl::Answer(const CellularCallInfo &callInfo)
{
    CellularCallConnectionCS *pConnection = GetConnectionData(callInfo.phoneNum);
    if (pConnection == nullptr) {
        TELEPHONY_LOGE("CSControl::Answer, error type: connection is null");
        return ERR_CONNECTION;
    }

    /**
     * <stat> (state of the call):
     * 0 active
     * 1 held
     * 2 dialing (MO call)
     * 3 alerting (MO call)
     * 4 incoming (MT call)
     * 5 waiting (MT call)
     */
    // There is an active call when you call, or third party call waiting
    if (IsInState(CALL_STATUS_ACTIVE) || pConnection->GetStatus() == CALL_STATUS_WAITING) {
        TELEPHONY_LOGD("Answer there is an active call when you call, or third party call waiting");
        CellularCallConnectionCS *con = FindConnectionByState(CALL_STATUS_ACTIVE);
        if (con == nullptr) {
            TELEPHONY_LOGE("Answer return, error type: con is null, there are no active calls");
            return ERR_CONNECTION;
        }

        /**
         * shows commands to start the call, to switch from voice to data (In Call Modification) and to hang up the
         * call. +CMOD and +FCLASS commands indicate the current settings before dialling or answering command, not
         * that they shall be given just before D or A command.
         */
        return con->SwapRequest(slotId_);
    }

    if (pConnection->GetStatus() == CALL_STATUS_INCOMING || pConnection->GetStatus() == CALL_STATUS_ALERTING) {
        return pConnection->AnswerRequest(slotId_);
    }

    TELEPHONY_LOGE("CSControl::Answer return, error type: call state error, phone not ringing.");
    return ERR_CALL_STATE;
}

int32_t CSControl::Reject(const CellularCallInfo &callInfo)
{
    CellularCallConnectionCS *pConnection = GetConnectionData(callInfo.phoneNum);
    if (pConnection == nullptr) {
        TELEPHONY_LOGI("Reject: connection cannot be matched, use index directly");
        pConnection = FindConnectionByIndex(callInfo.index);
    }
    if (pConnection == nullptr) {
        TELEPHONY_LOGE("CSControl::Reject, error type: connection is null");
        return ERR_CONNECTION;
    }

    /**
     * shows commands to start the call, to switch from voice to data (In Call Modification) and to hang up the call.
     * +CMOD and +FCLASS commands indicate the current settings before dialling or answering command,
     * not that they shall be given just before D or A command.
     */
    if (!pConnection->IsRingingState()) {
        TELEPHONY_LOGE("CSControl::Reject return, error type: call state error, phone not ringing.");
        return ERR_CALL_STATE;
    }
    if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("CSControl::Reject, error type: GetInstance() is nullptr");
        return ERR_NULL_POINTER;
    }
    DelayedSingleton<CellularCallRegister>::GetInstance()->ReportSingleCallInfo(
        *pConnection, CALL_STATUS_DISCONNECTING);
    return pConnection->RejectRequest(slotId_);
}

int32_t CSControl::Hold()
{
    /**
     * When the call hold service is invoked, communication is interrupted on the traffic channel and the traffic
     * channel is released from the existing call. The traffic channel is reserved for the served mobile subscriber
     * invoking the call hold service. The served mobile subscriber can only have one call on hold at a time.
     */
    if (IsInState(CALL_STATUS_INCOMING)) {
        TELEPHONY_LOGE("Hold return, error type: call state error.");
        return ERR_CALL_STATE;
    }
    CellularCallConnectionCS connection;
    return connection.HoldRequest(slotId_);
}

int32_t CSControl::Active()
{
    // A notification shall be send towards the previously held party that the call has been retrieved.
    if (IsInState(CALL_STATUS_INCOMING)) {
        TELEPHONY_LOGE("Active return, error type: call state error.");
        return ERR_CALL_STATE;
    }
    CellularCallConnectionCS connection;
    return connection.ActiveRequest(slotId_);
}

int32_t CSControl::Swap()
{
    /**
     * If the served mobile subscriber is connected to an active call and has another call on hold, she can:
     * 1) Alternate from one call to the other.
     * 2) Disconnect the active call.
     * 3) Disconnect the held call.
     * 4) Disconnect both calls.
     */
    if (IsInState(CALL_STATUS_INCOMING)) {
        TELEPHONY_LOGE("Swap return, error type: call state error.");
        return ERR_CALL_STATE;
    }
    CellularCallConnectionCS connection;
    return connection.SwapRequest(slotId_);
}

/**
 * Explicitly choose one remote party to have a private communication with.
 * This results in that remote party being removed from the multiParty call which is placed on hold,
 * and the conversation between the served mobile subscriber and the designated remote party being a normal
 * active call. The remaining remote parties may have communication with each other in this state.
 */
int32_t CSControl::Split(const std::string &splitString, int32_t index)
{
    if (splitString.empty()) {
        TELEPHONY_LOGW("Split, splitString is empty.");
    }

    CellularCallConnectionCS *pConnection = GetConnectionData(splitString);
    if (pConnection != nullptr) {
        return pConnection->SplitRequest(pConnection->GetIndex(), slotId_, VOICE_CALL);
    }

    TELEPHONY_LOGI("Split: connection cannot be matched, use index directly");
    CellularCallConnectionCS connection;
    return connection.SplitRequest(index, slotId_, VOICE_CALL);
}

/**
 * Add another remote party, to which a private communication has been established using
 * the same procedures as in Section 1.3.8.1, if the number of remote parties does not then
 * exceed the maximum number allowed, which results in an active multiParty call.
 */
int32_t CSControl::Join()
{
    CellularCallConnectionCS connectionCs;
    return connectionCs.JoinRequest(slotId_, VOICE_CALL);
}

int32_t CSControl::CallSupplement(CallSupplementType type)
{
    switch (type) {
        // 3GPP TS 27.007 V3.9.0 (2001-06) Call related supplementary services +CHLD
        // 3GPP TS 27.007 V3.9.0 (2001-06) 7.22	Informative examples
        case CallSupplementType::TYPE_HANG_UP_HOLD_WAIT:
        // release the second (active) call and recover the first (held) call
        case CallSupplementType::TYPE_HANG_UP_ACTIVE: {
            CellularCallConnectionCS connection;
            return connection.CallSupplementRequest(type, slotId_);
        }
        case CallSupplementType::TYPE_HANG_UP_ALL: {
            TELEPHONY_LOGD("CallSupplement, hang up all call");
            CellularCallConnectionCS connection;
            return connection.RejectRequest(slotId_);
        }
        default: {
            TELEPHONY_LOGW("CallSupplement warring, type is invalid");
            return ERR_PARAMETER_INVALID;
        }
    }
}

int32_t CSControl::TransmitDTMF(char cDTMFCode, const std::string &phoneNum)
{
    /**
     * 3gpp 27007-430_2001
     *
     * C.2.11	DTMF and tone generation +VTS
     *
     * This command allows the transmission of DTMF tones and arbitrary tones (see note).
     * These tones may be used (for example) when announcing the start of a recording period.
     * The command is write only.
     * In this profile of commands, this command does not operate in data or fax modes of operation (+FCLASS=0,1,2
     7). NOTE 1:	D is used only for dialling.

     The string parameter of the command consists of combinations of the following separated by commas:
     1. <DTMF>. A single ASCII character in the set 0 9, #,*,A D.
     This is interpreted as a single ACSII character whose duration is set by the +VTD command.
        NOTE 2:	In GSM this operates only in voice mode.
     2. [<tone1>,<tone2>,<duration>].
     This is interpreted as a dual tone of frequencies <tone1> and <tone2>, lasting for a time <duration> (in 10 ms
     multiples). NOTE 3:	This does not operate in GSM.
     3. {<DTMF>,<duration>}. This is interpreted as a DTMF tone of different duration from that mandated by the
     +VTD command. NOTE 4:	In GSM this operates only in voice mode.
     */
    TELEPHONY_LOGD("CSControl::TransmitDTMF start");
    CellularCallConnectionCS *pConnection = GetConnectionData(phoneNum);
    if (pConnection == nullptr) {
        TELEPHONY_LOGE("CSControl::TransmitDTMF, error type: connection is null");
        return ERR_CONNECTION;
    }
    if (!IsDTMFKey(cDTMFCode)) {
        TELEPHONY_LOGE("CSControl::TransmitDTMF return, error type: cDTMFCode invalid.");
        return ERR_PARAMETER_OUT_OF_RANGE;
    }
    int32_t index = pConnection->GetIndex();
    return pConnection->TransmitDTMFRequest(cDTMFCode, index, slotId_);
}

int32_t CSControl::InitiateDTMF(char cDTMFCode, const std::string &phoneNum)
{
    /**
     * The messages when sent across the air interface should contain the following information:
     * a) START DTMF : Containing the digit value (0-9,A,B,C,D,*,#);
     * b) START DTMF ACKNOWLEDGE: Containing the digit value (0-9,A,B,C,D,*,#) corresponding to the DTMF tone that
     * the network applies towards the remote user;
     * c) STOP DTMF : No further info;
     * d) STOP DTMF ACKNOWLEDGE: No further info.
     * Only a single digit will be passed in each START DTMF and START DTMF ACKNOWLEDGE message
     */
    CellularCallConnectionCS *pConnection = GetConnectionData(phoneNum);
    if (pConnection == nullptr) {
        TELEPHONY_LOGE("InitiateDTMF, error type: connection is null");
        return ERR_CONNECTION;
    }
    if (!IsDTMFKey(cDTMFCode)) {
        TELEPHONY_LOGE("InitiateDTMF return, error type: cDTMFCode invalid.");
        return ERR_PARAMETER_OUT_OF_RANGE;
    }
    int32_t index = pConnection->GetIndex();
    return pConnection->InitiateDTMFRequest(cDTMFCode, index, slotId_);
}

int32_t CSControl::CeaseDTMF(const std::string &phoneNum)
{
    /**
     * The messages when sent across the air interface should contain the following information:
     * a) START DTMF : Containing the digit value (0-9,A,B,C,D,*,#);
     * b) START DTMF ACKNOWLEDGE: Containing the digit value (0-9,A,B,C,D,*,#) corresponding to the DTMF tone that
     * the network applies towards the remote user;
     * c) STOP DTMF : No further info;
     * d) STOP DTMF ACKNOWLEDGE: No further info.
     * Only a single digit will be passed in each START DTMF and START DTMF ACKNOWLEDGE message
     */
    CellularCallConnectionCS *pConnection = GetConnectionData(phoneNum);
    if (pConnection == nullptr) {
        TELEPHONY_LOGE("CeaseDTMF, error type: connection is null");
        return ERR_CONNECTION;
    }
    int32_t index = pConnection->GetIndex();
    return pConnection->CeaseDTMFRequest(index, slotId_);
}

int32_t CSControl::TransmitDTMFString(const std::string &sDTMFCode, int32_t switchOn, int32_t switchOff)
{
    /**
     * 3gpp 27007-430_2001
     *
     * C.2.11	DTMF and tone generation +VTS
     *
     * This command allows the transmission of DTMF tones and arbitrary tones (see note).
     * These tones may be used (for example) when announcing the start of a recording period.
     * The command is write only.
     * In this profile of commands, this command does not operate in data or fax modes of operation (+FCLASS=0,1,2
     7).
     * NOTE 1:	D is used only for dialling.

     * The string parameter of the command consists of combinations of the following separated by commas:
     * 1. <DTMF>. A single ASCII character in the set 0 9, #,*,A D.
     * This is interpreted as a single ACSII character whose duration is set by the +VTD command.
     *   NOTE 2:	In GSM this operates only in voice mode.
     * 2. [<tone1>,<tone2>,<duration>].
     * This is interpreted as a dual tone of frequencies <tone1> and <tone2>, lasting for a time <duration> (in 10
     ms multiples).
     *   NOTE 3:	This does not operate in GSM.
     * 3. {<DTMF>,<duration>}. This is interpreted as a DTMF tone of different duration from that mandated by the
     +VTD command.
     *   NOTE 4:	In GSM this operates only in voice mode.
     */
    // In GSM this operates only in voice mode.
    CellularCallConnectionCS connection;
    for (unsigned int i = 0; i < sDTMFCode.length(); i++) {
        // A single ASCII character in the set 0 9, #,*,A D.
        if (!IsDTMFKey(sDTMFCode.at(i))) {
            return ERR_PARAMETER_OUT_OF_RANGE;
        }
    }
    return connection.TransmitDTMFStringRequest(sDTMFCode, switchOn, switchOff, slotId_);
}

bool CSControl::CalculateInternationalRoaming() const
{
    bool ret = true;
    ModuleServiceUtils moduleServiceUtils;
    std::string operatorCountryIso = moduleServiceUtils.GetNetworkCountryCode(slotId_);
    std::string simCountryIso = moduleServiceUtils.GetIsoCountryCode(slotId_);
    ret = !operatorCountryIso.empty() && !simCountryIso.empty() && (operatorCountryIso != simCountryIso);
    if (ret) {
        if (simCountryIso == "us") {
            ret = operatorCountryIso != "vi";
        } else if (simCountryIso == "vi") {
            ret = operatorCountryIso != "us";
        }
    }
    return ret;
}

bool CSControl::CanCall()
{
    TELEPHONY_LOGD("CanCall, connection size: %{public}zu.", connectionMap_.size());
    unsigned int numSize = 6;
    return connectionMap_.size() <= numSize;
}

bool CSControl::IsInState(TelCallState state)
{
    CellularCallConnectionCS *pConnection = FindConnectionByState(state);
    if (pConnection != nullptr) {
        return true;
    }
    return false;
}

bool CSControl::SetConnectionData(const std::string &key, CellularCallConnectionCS &pConnectionCS)
{
    if (key.empty()) {
        TELEPHONY_LOGE("SetConnectionData, key is empty.");
        return false;
    }
    if (!connectionMap_.insert(std::make_pair(key, pConnectionCS)).second) {
        TELEPHONY_LOGE("SetConnectionData, key already exists.");
        return false;
    }
    return true;
}

CellularCallConnectionCS *CSControl::GetConnectionData(const std::string &key)
{
    if (key.empty()) {
        TELEPHONY_LOGE("GetConnectionData, key is empty.");
        return nullptr;
    }

    auto p = connectionMap_.find(key);
    if (p != connectionMap_.end()) {
        return &p->second;
    }
    return nullptr;
}

CellularCallConnectionCS *CSControl::FindConnectionByState(TelCallState state)
{
    CellularCallConnectionCS *pConnection = nullptr;
    std::map<std::string, CellularCallConnectionCS>::iterator it;
    for (it = connectionMap_.begin(); it != connectionMap_.end(); ++it) {
        pConnection = &it->second;
        if (pConnection != nullptr && pConnection->GetStatus() == state) {
            return pConnection;
        }
    }
    return nullptr;
}

CellularCallConnectionCS *CSControl::FindConnectionByIndex(int32_t index)
{
    CellularCallConnectionCS *pConnection = nullptr;
    std::map<std::string, CellularCallConnectionCS>::iterator it;
    for (it = connectionMap_.begin(); it != connectionMap_.end(); ++it) {
        pConnection = &it->second;
        if (pConnection != nullptr && pConnection->GetIndex() == index) {
            return pConnection;
        }
    }
    return nullptr;
}

int CSControl::DealCsCallsData(const CallInfoList &callInfoList)
{
    if (callInfoList.callSize <= 0 && !connectionMap_.empty()) {
        return ReportHungUpInfo();
    } else if (callInfoList.callSize > 0 && connectionMap_.empty()) {
        return ReportIncomingInfo(callInfoList);
    } else if (callInfoList.callSize > 0 && !connectionMap_.empty()) {
        return ReportUpdateInfo(callInfoList);
    }
    return ERR_REPORT_CALLS_INFO;
}

int CSControl::ReportUpdateInfo(const CallInfoList &callInfoList)
{
    CallsReportInfo callsReportInfo;
    for (int i = 0; i < callInfoList.callSize; ++i) {
        CallReportInfo cellularCallReportInfo = EncapsulationCallReportInfo(callInfoList.calls[i]);

        CellularCallConnectionCS *pConnection = nullptr;
        pConnection = GetConnectionData(callInfoList.calls[i].number);
        if (pConnection == nullptr) {
            CellularCallConnectionCS connection;
            connection.SetOrUpdateCallReportInfo(cellularCallReportInfo);
            connection.SetFlag(true);
            connection.SetIndex(callInfoList.calls[i].index);
            SetConnectionData(callInfoList.calls[i].number, connection);
        } else {
            pConnection->SetFlag(true);
            pConnection->SetIndex(callInfoList.calls[i].index);
            pConnection->SetOrUpdateCallReportInfo(cellularCallReportInfo);
        }
        callsReportInfo.callVec.push_back(cellularCallReportInfo);
    }

    DeleteConnection(callsReportInfo, callInfoList);
    if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("ReportUpdateInfo return, GetInstance() is nullptr.");
        return ERR_NULL_POINTER;
    }
    DelayedSingleton<CellularCallRegister>::GetInstance()->ReportCallsInfo(callsReportInfo);
    return TELEPHONY_SUCCESS;
}

void CSControl::DeleteConnection(CallsReportInfo &callsReportInfo, const CallInfoList &callInfoList)
{
    auto it = connectionMap_.begin();
    while (it != connectionMap_.end()) {
        CellularCallConnectionCS *pConnection = &it->second;
        CallReportInfo callReportInfo = pConnection->GetCallReportInfo();
        if (!pConnection->GetFlag()) {
            callReportInfo.state = CALL_STATUS_DISCONNECTED;
            callsReportInfo.callVec.push_back(callReportInfo);
            connectionMap_.erase(it++);
        } else {
            pConnection->SetFlag(false);
            ++it;
        }
    }
}

CallReportInfo CSControl::EncapsulationCallReportInfo(const CallInfo &callInfo)
{
    CallReportInfo callReportInfo;
    if (memset_s(&callReportInfo, sizeof(callReportInfo), 0, sizeof(callReportInfo)) != EOK) {
        TELEPHONY_LOGE("EncapsulationCallReportInfo return, memset_s fail.");
        return callReportInfo;
    }

    /**
     * <idx>: integer type;
     * call identification number as described in 3GPP TS 22.030 [19] subclause 4.5.5.1
     * this number can be used in +CHLD command operations
     * <dir>:
     */
    int32_t cpyLen = strlen(callInfo.number.c_str()) + 1;
    if (strcpy_s(callReportInfo.accountNum, cpyLen, callInfo.number.c_str()) != EOK) {
        TELEPHONY_LOGE("EncapsulationCallReportInfo return, strcpy_s fail.");
        return callReportInfo;
    }

    /**
     * <stat> (state of the call):
     * 0 active
     * 1 held
     * 2 dialing (MO call)
     * 3 alerting (MO call)
     * 4 incoming (MT call)
     * 5 waiting (MT call)
     */
    callReportInfo.index = callInfo.index;
    callReportInfo.state = static_cast<TelCallState>(callInfo.state);
    callReportInfo.callType = CallType::TYPE_CS;
    callReportInfo.callMode = VideoStateType::TYPE_VOICE;
    return callReportInfo;
}

int32_t CSControl::ReportIncomingInfo(const CallInfoList &callInfoList)
{
    CallsReportInfo callsReportInfo;
    for (int i = 0; i < callInfoList.callSize; ++i) {
        CallReportInfo cellularCallReportInfo = EncapsulationCallReportInfo(callInfoList.calls[i]);

        CellularCallConnectionCS connection;
        connection.SetStatus(static_cast<TelCallState>(callInfoList.calls[i].state));
        connection.SetIndex(callInfoList.calls[i].index);
        connection.SetOrUpdateCallReportInfo(cellularCallReportInfo);
        SetConnectionData(callInfoList.calls[i].number, connection);

        callsReportInfo.callVec.push_back(cellularCallReportInfo);
    }
    if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("ReportIncomingInfo return, GetInstance() is nullptr.");
        return ERR_PARAMETER_INVALID;
    }
    DelayedSingleton<CellularCallRegister>::GetInstance()->ReportCallsInfo(callsReportInfo);
    return TELEPHONY_SUCCESS;
}

int32_t CSControl::ReportHungUpInfo()
{
    CallsReportInfo callsReportInfo;
    std::map<std::string, CellularCallConnectionCS>::iterator it;
    for (it = connectionMap_.begin(); it != connectionMap_.end(); ++it) {
        CellularCallConnectionCS *pConnection = &it->second;
        CallReportInfo callReportInfo = pConnection->GetCallReportInfo();
        callReportInfo.state = CALL_STATUS_DISCONNECTED;
        callsReportInfo.callVec.push_back(callReportInfo);
    }
    if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("ReportHungUpInfo return, GetInstance() is nullptr.");
        return ERR_NULL_POINTER;
    }
    DelayedSingleton<CellularCallRegister>::GetInstance()->ReportCallsInfo(callsReportInfo);
    ReleaseAllConnection();
    return TELEPHONY_SUCCESS;
}

void CSControl::ReleaseAllConnection()
{
    connectionMap_.clear();
}

bool CSControl::IsDTMFKey(char c)
{
    /**
     * 1. <DTMF>. A single ASCII character in the set 0 9, #,*,A D. This is interpreted as a single ASCII character
     * whose duration is set by the +VTD command. NOTE 2:	In GSM this operates only in voice mode.
     */
    return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'D') || c == '*' || c == '#';
}

void CSControl::SetSlotId(int32_t id)
{
    slotId_ = id;
}

std::shared_ptr<MMIHandler> CSControl::GetMMIHandler()
{
    return handler_;
}
} // namespace Telephony
} // namespace OHOS