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

#include "call_control_manager.h"

#include <securec.h>
#include <string_ex.h>

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

#include "common_type.h"
#include "cs_call.h"
#include "ims_call.h"
#include "ott_call.h"
#include "call_manager_broadcast_subscriber.h"
#include "call_ability_handler.h"
#include "report_call_state_handler.h"
#include "cellular_call_ipc_interface_proxy.h"
#include "audio_control_manager.h"

namespace OHOS {
namespace Telephony {
CallControlManager::CallControlManager() : callStateListenerPtr_(nullptr), callRequestHandlerServicePtr_(nullptr)
{
    dialSrcInfo_.callId = ERR_ID;
    dialSrcInfo_.number = "";
    dialSrcInfo_.extras.Clear();
    dialSrcInfo_.isDialing = false;
}

CallControlManager::~CallControlManager() {}

bool CallControlManager::Init()
{
    callStateListenerPtr_ = std::make_unique<CallStateListener>();
    if (callStateListenerPtr_ == nullptr) {
        TELEPHONY_LOGE("callStateListenerPtr_ is null");
        return false;
    }
    callRequestHandlerServicePtr_ = std::make_unique<CallRequestHandlerService>();
    if (callRequestHandlerServicePtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestHandlerServicePtr_ is null");
        return false;
    }
    callRequestHandlerServicePtr_->Start();

    reportCallStateHandlerPtr_ = (std::make_unique<ReportCallStateHandlerService>()).release();
    if (reportCallStateHandlerPtr_ == nullptr) {
        TELEPHONY_LOGE("reportCallStateHandlerPtr_ is null");
        return false;
    }
    reportCallStateHandlerPtr_->Start();
    // Broadcast subscription
    MatchingSkills matchingSkills;
    matchingSkills.AddEvent(SIM_STATE_UPDATE_ACTION);
    CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    std::shared_ptr<CallManagerBroadcastSubscriber> subscriberPtr =
        std::make_shared<CallManagerBroadcastSubscriber>(subscriberInfo);
    if (subscriberPtr != nullptr) {
        bool subscribeResult = CommonEventManager::SubscribeCommonEvent(subscriberPtr);
        if (!subscribeResult) {
            TELEPHONY_LOGW("SubscribeCommonEvent failed");
        }
    }
    callStateListenerPtr_->AddOneObserver(reportCallStateHandlerPtr_);
#ifdef AUDIO_SUPPORT
    callStateListenerPtr_->AddOneObserver(DelayedSingleton<AudioControlManager>::GetInstance().get());
#endif
    hungUpSms_ = (std::make_unique<HangUpSms>()).release();
    callStateListenerPtr_->AddOneObserver(hungUpSms_);
    callStateListenerPtr_->AddOneObserver(DelayedSingleton<CallAbilityHandlerService>::GetInstance().get());
    return true;
}

int32_t CallControlManager::DialCall(std::u16string &number, AppExecFwk::PacMap &extras)
{
    int32_t ret = CALL_MANAGER_DIAL_FAILED;
    sptr<CallBase> callObjectPtr = nullptr;
    std::string accountNumber(Str16ToStr8(number));
    ret = NumberLegalityCheck(accountNumber);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Invalid number!");
        return ret;
    }
    DialType dialType = (DialType)extras.GetIntValue("dialType");
    if (dialType == DialType::DIAL_UNKNOW_TYPE) {
        TELEPHONY_LOGE("unknow dial type!");
        return CALL_MANAGER_UNKNOW_DIAL_TYPE;
    }
    int32_t accountId = extras.GetIntValue("accountId");
    ret = DialPolicy(accountId);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("dial policy result：%{public}d", ret);
        return ret;
    }
    // temporarily save dial information
    {
        std::lock_guard<std::mutex> lock(mutex_);
        dialSrcInfo_.callId = ERR_ID;
        dialSrcInfo_.number = accountNumber;
        dialSrcInfo_.extras.Clear();
        dialSrcInfo_.extras = extras;
        dialSrcInfo_.isDialing = true;
    }
    if (callRequestHandlerServicePtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestHandlerServicePtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
    ret = callRequestHandlerServicePtr_->DialCall();
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("DialCall failed!");
        return ret;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallControlManager::AnswerCall(int32_t callId, int32_t videoState)
{
    int32_t ret = CALL_MANAGER_ACCEPT_FAILED;
    ret = AnswerCallPolicy(callId);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("AnswerCallPolicy failed!");
        return ret;
    }
    if (callRequestHandlerServicePtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestHandlerServicePtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
    ret = callRequestHandlerServicePtr_->AnswerCall(callId, videoState);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("AnswerCall failed!");
        return ret;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallControlManager::RejectCall(int32_t callId, bool rejectWithMessage, std::u16string textMessage)
{
    int32_t ret = CALL_MANAGER_REJECT_FAILED;
    if (callRequestHandlerServicePtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestHandlerServicePtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
    ret = RejectCallPolicy(callId);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("RejectCallPolicy failed!");
        return ret;
    }
    std::string messageStr(Str16ToStr8(textMessage));
    ret = callRequestHandlerServicePtr_->RejectCall(callId, rejectWithMessage, messageStr);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("RejectCall failed!");
        return ret;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallControlManager::HangUpCall(int32_t callId)
{
    int32_t ret = CALL_MANAGER_HANGUP_FAILED;
    if (callRequestHandlerServicePtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestHandlerServicePtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
    ret = HangUpPolicy(callId);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("HangUpPolicy failed!");
        return ret;
    }
    ret = callRequestHandlerServicePtr_->HangUpCall(callId);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("HangUpCall failed!");
        return ret;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallControlManager::GetCallState()
{
    CallStateToApp callState = CallStateToApp::CALL_STATE_UNKNOWN;
    if (!HasCallExist()) {
        callState = CallStateToApp::CALL_STATE_IDLE;
    } else {
        callState = CallStateToApp::CALL_STATE_OFFHOOK;
        if (HasRingingCall()) {
            callState = CallStateToApp::CALL_STATE_RINGING;
        }
    }
    return (int32_t)callState;
}

int32_t CallControlManager::HoldCall(int32_t callId)
{
    int32_t ret = CALL_MANAGER_HOLD_FAILED;
    ret = HoldCallPolicy(callId);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("HoldCall failed!");
        return ret;
    }
    if (callRequestHandlerServicePtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestHandlerServicePtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
    ret = callRequestHandlerServicePtr_->HoldCall(callId);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("HoldCall failed!");
        return ret;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallControlManager::UnHoldCall(const int32_t callId)
{
    int32_t ret = CALL_MANAGER_UNHOLD_FAILED;
    ret = UnHoldCallPolicy(callId);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("UnHoldCall failed!");
        return ret;
    }
    if (callRequestHandlerServicePtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestHandlerServicePtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
    ret = callRequestHandlerServicePtr_->UnHoldCall(callId);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("UnHoldCall failed!");
        return ret;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallControlManager::SwitchCall()
{
    int32_t ret = CALL_MANAGER_SWAP_FAILED;
    int32_t callId = TELEPHONY_FAIL;
    ret = SwitchCallPolicy(callId);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("SwitchCall failed!");
        return ret;
    }
    if (callRequestHandlerServicePtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestHandlerServicePtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
    ret = callRequestHandlerServicePtr_->SwitchCall(callId);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("SwitchCall failed!");
        return ret;
    }
    return TELEPHONY_SUCCESS;
}

bool CallControlManager::HasCall()
{
    return HasCallExist();
}

bool CallControlManager::IsNewCallAllowed()
{
    return IsNewCallAllowedCreate();
}

bool CallControlManager::IsRinging()
{
    return HasRingingCall();
}

bool CallControlManager::HasEmergency()
{
    return HasEmergencyCall();
}

bool CallControlManager::NotifyNewCallCreated(sptr<CallBase> &callObjectPtr)
{
    if (callObjectPtr == nullptr) {
        TELEPHONY_LOGE("callObjectPtr is null!");
        return false;
    }
    if (callStateListenerPtr_ != nullptr) {
        callStateListenerPtr_->NewCallCreated(callObjectPtr);
    }
    return true;
}

bool CallControlManager::NotifyCallDestroyed(sptr<CallBase> &callObjectPtr)
{
    if (callObjectPtr == nullptr) {
        TELEPHONY_LOGE("callObjectPtr is null!");
        return false;
    }
    if (callStateListenerPtr_ != nullptr) {
        callStateListenerPtr_->CallDestroyed(callObjectPtr);
        return true;
    }
    return false;
}

bool CallControlManager::NotifyCallStateUpdated(
    sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState)
{
    if (callObjectPtr == nullptr) {
        TELEPHONY_LOGE("callObjectPtr is null!");
        return false;
    }
    if (callStateListenerPtr_ != nullptr) {
        callStateListenerPtr_->CallStateUpdated(callObjectPtr, priorState, nextState);
        return true;
    }
    return false;
}

bool CallControlManager::NotifyIncomingCallAnswered(sptr<CallBase> &callObjectPtr)
{
    if (callObjectPtr == nullptr) {
        TELEPHONY_LOGE("callObjectPtr is null!");
        return false;
    }
    if (callStateListenerPtr_ != nullptr) {
        callStateListenerPtr_->IncomingCallActivated(callObjectPtr);
        return true;
    }
    return false;
}

bool CallControlManager::NotifyIncomingCallRejected(
    sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content)
{
    if (callObjectPtr == nullptr) {
        TELEPHONY_LOGE("callObjectPtr is null!");
        return false;
    }
    if (callStateListenerPtr_ != nullptr) {
        callStateListenerPtr_->IncomingCallHungUp(callObjectPtr, isSendSms, content);
        return true;
    }
    return false;
}

bool CallControlManager::NotifyCallEventUpdated(CallEventInfo &info)
{
    if (callStateListenerPtr_ != nullptr) {
        callStateListenerPtr_->CallEventUpdated(info);
        return true;
    }
    return false;
}

int32_t CallControlManager::StartDtmf(int32_t callId, char str)
{
    int32_t ret;
    sptr<CallBase> call = GetOneCallObject(callId);
    if (call == nullptr) {
        return CALL_MANAGER_CALL_NULL;
    }
    if (!call->IsAliveState()) {
        return CALL_MANAGER_CALL_STATE_MISMATCH_OPERATION;
    }
    std::string phoneNum = call->GetAccountNumber();
    ret = call->StartDtmf(phoneNum, str);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("StartDtmf failed, return:%{public}d", ret);
    }
    return ret;
}

int32_t CallControlManager::SendDtmf(int32_t callId, char str)
{
    int32_t ret;
    sptr<CallBase> call = GetOneCallObject(callId);
    if (call == nullptr) {
        return CALL_MANAGER_CALL_NULL;
    }
    if (!call->IsAliveState()) {
        return CALL_MANAGER_CALL_STATE_MISMATCH_OPERATION;
    }
    std::string phoneNum = call->GetAccountNumber();
    ret = call->SendDtmf(phoneNum, str);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("SendDtmf failed, return:%{public}d", ret);
    }
    return ret;
}

int32_t CallControlManager::SendBurstDtmf(int32_t callId, std::u16string str, int32_t on, int32_t off)
{
    int32_t ret;
    std::string strCode = Str16ToStr8(str);
    sptr<CallBase> call = GetOneCallObject(callId);
    if (call == nullptr) {
        return CALL_MANAGER_CALL_NULL;
    }
    if (!((on == DtmfPlaytime::DTMF_PLAY_TONE_MSEC_0) || (on == DtmfPlaytime::DTMF_PLAY_TONE_MSEC_1) ||
        (on == DtmfPlaytime::DTMF_PLAY_TONE_MSEC_2) || (on == DtmfPlaytime::DTMF_PLAY_TONE_MSEC_3) ||
        (on == DtmfPlaytime::DTMF_PLAY_TONE_MSEC_4) || (on == DtmfPlaytime::DTMF_PLAY_TONE_MSEC_5) ||
        (on == DtmfPlaytime::DTMF_PLAY_TONE_MSEC_6) || (on == DtmfPlaytime::DTMF_PLAY_TONE_MSEC_7))) {
        return CALL_MANAGER_DTMF_PARAMETER_INVALID;
    }
    if ((off < DtmfPlayIntervalTime::DTMF_PLAY_TONE_MIN_INTERVAL_MSEC) ||
        (off > DtmfPlayIntervalTime::DTMF_PLAY_TONE_MAX_INTERVAL_MSEC)) {
        off = DtmfPlayIntervalTime::DTMF_PLAY_TONE_DEFAULT_INTERVAL_MSEC;
    }
    if (on == DtmfPlaytime::DTMF_PLAY_TONE_MSEC_1) {
        on = DtmfPlaytime::DTMF_PLAY_TONE_DEFAULT_MSEC;
    }
    if (strCode.empty()) {
        return CALL_MANAGER_SEND_DTMF_INPUT_IS_EMPTY;
    }
    if (!call->IsAliveState()) {
        return CALL_MANAGER_CALL_STATE_MISMATCH_OPERATION;
    }
    std::string phoneNum = call->GetAccountNumber();
    ret = call->SendBurstDtmf(phoneNum, strCode, on, off);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("SendBurstDtmf failed, return:%{public}d", ret);
    }
    return ret;
}

int32_t CallControlManager::StopDtmf(int32_t callId)
{
    int32_t ret = TELEPHONY_SUCCESS;
    sptr<CallBase> call = GetOneCallObject(callId);
    if (call == nullptr) {
        return CALL_MANAGER_CALL_NULL;
    }
    if (!call->IsAliveState()) {
        return CALL_MANAGER_CALL_STATE_MISMATCH_OPERATION;
    }
    std::string phoneNum = call->GetAccountNumber();
    ret = call->StopDtmf(phoneNum);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("StopDtmf failed, return:%{public}d", ret);
    }
    return ret;
}

int32_t CallControlManager::CombineConference(int32_t mainCallId)
{
    int32_t ret = TELEPHONY_ERROR;
    sptr<CallBase> mainCall = GetOneCallObject(mainCallId);
    if (mainCall == nullptr) {
        TELEPHONY_LOGW("GetOneCallObject failed, mainCallId:%{public}d", mainCallId);
        return CALL_MANAGER_CALL_NULL;
    }
    if (mainCall->GetTelCallState() != TelCallState::CALL_STATUS_ACTIVE) {
        TELEPHONY_LOGE("mainCall state should be active ");
        return CALL_MANAGER_CALL_STATE_MISMATCH_OPERATION;
    }
    ret = mainCall->CanCombineConference();
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGW("CanCombineConference failed");
        return ret;
    }
    if (!CallObjectManager::IsCallExist(mainCall->GetCallType(), TelCallState::CALL_STATUS_HOLDING)) {
        TELEPHONY_LOGE("callType:%{public}d,callState:CALL_STATUS_HOLDING is not exist!", mainCall->GetCallType());
        return CALL_MANAGER_CALL_STATE_MISMATCH_OPERATION;
    }
    if (callRequestHandlerServicePtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestHandlerServicePtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
    ret = callRequestHandlerServicePtr_->CombineConference(mainCallId);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("CombineConference failed!");
        return ret;
    }
    return ret;
}

int32_t CallControlManager::SeparateConference(int32_t callId)
{
    int32_t ret = TELEPHONY_SUCCESS;
    sptr<CallBase> call = GetOneCallObject(callId);
    RETURN_FAILURE_IF_NULLPTR(call, "call is NULL!", CALL_MANAGER_CALL_NULL);
    ret = call->CanSeparateConference();
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGW("CanSeparateConference failed");
        return ret;
    }
    if (callRequestHandlerServicePtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestHandlerServicePtr_ is nullptr!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
    ret = callRequestHandlerServicePtr_->SeparateConference(callId);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("CombineConference failed!");
        return ret;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallControlManager::GetMainCallId(int32_t callId)
{
    sptr<CallBase> call = GetOneCallObject(callId);
    RETURN_FAILURE_IF_NULLPTR(call, "call is NULL!", ERR_ID);
    return call->GetMainCallId();
}

std::vector<std::u16string> CallControlManager::GetSubCallIdList(int32_t callId)
{
    sptr<CallBase> call = GetOneCallObject(callId);
    if (call == nullptr) {
        std::vector<std::u16string> vec;
        vec.clear();
        return vec;
    }
    return call->GetSubCallIdList();
}

std::vector<std::u16string> CallControlManager::GetCallIdListForConference(int32_t callId)
{
    sptr<CallBase> call = GetOneCallObject(callId);
    if (call == nullptr) {
        std::vector<std::u16string> vec;
        vec.clear();
        return vec;
    }
    return call->GetCallIdListForConference();
}

int32_t CallControlManager::DialProcess()
{
    int32_t ret = CALL_MANAGER_DIAL_FAILED;
    DialType dialType = DialType::DIAL_UNKNOW_TYPE;
    // determine whether the device is in dialing state
    {
        std::lock_guard<std::mutex> lock(mutex_);
        if (!dialSrcInfo_.isDialing) {
            TELEPHONY_LOGE("the device is not dialing!");
            return ret;
        }
        dialType = (DialType)dialSrcInfo_.extras.GetIntValue("dialType");
    }
    switch (dialType) {
        case DialType::DIAL_CARRIER_TYPE:
            ret = CarrierDialProcess();
            break;
        case DialType::DIAL_VOICE_MAIL_TYPE:
            ret = VoiceMailDialProcess();
            break;
        case DialType::DIAL_OTT_TYPE:
            ret = OttDialProcess();
            break;
        default:
            TELEPHONY_LOGE("invalid dialType:%{public}d", dialType);
            break;
    }
    return ret;
}

int32_t CallControlManager::CreateNewCall(const CallReportInfo &info)
{
    int32_t ret = CALL_MANAGER_CREATE_CALL_OBJECT_FAIL;
    if (info.state != TelCallState::CALL_STATUS_DIALING) {
        TELEPHONY_LOGE("invalid call state:%{public}d", info.state);
        return ret;
    }
    sptr<CallBase> callPtr = nullptr;
    switch (info.callType) {
        case CallType::TYPE_CS: {
            std::unique_ptr<CSCall> csCall = std::make_unique<CSCall>();
            if (csCall != nullptr) {
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    csCall->OutCallInit(info, dialSrcInfo_.extras, GetNewCallId());
                }
                callPtr = csCall.release();
                AddOneCallObject(callPtr);
            }
            break;
        }
        case CallType::TYPE_IMS: {
            std::unique_ptr<IMSCall> imsCall = std::make_unique<IMSCall>();
            if (imsCall != nullptr) {
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    imsCall->OutCallInit(info, dialSrcInfo_.extras, GetNewCallId());
                }
                callPtr = imsCall.release();
                AddOneCallObject(callPtr);
            }
            break;
        }
        case CallType::TYPE_OTT: {
            std::unique_ptr<OTTCall> ottCall = std::make_unique<OTTCall>();
            if (ottCall != nullptr) {
                {
                    std::lock_guard<std::mutex> lock(mutex_);
                    ottCall->OutCallInit(info, dialSrcInfo_.extras, GetNewCallId());
                }
                callPtr = ottCall.release();
                AddOneCallObject(callPtr);
            }
            break;
        }
        default:
            TELEPHONY_LOGE("Invalid call type!");
            return CALL_MANAGER_CREATE_CALL_OBJECT_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallControlManager::NumberLegalityCheck(std::string &number)
{
    if (number.empty()) {
        TELEPHONY_LOGE("phone number is NULL!");
        return CALL_MANAGER_PHONE_NUMBER_NULL;
    }
    if (number.length() > kMaxNumberLen) {
        TELEPHONY_LOGE(
            "the number length exceeds limit,len:%{public}zu,maxLen:%{public}d", number.length(), kMaxNumberLen);
        return CALL_MANAGER_PHONE_BEYOND;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallControlManager::CarrierDialProcess()
{
    int32_t ret = CALL_MANAGER_DIAL_FAILED;
    DialScene dialScene = (DialScene)dialSrcInfo_.extras.GetIntValue("dialScene");
    if (dialScene != CALL_NORMAL && dialScene != CALL_PRIVILEGED && dialScene != CALL_EMERGENCY) {
        TELEPHONY_LOGE("dialScene incorrect!");
        return CALL_MANAGER_DIAL_SCENE_INCORRECT;
    }
    CellularCallInfo callInfo;
    PackCellularCallInfo(callInfo);
    // Obtain gateway information
    ret = DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->Dial(callInfo);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Dial failed!");
        return CALL_MANAGER_DIAL_FAILED;
    }
    return ret;
}

int32_t CallControlManager::VoiceMailDialProcess()
{
    int32_t ret = CALL_MANAGER_DIAL_FAILED;
    CellularCallInfo callInfo;
    PackCellularCallInfo(callInfo);
    DialScene dialScene = (DialScene)dialSrcInfo_.extras.GetIntValue("dialScene");
    if (dialScene == CALL_NORMAL || dialScene == CALL_PRIVILEGED) {
        TELEPHONY_LOGE("this is voice mail");
        ret = DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->Dial(callInfo);
    }
    TELEPHONY_LOGE("dialScene is invalid");
    return ret;
}

int32_t CallControlManager::OttDialProcess()
{
    return CALL_MANAGER_DIAL_FAILED;
}

void CallControlManager::PackCellularCallInfo(CellularCallInfo &callInfo)
{
    std::lock_guard<std::mutex> lock(mutex_);
    callInfo.callId = dialSrcInfo_.callId;
    callInfo.accountId = dialSrcInfo_.extras.GetIntValue("accountId");
    callInfo.callType = CallType::TYPE_CS;
    callInfo.videoState = dialSrcInfo_.extras.GetIntValue("videoState");
    callInfo.index = ERR_ID;
    callInfo.slotId = callInfo.accountId;
    (void)memset_s(callInfo.phoneNum, kMaxNumberLen, 0, kMaxNumberLen);
    if (memcpy_s(callInfo.phoneNum, kMaxNumberLen, dialSrcInfo_.number.c_str(), dialSrcInfo_.number.length()) != 0) {
        TELEPHONY_LOGW("memcpy_s failed!");
        return;
    }
}
} // namespace Telephony
} // namespace OHOS
