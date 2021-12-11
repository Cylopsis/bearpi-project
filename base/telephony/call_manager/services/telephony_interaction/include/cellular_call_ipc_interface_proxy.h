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

#ifndef CELLULAR_CALL_IPC_INTERFACE_PROXY_H
#define CELLULAR_CALL_IPC_INTERFACE_PROXY_H

#include <mutex>

#include "if_system_ability_manager.h"
#include "refbase.h"
#include "rwlock.h"
#include "singleton.h"

#include "call_status_callback.h"
#include "cellular_call_interface.h"
#include "i_call_status_callback.h"
#include "timer.h"

namespace OHOS {
namespace Telephony {
class CellularCallIpcInterfaceProxy : public Timer,
                                      public std::enable_shared_from_this<CellularCallIpcInterfaceProxy> {
    DECLARE_DELAYED_SINGLETON(CellularCallIpcInterfaceProxy)
public:
    void Init(int32_t systemAbilityId);
    void unInit();
    static void task();
    /**
     * Call management diale interface
     *
     * @param CallInfo, dial param.
     * @return Returns 0 on success, others on failure.
     */
    int Dial(const CellularCallInfo &callInfo);

    /**
     * End.
     *
     * @param CallInfo, End param.
     * @return Returns 0 on success, others on failure.
     */
    int End(const CellularCallInfo &callInfo);

    /**
     * Reject.
     *
     * @param CallInfo, Reject param.
     * @return Returns 0 on success, others on failure.
     */
    int Reject(const CellularCallInfo &callInfo);

    /**
     * Answer.
     *
     * @param CallInfo, Answer param.
     * @return Returns 0 on success, others on failure.
     */
    int Answer(const CellularCallInfo &callInfo);

    /**
     * Hold.
     *
     * @param CellularCallInfo, Hold param.
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int Hold(const CellularCallInfo &callInfo);

    /**
     * Active.
     *
     * @param CellularCallInfo, Active param.
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int Active(const CellularCallInfo &callInfo);

    /**
     * Swap.
     *
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int Swap();

    /**
     * @brief Is it an emergency call
     * @param string &phoneNum
     * @param slotId
     * @return Returns 0 on success, others on failure.
     */
    int IsUrgentCall(const std::string &phoneNum, int32_t slotId, int32_t &errorCode);

    /**
     * Merge into multiple calls
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int Join();

    /**
     * Split.
     *
     * @param std::string splitString
     * @param index
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int Split(const std::string &splitString, int32_t index);

    /**
     * InitiateDTMF.
     *
     * @param DTMF Code.
     * @param phoneNum.
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int InitiateDTMF(char cDTMFCode, const std::string &phoneNum);

    /**
     * CeaseDTMF.
     *
     * @param phoneNum.
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int CeaseDTMF(const std::string &phoneNum);

    /**
     * TransmitDTMF.
     *
     * @param DTMF Code.
     * @param phoneNum.
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int TransmitDTMF(char cDTMFCode, const std::string &phoneNum);

    /**
     * Send DTMF String.
     * @param DTMF Code string.
     * @param phoneNum.
     * @param switchOn.
     * @param switchOff.
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int TransmitDTMFString(const std::string &dtmfCodeStr, const std::string &phoneNum, PhoneNetType phoneNetType,
        int32_t switchOn, int32_t switchOff);

    /**
     * Set Call Forwarding
     * @param CallForwardInfo
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int SetCallTransfer(const CallTransferInfo &info, int32_t slotId);

    /**
     * Inquire Call Forwarding
     * @param CallForwardInfo
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int InquireCallTransfer(CallTransferType type, int32_t slotId);

    /**
     * Set Call Waiting
     * @param activate
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int SetCallWaiting(bool activate, int32_t slotId);

    /**
     * Inquire Call Waiting
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int InquireCallWaiting(int32_t slotId);

    /**
     * Set Call Barring
     * @param CallBarringInfo
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int SetCallRestriction(const CallRestrictionInfo &info, int32_t slotId);

    /**
     * Inquire Call Barring
     * @param CallBarringInfo
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int InquireCallRestriction(CallRestrictionType facType, int32_t slotId);

    /**
     * RegisterCallBack
     *
     * @return Returns 0 on success, others on failure.
     */
    int RegisterCallBack(const sptr<ICallStatusCallback> &callback);

private:
    int32_t ConnectService();
    int32_t RegisterCallBackFun();
    void DisconnectService();
    int32_t ReConnectService();
    void OnDeath();
    void Clean();
    void NotifyDeath();

private:
    int32_t systemAbilityId_;
    sptr<ICallStatusCallback> cellularCallCallbackPtr_;
    sptr<CellularCallInterface> cellularCallInterfacePtr_;
    sptr<IRemoteObject::DeathRecipient> cellularCallRecipient_;
    bool connectState_;
    Utils::RWLock rwClientLock_;
    std::mutex mutex_;
};
} // namespace Telephony
} // namespace OHOS

#endif