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

#ifndef CELLULAR_CALL_CONTROL_CS_H
#define CELLULAR_CALL_CONTROL_CS_H

#include "call_manager_inner_type.h"

#include "singleton.h"
#include "hril_call_parcel.h"
#include "event_handler.h"
#include "core_manager.h"
#include "cellular_call_connection_cs.h"
#include "mmi_handler.h"

namespace OHOS {
namespace Telephony {
class CSControl {
public:
    /**
     *  new CSControl
     *
     */
    CSControl();

    /**
     * ~CSControl destructor
     */
    ~CSControl();

    /**
     * CSControl Dial
     *
     * 27007-430_2001 6.27 Informative examples
     * 3GPP TS 22.030 [19]
     *
     * originate a voice call
     *
     * @param CellularCallInfo
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t Dial(const CellularCallInfo &callInfo);

    /**
     * CSControl End
     *
     * 27007-430_2001 7.13 Call related supplementary services +CHLD
     * 3GPP TS 22.030 [19]
     *
     * release the active call
     *
     * @param CellularCallInfo
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t End(const CellularCallInfo &callInfo);

    /**
     * CSControl Answer
     * 27007-430_2001 6.6 Alternating mode call control method
     * 3GPP TS 22.030 [19]
     *
     * Answer an incoming voice call.
     *
     * @param CellularCallInfo
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t Answer(const CellularCallInfo &callInfo);

    /**
     * CSControl Reject
     * 27007-430_2001 6.6 Alternating mode call control method
     * 3GPP TS 22.030 [19]
     *
     * Reject an incoming voice call
     *
     * @param CellularCallInfo
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t Reject(const CellularCallInfo &callInfo);

    /**
     * CSControl Hold
     *
     * 22083-400_2001 2 Call hold
     * 3GPP TS 22.030 [3]
     * 3GPP TS 23.083 V4.2.0 (2001-04) 2 Call hold (HOLD)
     *
     * The call hold service allows a served mobile subscriber
     *
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t Hold();

    /**
     * CSControl Active
     *
     * 22083-400_2001 2 Call hold
     * 3GPP TS 22.030 [3]
     *
     * Retrieve the held call.
     *
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t Active();

    /**
     * CSControl Swap
     *
     * 22083-400_2001 2 Call hold
     * 3GPP TS 22.030 [3]
     *
     * Alternate from one call to the other
     *
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t Swap();

    /**
     * CSControl Join
     *
     * 22084-400_2001 1.3.8.2	Managing an active multiParty call
     * 3GPP TS 22.030
     *
     * Add another remote party
     * @return Error Code: Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t Join();

    /**
     * CSControl Split
     *
     * 22084-400_2001 1.3.8.2	Managing an active multiParty call
     * 3GPP TS 22.030
     *
     * Separate a remote party
     * @param std::string splitString
     * @param index
     * @return Error Code: Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t Split(const std::string &splitString, int32_t index);

    /**
     * CSControl CallSupplement
     *
     * 3GPP TS 27.007 V3.9.0 (2001-06) Call related supplementary services +CHLD
     * 3GPP TS 27.007 V3.9.0 (2001-06) 7.22	Informative examples
     *
     * @param CallSupplementType
     * @return Error Code: Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t CallSupplement(CallSupplementType type);

    /**
     * CSControl TransmitDTMF
     *
     * 23014-400_2001 6	Support of DTMF across the air interface
     * 3GPP TS 22.030
     *
     * @param DTMF Code
     * @return Error Code: Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t TransmitDTMF(char cDTMFCode, const std::string &phoneNum);

    /**
     * CSControl InitiateDTMF
     *
     * 23014-400_2001 6	Support of DTMF across the air interface
     * 3GPP TS 22.030
     *
     * START DTMF : Containing the digit value (0-9,A,B,C,D,*,#)
     * @param DTMF Code
     * @return Error Code: Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t InitiateDTMF(char cDTMFCode, const std::string &phoneNum);

    /**
     * CSControl CeaseDTMF
     *
     * 23014-400_2001 6	Support of DTMF across the air interface
     * 3GPP TS 22.030
     *
     * STOP DTMF : No further info
     * @return Error Code: Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t CeaseDTMF(const std::string &phoneNum);

    /**
     * Send DTMF String
     *
     * 23014-400_2001 6	Support of DTMF across the air interface
     * 3GPP TS 22.030
     *
     * @param sDTMFCode
     * @param on the DTMF ON length in milliseconds, or 0 for default
     * @param off the DTMF OFF length in milliseconds, or 0 for default
     * @return Returns true for MMI was successfully executed, otherwise not the MMI process
     */
    int32_t TransmitDTMFString(const std::string &sDTMFCode, int32_t switchOn, int32_t switchOff);

