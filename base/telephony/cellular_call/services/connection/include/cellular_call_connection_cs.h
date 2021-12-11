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

#ifndef CELLULAR_CALL_CONNECTION_CS_H
#define CELLULAR_CALL_CONNECTION_CS_H

#include "core.h"
#include "call_manager_inner_type.h"
#include "hril_call_parcel.h"
#include "telephony_errors.h"
#include "cellular_call_data_struct.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
class CellularCallConnectionCS {
public:
    /**
     * MO new CellularCallConnectionCS
     *
     */
    CellularCallConnectionCS();

    /**
     * ~CellularCallConnectionCS. destructor
     */
    ~CellularCallConnectionCS();

    /**
     *  DialRequest
     * @param DialRequestStruct
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t DialRequest(const DialRequestStruct &dialRequest, int32_t slotId);

    /**
     *  EndRequest
     * @param slotId
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t EndRequest(int32_t slotId);

    /**
     *  AnswerRequest
     *
     * @param slotId
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t AnswerRequest(int32_t slotId);

    /**
     *  RejectRequest
     *
     * @param slotId
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t RejectRequest(int32_t slotId);

    /**
     *  SwapRequest
     *
     * @param slotId
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t SwapRequest(int32_t slotId);

    /**
     *  HoldRequest
     *
     * @param slotId
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t HoldRequest(int32_t slotId);

    /**
     *  ActiveRequest
     *
     * @param slotId
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t ActiveRequest(int32_t slotId);

    /**
     *  JoinRequest
     *
     * @param slotId
     * @param voiceCall
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t JoinRequest(int32_t slotId, int32_t voiceCall);

    /**
     *  SplitRequest
     *
     * @param index
     * @param slotId
     * @param voiceCall
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t SplitRequest(int32_t index, int32_t slotId, int32_t voiceCall);

    /**
     * CallSupplement Request
     *
     * 22083-400_2001 2 Call hold
     * 3GPP TS 22.030 [3]
     *
     * @param CallSupplementType
     * @param slotId
     * @return Error Code: Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t CallSupplementRequest(CallSupplementType type, int32_t slotId);

    /**
     * CSControl GetCsCallsDataRequest
     * 27007-430_2001 7.18 List current calls +CLCC
     * 3GPP TS 22.030 [19]
     *
     * Returns list of current calls of ME.
     *
     * @param slotId
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t GetCsCallsDataRequest(int32_t slotId, int64_t lastCallsDataFlag);

    /**
     *  SendCDMAThreeWayDialRequest
     *
     * @param slotId
     * @return Error Code: Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t SendCDMAThreeWayDialRequest(int32_t slotId);

    /**
     *  TransmitDTMFRequest
     *
     * @param char cDTMFCode
     * @param slotId
     * @return Error Code: Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t TransmitDTMFRequest(char cDTMFCode, int32_t index, int32_t slotId);

    /**
     *  InitiateDTMFRequest
     *
     * @param char cDTMFCode
     * @param slotId
     * @return Error Code: Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t InitiateDTMFRequest(char cDTMFCode, int32_t index, int32_t slotId);

    /**
     *  CeaseDTMFRequest
     *
     * @param slotId
     * @return Error Code: Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t CeaseDTMFRequest(int32_t index, int32_t slotId);

    /**
     *  TransmitDTMFStringRequest
     * @param
     * @return Error Code: Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t TransmitDTMFStringRequest(const std::string &sDTMFCode, int32_t on, int32_t off, int32_t slotId);

    /**
     * Set or Update CallReportInfo
     * @param CallReportInfo
     */
    void SetOrUpdateCallReportInfo(CallReportInfo &callReportInfo);

    /**
     * Get CallReportInfo
     * @return CallReportInfo
     */
    CallReportInfo GetCallReportInfo();

    /**
     * SetStatus
     * @param state
     */
    void SetStatus(TelCallState state);

    /**
     * GetStatus
     * @return TelCallState
     */
    TelCallState GetStatus();

    /**
     * IsRingingState.
     *
     * @return bool IsRingingState
     */
    bool IsRingingState();

    /**
     * Set Flag
     * @param bool
     */
    void SetFlag(bool flag);

    /**
     * Get Flag
     * @return bool
     */
    bool GetFlag();

    /**
     * Set Index
     * @param int
     */
    void SetIndex(int index);

    /**
     * Get Index
     * @return int
     */
    int GetIndex();

    /**
     * AsynchronousRegister
     */
    void AsynchronousRegister();

private:
    /**
     * Get Core for core service
     * @param slotId
     * @return Core for core service
     */
    std::shared_ptr<Core> GetCore(int32_t slotId);

private:
    CallReportInfo callReportInfo_;
    bool flag_ = false;
    int index_ = 1;
    static constexpr HiviewDFX::HiLogLabel LOG_LABEL = {LOG_CORE, LOG_DOMAIN, "CellularCallConnectionCS"};
};
} // namespace Telephony
} // namespace OHOS

#endif // CELLULAR_CALL_CONNECTION_CS_H