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

#include "carrier_call.h"

#include <securec.h>

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

#include "call_number_utils.h"
#include "cellular_call_ipc_interface_proxy.h"

namespace OHOS {
namespace Telephony {
CarrierCall::CarrierCall()
    : dialScene_(CALL_NORMAL), slotId_(0), index_(ERR_ID), isEcc_(false), isDefault_(true),
      isVoicemailNumber_(false)
{}

CarrierCall::~CarrierCall() {}

void CarrierCall::InitCarrierOutCallInfo(const CallReportInfo &info, AppExecFwk::PacMap &extras, int32_t callId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    dialScene_ = (DialScene)extras.GetIntValue("dialScene");
    slotId_ = extras.GetIntValue("accountId");
    isEcc_ = false;
    isDefault_ = true; // judging by the package name of the dial application
    isVoicemailNumber_ = false;
    if ((DialType)extras.GetIntValue("dialType") == DialType::DIAL_VOICE_MAIL_TYPE) {
        isVoicemailNumber_ = true;
    }
    InitOutCallBase(info, extras, callId);
}

void CarrierCall::InitCarrierInCallInfo(const CallReportInfo &info, int32_t callId)
{
    std::lock_guard<std::mutex> lock(mutex_);
    accountNumber_ = info.accountNum;
    slotId_ = info.accountId;
    index_ = info.index;
    InitInCallBase(info, callId);
}

int32_t CarrierCall::CarrierDialingProcess()
{
    TELEPHONY_LOGE("start");
    int32_t ret = CALL_MANAGER_DIAL_FAILED;
    int32_t errorCode = TELEPHONY_FAIL;
    bool isEcc = DelayedSingleton<CallNumberUtils>::GetInstance()->CheckNumberIsEmergency(
        accountNumber_, slotId_, errorCode);
    {
        std::lock_guard<std::mutex> lock(mutex_);
        isEcc_ = isEcc;
    }
    ret = DialCallBase();
    if (ret != TELEPHONY_SUCCESS) {
        CarrierHangUpCall();
    }
    TELEPHONY_LOGE("end");
    return ret;
}

int32_t CarrierCall::CarrierAcceptCall(int32_t videoState)
{
    int32_t ret = CALL_MANAGER_ACCEPT_FAILED;
    CellularCallInfo callInfo;
    AcceptCallBase();

    PackCellularCallInfo(callInfo);
    ret = DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->Answer(callInfo);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Accept failed!");
        return CALL_MANAGER_ACCEPT_FAILED;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CarrierCall::CarrierRejectCall(bool isSendSms, std::string &content)
{
    int32_t ret = CALL_MANAGER_REJECT_FAILED;
    CellularCallInfo callInfo;
    ret = RejectCallBase();
    if (ret != TELEPHONY_SUCCESS) {
        return ret;
    }
    PackCellularCallInfo(callInfo);
    ret = DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->Reject(callInfo);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Reject failed!");
        return CALL_MANAGER_REJECT_FAILED;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CarrierCall::CarrierHangUpCall()
{
    int32_t ret = CALL_MANAGER_HANGUP_FAILED;
    CellularCallInfo callInfo;
    PackCellularCallInfo(callInfo);
    ret = DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->End(callInfo);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("End failed!");
        return CALL_MANAGER_HANGUP_FAILED;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CarrierCall::CarrierHoldCall()
{
    int32_t ret = CALL_MANAGER_HANGUP_FAILED;
    CellularCallInfo callInfo;
    PackCellularCallInfo(callInfo);
    ret = DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->Hold(callInfo);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Hold failed!");
        return CALL_MANAGER_HOLD_FAILED;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CarrierCall::CarrierUnHoldCall()
{
    int32_t ret = CALL_MANAGER_HANGUP_FAILED;
    CellularCallInfo callInfo;
    PackCellularCallInfo(callInfo);
    ret = DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->Active(callInfo);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("UnHold failed!");
        return CALL_MANAGER_UNHOLD_FAILED;
    }
    return TELEPHONY_SUCCESS;
}

void CarrierCall::GetCallAttributeCarrierInfo(CallAttributeInfo &info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    info.isEcc = isEcc_;
    info.accountId = slotId_;
    GetCallAttributeBaseInfo(info);
}

bool CarrierCall::GetEmergencyState()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return isEcc_;
}

int32_t CarrierCall::CarrierSwitchCall()
{
    int32_t ret = CALL_MANAGER_SWAP_FAILED;
    ret = DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->Swap();
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("UnHold failed!");
        return CALL_MANAGER_UNHOLD_FAILED;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CarrierCall::GetCallTransferInfo()
{
    return TELEPHONY_SUCCESS;
}

int32_t CarrierCall::SetCallTransferInfo()
{
    return TELEPHONY_SUCCESS;
}

int32_t CarrierCall::GetSlotId()
{
    return slotId_;
}

int32_t CarrierCall::CarrierCombineConference()
{
    return DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->Join();
}

void CarrierCall::PackCellularCallInfo(CellularCallInfo &callInfo)
{
    callInfo.callId = callId_;
    callInfo.callType = callType_;
    callInfo.videoState = (int32_t)videoState_;
    callInfo.index = index_;
    callInfo.slotId = slotId_;
    (void)memset_s(callInfo.phoneNum, kMaxNumberLen, 0, kMaxNumberLen);
    if (memcpy_s(callInfo.phoneNum, kMaxNumberLen, accountNumber_.c_str(), accountNumber_.length()) != 0) {
        TELEPHONY_LOGW("memcpy_s failed!");
        return;
    }
}

int32_t CarrierCall::StartDtmf(std::string &phoneNum, char str)
{
    return DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->InitiateDTMF(str, phoneNum);
}

int32_t CarrierCall::StopDtmf(std::string &phoneNum)
{
    return DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->CeaseDTMF(phoneNum);
}

int32_t CarrierCall::SendDtmf(std::string &phoneNum, char str)
{
    return DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->TransmitDTMF(str, phoneNum);
}

int32_t CarrierCall::SendBurstDtmf(std::string &phoneNum, std::string str, int32_t on, int32_t off)
{
    PhoneNetType phoneNetType = PhoneNetType::PHONE_TYPE_CDMA;
    return DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->TransmitDTMFString(
        str, phoneNum, phoneNetType, on, off);
}
} // namespace Telephony
} // namespace OHOS