    /**
     * CSControl ReleaseAllConnection
     */
    void ReleaseAllConnection();

    /**
     * CSControl DealCsCallsData.
     * @returns Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int DealCsCallsData(const CallInfoList &callInfoList);

    /**
     * Set Slot Id
     * @param id
     */
    void SetSlotId(int32_t id);

    /**
     * GetMMIHandler
     * @return std::shared_ptr<MMIHandler>
     */
    std::shared_ptr<MMIHandler> GetMMIHandler();

private:
    /**
     * Determines if a connection is currently in this state
     * @param TelCallState
     * @return Returns true or false
     */
    bool IsInState(TelCallState state);

    /**
     * Determine whether the call can be initiated currently
     * @return Returns true can to call
     */
    bool CanCall();

    /**
     * CSControl SetConnectionData
     * @param string phoneNUm
     * @param CellularCallConnectionCS
     * @return bool
     */
    bool SetConnectionData(const std::string &key, CellularCallConnectionCS &pConnectionCS);

    /**
     * CSControl GetConnectionData
     * @param string phoneNUm
     * @return CellularCallConnectionCS pointer
     */
    CellularCallConnectionCS *GetConnectionData(const std::string &key);

    /**
     * CSControl FindConnectionByState
     * @param TelCallState
     * @return CellularCallConnectionCS pointer
     */
    CellularCallConnectionCS *FindConnectionByState(TelCallState state);

    /**
     * CSControl FindConnectionByIndex
     * @param index
     * @return CellularCallConnectionCS pointer
     */
    CellularCallConnectionCS *FindConnectionByIndex(int32_t index);

    /**
     * Calculate International Roaming
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    bool CalculateInternationalRoaming() const;

    /**
     * Report being hung up data
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t ReportHungUpInfo();

    /**
     * Report Incoming info
     * @param list
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t ReportIncomingInfo(const CallInfoList &list);

    /**
     * Report update info
     * @param list
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t ReportUpdateInfo(const CallInfoList &list);

    /**
     * Encapsulation CallReportInfo
     * @param callInfo
     * @return CallReportInfo
     */
    CallReportInfo EncapsulationCallReportInfo(const CallInfo &callInfo);

    /**
     * DeleteConnection Connection send
     * @param cellularCSCallResponseInfo
     */
    void DeleteConnection(CallsReportInfo &callsResponseInfo, const CallInfoList &callInfoList);

    /**
     * Dial MMI Code
     * @param CellularCallInfo
     * @param std::string dialString
     * @returns Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t DialMMI(const CellularCallInfo &callInfo, const std::string &dialString);

    /**
     * Dial Cdma
     * @param CellularCallInfo
     * @param std::string dialString
     * @returns Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t DialCdma(const CellularCallInfo &callInfo, const std::string &dialString);

    /**
     *  Dial Gsm
     * @param CellularCallInfo
     * @param std::string phoneNum
     * @param CLIRMode clirMode
     * @returns Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t DialGsm(const CellularCallInfo &callInfo, const std::string &phoneNum, CLIRMode &clirMode);

    /**
     * CSControl init
     */
    bool Init();

    /**
     * Encapsulate Dial Common
     * @param std::string phoneNum
     * @param CLIRMode clirMode
     * @returns Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t EncapsulateDialCommon(const std::string &phoneNum, CLIRMode &clirMode);

    /**
     * is DTMF Key
     * 23014-400_2001 6	Support of DTMF across the air interface
     * 3GPP TS 22.030
     * @param char
     * @returns bool
     */
    bool IsDTMFKey(char c);

private:
    std::map<std::string, CellularCallConnectionCS> connectionMap_; // save callConnection map
    int32_t slotId_ = CoreManager::DEFAULT_SLOT_ID;
    std::shared_ptr<AppExecFwk::EventRunner> eventLoop_;
    std::shared_ptr<MMIHandler> handler_;
    const int32_t VOICE_CALL = 0;
    static constexpr HiviewDFX::HiLogLabel LOG_LABEL = {LOG_CORE, LOG_DOMAIN, "CSControl"};
};
} // namespace Telephony
} // namespace OHOS

#endif // CELLULAR_CALL_CONTROL_CS_H
