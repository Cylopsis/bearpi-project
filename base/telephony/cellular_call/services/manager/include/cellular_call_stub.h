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

#ifndef CELLULAR_CALL_STUB_H
#define CELLULAR_CALL_STUB_H

#include "cellular_call_data_struct.h"
#include "cellular_call_interface.h"
#include "telephony_log_wrapper.h"
#include "iremote_stub.h"
#include "cs_control.h"

namespace OHOS {
namespace Telephony {
class CellularCallStub : public IRemoteStub<CellularCallInterface> {
public:
    /**
     * CellularCallStub Constructors
     */
    CellularCallStub();

    /**
     * ~CellularCallStub Destructor
     */
    virtual ~CellularCallStub();

    /**
     * OnRemoteRequest
     *
     * @param code
     * @param data
     * @param reply
     * @param option
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    /**
     * dial
     *
     * @param CellularCallInfo, dial param.
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int Dial(const CellularCallInfo &callInfo) override;

    /**
     * End.
     *
     * @param CellularCallInfo, End param.
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int End(const CellularCallInfo &callInfo) override;

    /**
     * Reject.
     *
     * @param CellularCallInfo, Reject param.
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int Reject(const CellularCallInfo &callInfo) override;

    /**
     * Answer.
     *
     * @param CellularCallInfo, Answer param.
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int Answer(const CellularCallInfo &callInfo) override;

    /**
     * Hold.
     *
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int Hold() override;

    /**
     * Active.
     *
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int Active() override;

    /**
     * Swap.
     *
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int Swap() override;

    /**
     * IsUrgentCall.
     *
     * @param phone number.
     * @param slotId.
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int IsUrgentCall(const std::string &phoneNum, int32_t slotId, int32_t &errorCode) override;

    /**
     * Merge into multiple calls
     *
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int Join() override;

    /**
     * Split.
     *
     * @param std::string splitString
     * @param index
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int Split(const std::string &splitString, int32_t index) override;

    /**
     * CallSupplement.
     *
     * @param CallSupplementType
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int CallSupplement(CallSupplementType type) override;

    /**
     * Register CallManager CallBack
     *
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int RegisterCallManagerCallBack(const sptr<ICallStatusCallback> &callback) override;

    /**
     * UnRegister CallManager CallBack
     *
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int UnRegisterCallManagerCallBack() override;

    /**
     * InitiateDTMF.
     *
     * @param DTMF Code.
     * @param phoneNum.
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int InitiateDTMF(char cDTMFCode, const std::string &phoneNum) override;

    /**
     * CeaseDTMF.
     *
     * @param phoneNum.
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int CeaseDTMF(const std::string &phoneNum) override;

    /**
     * TransmitDTMF.
     *
     * @param DTMF Code.
     * @param phoneNum.
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int TransmitDTMF(char cDTMFCode, const std::string &phoneNum) override;

    /**
     * Send DTMF String.
     * @param DTMF Code string.
     * @param phoneNum.
     * @param switchOn.
     * @param switchOff.
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t TransmitDTMFString(
        const std::string &dtmfCodeStr, const std::string &phoneNum, int32_t switchOn, int32_t switchOff) override;

    /**
     * Set Call Transfer
     * @param CallTransferInfo
     * @param slot Id
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t SetCallTransfer(const CallTransferInfo &cfInfo, int32_t slotId) override;

    /**
     * Inquire Call Transfer
     * @param CallTransferType
     * @param slot Id
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t InquireCallTransfer(CallTransferType type, int32_t slotId) override;

    /**
     * Set Call Waiting
     * @param activate
     * @param slot Id
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t SetCallWaiting(bool activate, int32_t slotId) override;

    /**
     * Inquire Call Waiting
     * @param slot Id
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t InquireCallWaiting(int32_t slotId) override;

    /**
     * Set Call Restriction
     * @param CallRestrictionInfo
     * @param slot Id
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t SetCallRestriction(const CallRestrictionInfo &crInfo, int32_t slotId) override;

    /**
     * Inquire Call Restriction
     * @param CallRestrictionType
     * @param slot Id
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t InquireCallRestriction(CallRestrictionType facType, int32_t slotId) override;

    /**
     * Get CsControl
     * @param slotId
     * @return shared_ptr CSControl
     */
    std::shared_ptr<CSControl> GetCsControl(int32_t slotId);

private:
    /**
     * Dial Inner.
     * @param data send data
     * @param reply Received data
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int DialInner(MessageParcel &data, MessageParcel &reply);

    /**
     * End Inner.
     * @param data send data
     * @param reply Received data
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int EndInner(MessageParcel &data, MessageParcel &reply);

    /**
     * Reject Inner.
     * @param data send data
     * @param reply Received data
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int RejectInner(MessageParcel &data, MessageParcel &reply);

    /**
     * Answer Inner.
     * @param data send data
     * @param reply Received data
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int AnswerInner(MessageParcel &data, MessageParcel &reply);

    /**
     * Is Emergency Call Inner.
     * @param data  send data
     * @param reply Received data
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int IsUrgentCallInner(MessageParcel &data, MessageParcel &reply);

    /**
     * Hold Inner.
     * @param data send data
     * @param reply Received data
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int HoldInner(MessageParcel &data, MessageParcel &reply);

    /**
     * Active Inner.
     * @param data send data
     * @param reply Received data
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int ActiveInner(MessageParcel &data, MessageParcel &reply);

    /**
     * Swap Inner.
     * @param data send data
     * @param reply Received data
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int SwapInner(MessageParcel &data, MessageParcel &reply);

    /**
     * Join Inner.
     * @param data  send data
     * @param reply Received data
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int JoinInner(MessageParcel &data, MessageParcel &reply);

    /**
     * Split Inner.
     * @param data  send data
     * @param reply Received data
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int SplitInner(MessageParcel &data, MessageParcel &reply);

    /**
     * CallSupplement Inner.
     * @param data  send data
     * @param reply Received data
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int CallSupplementInner(MessageParcel &data, MessageParcel &reply);

    /**
     * Register Call Back Inner.
     * @param data send data
     * @param reply Received data
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int RegisterCallBackInner(MessageParcel &data, MessageParcel &reply);

    /**
     * UnRegister Call Back Inner.
     * @param data send data
     * @param reply Received data
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int UnRegisterCallBackInner(MessageParcel &data, MessageParcel &reply);

    /**
     * start DTMF Inner.
     * @param data  send data
     * @param reply Received data
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int InitiateDTMFInner(MessageParcel &data, MessageParcel &reply);

    /**
     * stop DTMF Inner.
     * @param data  send data
     * @param reply Received data
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int CeaseDTMFInner(MessageParcel &data, MessageParcel &reply);

    /**
     * send DTMF Inner.
     * @param data  send data
     * @param reply Received data
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int TransmitDTMFInner(MessageParcel &data, MessageParcel &reply);

    /**
     * send DTMF String Inner.
     * @param data  send data
     * @param reply Received data
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int TransmitDTMFStringInner(MessageParcel &data, MessageParcel &reply);

    /**
     * Set Call Transfer Inner.
     * @param data send data
     * @param reply Received data
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int SetCallTransferInner(MessageParcel &data, MessageParcel &reply);

    /**
     * Inquire Call Transfer Inner.
     * @param data send data
     * @param reply Received data
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int InquireCallTransferInner(MessageParcel &data, MessageParcel &reply);

    /**
     * Set Call Waiting
     * @param data  send data
     * @param reply Received data
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t SetCallWaitingInner(MessageParcel &data, MessageParcel &reply);

    /**
     * Inquire Call Waiting
     * @param data  send data
     * @param reply Received data
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t InquireCallWaitingInner(MessageParcel &data, MessageParcel &reply);

    /**
     * Set Call Restriction
     * @param data send data
     * @param reply Received data
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t SetCallRestrictionInner(MessageParcel &data, MessageParcel &reply);

    /**
     * Inquire Call Restriction
     * @param data send data
     * @param reply Received data
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t InquireCallRestrictionInner(MessageParcel &data, MessageParcel &reply);

    /**
     * init Control
     */
    void Init();

    /**
     * Is Valid Slot Id
     * @param slotId
     * @return bool
     */
    bool IsValidSlotId(int32_t slotId) const;

private:
    using RequestFuncType = int (CellularCallStub::*)(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, RequestFuncType> requestFuncMap_;
    std::map<int32_t, std::shared_ptr<CSControl>> controlMap_;
    int32_t slotId_ = CoreManager::DEFAULT_SLOT_ID;
    static constexpr HiviewDFX::HiLogLabel LOG_LABEL = {LOG_CORE, LOG_DOMAIN, "CellularCallStub"};
};
} // namespace Telephony
} // namespace OHOS

#endif // CELLULAR_CALL_STUB_H
