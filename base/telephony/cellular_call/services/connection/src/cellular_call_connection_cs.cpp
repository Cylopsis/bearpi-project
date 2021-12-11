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

#include "cellular_call_connection_cs.h"

#include <securec.h>

#include "core_manager.h"

#include "cellular_call_service.h"

namespace OHOS {
namespace Telephony {
CellularCallConnectionCS::CellularCallConnectionCS()
{
    if (memset_s(&callReportInfo_, sizeof(callReportInfo_), 0, sizeof(callReportInfo_)) != EOK) {
        TELEPHONY_LOGE("ReportUpdateInfo return, memset_s fail.");
    }
}

CellularCallConnectionCS::~CellularCallConnectionCS()
{
    TELEPHONY_LOGD("CellularCallConnectionCS::~CellularCallConnectionCS.");
}

int32_t CellularCallConnectionCS::DialRequest(const DialRequestStruct &dialRequest, int32_t slotId)
{
    auto rilCore = GetCore(slotId);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("DialRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_DIAL);
    if (event == nullptr) {
        TELEPHONY_LOGE("DialRequest return, error type: event is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    if (DelayedSingleton<CellularCallService>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("DialRequest return, error type: GetInstance() is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    event->SetOwner(DelayedSingleton<CellularCallService>::GetInstance()->GetHandler(slotId));
    rilCore->Dial(dialRequest.phoneNum, dialRequest.clirMode, event);

    return TELEPHONY_SUCCESS;
}

int32_t CellularCallConnectionCS::EndRequest(int32_t slotId)
{
    auto rilCore = GetCore(slotId);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("EndRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_HANGUP_CONNECT);
    if (event == nullptr) {
        TELEPHONY_LOGE("EndRequest return, error type: event is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    if (DelayedSingleton<CellularCallService>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("EndRequest return, error type: GetInstance() is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    event->SetOwner(DelayedSingleton<CellularCallService>::GetInstance()->GetHandler(slotId));
    rilCore->Hangup(index_, event);
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallConnectionCS::AnswerRequest(int32_t slotId)
{
    auto rilCore = GetCore(slotId);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("AnswerRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_ACCEPT_CALL);
    if (event == nullptr) {
        TELEPHONY_LOGE("AnswerRequest return, error type: event is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    if (DelayedSingleton<CellularCallService>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("AnswerRequest return, error type: GetInstance() is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    event->SetOwner(DelayedSingleton<CellularCallService>::GetInstance()->GetHandler(slotId));
    rilCore->Answer(event);
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallConnectionCS::RejectRequest(int32_t slotId)
{
    auto rilCore = GetCore(slotId);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("RejectRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_REJECT_CALL);
    if (event == nullptr) {
        TELEPHONY_LOGE("RejectRequest return, error type: event is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    if (DelayedSingleton<CellularCallService>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("RejectRequest return, error type: GetInstance() is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    event->SetOwner(DelayedSingleton<CellularCallService>::GetInstance()->GetHandler(slotId));
    rilCore->Reject(event);
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallConnectionCS::HoldRequest(int32_t slotId)
{
    auto rilCore = GetCore(slotId);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("HoldRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_HOLD_CALL);
    if (event == nullptr) {
        TELEPHONY_LOGE("HoldRequest return, error type: event is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    if (DelayedSingleton<CellularCallService>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("HoldRequest return, error type: GetInstance() is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    event->SetOwner(DelayedSingleton<CellularCallService>::GetInstance()->GetHandler(slotId));
    rilCore->Hold(event);
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallConnectionCS::ActiveRequest(int32_t slotId)
{
    auto rilCore = GetCore(slotId);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("ActiveRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_ACTIVE_CALL);
    if (event == nullptr) {
        TELEPHONY_LOGE("ActiveRequest return, error type: event is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    if (DelayedSingleton<CellularCallService>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("ActiveRequest return, error type: GetInstance() is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    event->SetOwner(DelayedSingleton<CellularCallService>::GetInstance()->GetHandler(slotId));
    rilCore->Active(event);
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallConnectionCS::SwapRequest(int32_t slotId)
{
    auto rilCore = GetCore(slotId);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("SwapRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_SWAP_CALL);
    if (event == nullptr) {
        TELEPHONY_LOGE("SwapRequest return, error type: event is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    if (DelayedSingleton<CellularCallService>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("SwapRequest return, error type: GetInstance() is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    event->SetOwner(DelayedSingleton<CellularCallService>::GetInstance()->GetHandler(slotId));
    rilCore->Swap(event);
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallConnectionCS::JoinRequest(int32_t slotId, int32_t voiceCall)
{
    auto rilCore = GetCore(slotId);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("JoinRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_JOIN_CALL);
    if (event == nullptr) {
        TELEPHONY_LOGE("JoinRequest return, error type: event is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    if (DelayedSingleton<CellularCallService>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("JoinRequest return, error type: GetInstance() is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    event->SetOwner(DelayedSingleton<CellularCallService>::GetInstance()->GetHandler(slotId));
    rilCore->Join(voiceCall, event);
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallConnectionCS::SplitRequest(int32_t index, int32_t slotId, int32_t voiceCall)
{
    auto rilCore = GetCore(slotId);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("SplitRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_SPLIT_CALL);
    if (event == nullptr) {
        TELEPHONY_LOGE("SplitRequest return, error type: event is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    if (DelayedSingleton<CellularCallService>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("SplitRequest return, error type: GetInstance() is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    event->SetOwner(DelayedSingleton<CellularCallService>::GetInstance()->GetHandler(slotId));
    rilCore->Split(index, voiceCall, event);
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallConnectionCS::CallSupplementRequest(CallSupplementType type, int32_t slotId)
{
    auto rilCore = GetCore(slotId);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("CallSupplementRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_CALL_SUPPLEMENT);
    if (event == nullptr) {
        TELEPHONY_LOGE("CallSupplementRequest return, error type: event is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    if (DelayedSingleton<CellularCallService>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("CallSupplementRequest return, error type: GetInstance() is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    event->SetOwner(DelayedSingleton<CellularCallService>::GetInstance()->GetHandler(slotId));
    rilCore->CallSupplement(type, event);
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallConnectionCS::SendCDMAThreeWayDialRequest(int32_t slotId)
{
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallConnectionCS::TransmitDTMFRequest(char cDTMFCode, int32_t index, int32_t slotId)
{
    auto rilCore = GetCore(slotId);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("TransmitDTMFRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_SEND_DTMF);
    if (event == nullptr) {
        TELEPHONY_LOGE("InitiateDTMFRequest return, error type: event is null.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    event->SetOwner(DelayedSingleton<CellularCallService>::GetInstance()->GetHandler(slotId));
    rilCore->SendDtmf(cDTMFCode, index, event);
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallConnectionCS::InitiateDTMFRequest(char cDTMFCode, int32_t index, int32_t slotId)
{
    auto rilCore = GetCore(slotId);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("InitiateDTMFRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_START_DTMF);
    if (event == nullptr) {
        TELEPHONY_LOGE("InitiateDTMFRequest return, error type: event is null.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    event->SetOwner(DelayedSingleton<CellularCallService>::GetInstance()->GetHandler(slotId));
    rilCore->StartDtmf(cDTMFCode, index, event);
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallConnectionCS::CeaseDTMFRequest(int32_t index, int32_t slotId)
{
    auto rilCore = GetCore(slotId);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("CeaseDTMFRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_STOP_DTMF);
    if (event == nullptr) {
        TELEPHONY_LOGE("CeaseDTMFRequest return, error type: event is null.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    event->SetOwner(DelayedSingleton<CellularCallService>::GetInstance()->GetHandler(slotId));
    rilCore->StopDtmf(index, event);
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallConnectionCS::TransmitDTMFStringRequest(
    const std::string &sDTMFCode, int32_t on, int32_t off, int32_t slotId)
{
    auto rilCore = GetCore(slotId);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("TransmitDTMFRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_SEND_DTMF);
    if (event == nullptr) {
        TELEPHONY_LOGE("InitiateDTMFRequest return, error type: event is null.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    event->SetOwner(DelayedSingleton<CellularCallService>::GetInstance()->GetHandler(slotId));
    rilCore->SendDtmf(sDTMFCode, index_, on, off, event);
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallConnectionCS::GetCsCallsDataRequest(int32_t slotId, int64_t lastCallsDataFlag)
{
    auto rilCore = GetCore(slotId);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("CellularCallHandler::GetCsCallsDataRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_CURRENT_CALLS, lastCallsDataFlag);
    if (event == nullptr) {
        TELEPHONY_LOGE("GetCsCallsDataRequest return, error type: event is null.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    event->SetOwner(DelayedSingleton<CellularCallService>::GetInstance()->GetHandler(slotId));

    // Implementation
    // Optional. Recommended when +CHLD command is implemented.
    rilCore->GetCallList(event);
    return TELEPHONY_SUCCESS;
}

std::shared_ptr<Core> CellularCallConnectionCS::GetCore(int32_t slotId)
{
    return CoreManager::GetInstance().getCore(slotId);
}

void CellularCallConnectionCS::SetOrUpdateCallReportInfo(CallReportInfo &callReportInfo)
{
    callReportInfo_ = callReportInfo;
}

CallReportInfo CellularCallConnectionCS::GetCallReportInfo()
{
    return callReportInfo_;
}

void CellularCallConnectionCS::SetStatus(TelCallState state)
{
    callReportInfo_.state = state;
}

TelCallState CellularCallConnectionCS::GetStatus()
{
    return callReportInfo_.state;
}

bool CellularCallConnectionCS::IsRingingState()
{
    return this->GetStatus() == CALL_STATUS_INCOMING || this->GetStatus() == CALL_STATUS_WAITING ||
        this->GetStatus() == CALL_STATUS_ALERTING;
}

void CellularCallConnectionCS::AsynchronousRegister()
{
    DelayedSingleton<CellularCallService>::GetInstance()->RegisterHandler();
}

void CellularCallConnectionCS::SetFlag(bool flag)
{
    flag_ = flag;
}

bool CellularCallConnectionCS::GetFlag()
{
    return flag_;
}

void CellularCallConnectionCS::SetIndex(int index)
{
    index_ = index;
}

int CellularCallConnectionCS::GetIndex()
{
    return index_;
}
} // namespace Telephony
} // namespace OHOS