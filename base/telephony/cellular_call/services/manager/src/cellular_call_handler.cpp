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

#include "cellular_call_handler.h"
#include "cellular_call_register.h"
#include "hril_call_parcel.h"

namespace OHOS {
namespace Telephony {
CellularCallHandler::CellularCallHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{
    requestFuncMap_[ObserverHandler::RADIO_DIAL] = &CellularCallHandler::DialResponse;
    requestFuncMap_[ObserverHandler::RADIO_HANGUP_CONNECT] = &CellularCallHandler::EndResponse;
    requestFuncMap_[ObserverHandler::RADIO_REJECT_CALL] = &CellularCallHandler::RejectResponse;
    requestFuncMap_[ObserverHandler::RADIO_ACCEPT_CALL] = &CellularCallHandler::AcceptResponse;
    requestFuncMap_[ObserverHandler::RADIO_HOLD_CALL] = &CellularCallHandler::HoldResponse;
    requestFuncMap_[ObserverHandler::RADIO_ACTIVE_CALL] = &CellularCallHandler::ActiveResponse;
    requestFuncMap_[ObserverHandler::RADIO_SWAP_CALL] = &CellularCallHandler::SwapResponse;
    requestFuncMap_[ObserverHandler::RADIO_JOIN_CALL] = &CellularCallHandler::JoinResponse;
    requestFuncMap_[ObserverHandler::RADIO_SPLIT_CALL] = &CellularCallHandler::SplitResponse;
    requestFuncMap_[ObserverHandler::RADIO_CALL_SUPPLEMENT] = &CellularCallHandler::SupplementResponse;
    requestFuncMap_[ObserverHandler::RADIO_CALL_STATE] = &CellularCallHandler::GetCallDataResponse;
    requestFuncMap_[ObserverHandler::RADIO_AVAIL] = &CellularCallHandler::GetCallDataResponse;
    requestFuncMap_[ObserverHandler::RADIO_NOT_AVAIL] = &CellularCallHandler::GetCallDataResponse;
    requestFuncMap_[ObserverHandler::RADIO_CURRENT_CALLS] = &CellularCallHandler::DealCallsDataResponse;
    requestFuncMap_[ObserverHandler::RADIO_SEND_DTMF] = &CellularCallHandler::SendDTMFResponse;
    requestFuncMap_[ObserverHandler::RADIO_START_DTMF] = &CellularCallHandler::StartDTMFResponse;
    requestFuncMap_[ObserverHandler::RADIO_STOP_DTMF] = &CellularCallHandler::StopDTMFResponse;
    requestFuncMap_[GET_CALL_DATA_ID] = &CellularCallHandler::DealCallsDataRequest;
    requestFuncMap_[ASYNCHRONOUS_REGISTER_ID] = &CellularCallHandler::AsynchronousRegister;
}

void CellularCallHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("CellularCallHandler::ProcessEvent, event is nullptr");
        return;
    }
    auto itFunc = requestFuncMap_.find(event->GetInnerEventId());
    if (itFunc != requestFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(event);
        }
    }
}

void CellularCallHandler::GetCallDataResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("GetCallDataResponse return, event is nullptr");
        return;
    }
    TELEPHONY_LOGD("GetCallDataResponse event id: %{public}d", event->GetInnerEventId());
    this->SendEvent(GET_CALL_DATA_ID, delayTime_, Priority::HIGH);
}

void CellularCallHandler::DealCallsDataResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("DealCallsDataResponse return, event is nullptr");
        return;
    }
    TELEPHONY_LOGD("DealCallsDataResponse query current calls result entry.");

    // Returns list of current calls of ME. If command succeeds but no calls are available,
    // no information response is sent to TE. Refer subclause 9.2 for possible <err> values.
    auto callInfoList = event->GetSharedObject<CallInfoList>();
    if (callInfoList == nullptr) {
        TELEPHONY_LOGE("DealCallsDataResponse, Cannot get the callInfoList, need to get rilResponseInfo");
        auto rilResponseInfo = event->GetSharedObject<HRilRadioResponseInfo>();
        if (rilResponseInfo == nullptr) {
            TELEPHONY_LOGE("DealCallsDataResponse return, callInfoList and rilResponseInfo is nullptr");
            return;
        }
        if (rilResponseInfo->error == HRilErrType::NONE) {
            TELEPHONY_LOGE("DealCallsDataResponse return, return no error!");
            return;
        }
        TELEPHONY_LOGE("DealCallsDataResponse, return error, report to call_manager");
        CellularCallEventInfo eventInfo;
        eventInfo.eventType = CellularCallEventType::EVENT_REQUEST_RESULT_TYPE;
        eventInfo.eventId = RequestResultEventId::RESULT_GET_CURRENT_CALLS_FAILED;
        if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
            TELEPHONY_LOGE("DealCallsDataResponse return, GetInstance is nullptr");
            return;
        }
        DelayedSingleton<CellularCallRegister>::GetInstance()->ReportEventResultInfo(eventInfo);
        return;
    }
    TELEPHONY_LOGD("DealCallsDataResponse callInfoList->callSize %{public}d ", callInfoList->callSize);
    if (pCSControl_ == nullptr) {
        TELEPHONY_LOGE("DealCallsDataResponse return, pCSControl_ is nullptr");
        return;
    }
    pCSControl_->DealCsCallsData(*callInfoList);
}

void CellularCallHandler::DialResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("DialResponse return, event is nullptr");
        return;
    }
    auto result = event->GetSharedObject<HRilRadioResponseInfo>();
    if (result == nullptr) {
        TELEPHONY_LOGE("DialResponse return, result is nullptr");
        return;
    }
    if (result->error != HRilErrType::NONE) {
        TELEPHONY_LOGE("DialResponse, dial return error, report to call_manager");
        CellularCallEventInfo eventInfo;
        eventInfo.eventType = CellularCallEventType::EVENT_REQUEST_RESULT_TYPE;

        /*
         * 3GPP TS 27.007 V3.9.0 (2001-06)
         * If ME has succeeded in establishing a logical link between application protocols and external interface,
         * it will send CONNECT message to the TE. Otherwise, the NO CARRIER response will be returned.
         */
        if (result->error == HRilErrType::HRIL_ERR_CMD_NO_CARRIER) {
            eventInfo.eventId = RequestResultEventId::RESULT_DIAL_NO_CARRIER;
        } else {
            eventInfo.eventId = RequestResultEventId::RESULT_DIAL_SEND_FAILED;
        }
        if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
            TELEPHONY_LOGE("DialResponse return, GetInstance is nullptr");
            return;
        }
        DelayedSingleton<CellularCallRegister>::GetInstance()->ReportEventResultInfo(eventInfo);
    }
    TELEPHONY_LOGD("DialResponse CanRequestCallsData true");
    lastCallsDataFlag_ = CurrentTimeMillis();
    CellularCallConnectionCS connectionCs;
    connectionCs.GetCsCallsDataRequest(slotId_, lastCallsDataFlag_);
}

void CellularCallHandler::EndResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("EndResponse return, event is nullptr");
        return;
    }
    auto result = event->GetSharedObject<HRilRadioResponseInfo>();
    if (result == nullptr) {
        TELEPHONY_LOGE("EndResponse return, result is nullptr");
        return;
    }
    if (result->error != HRilErrType::NONE) {
        TELEPHONY_LOGE("EndResponse, return error, report to call_manager");
        CellularCallEventInfo eventInfo;
        eventInfo.eventType = CellularCallEventType::EVENT_REQUEST_RESULT_TYPE;
        eventInfo.eventId = RequestResultEventId::RESULT_END_SEND_FAILED;
        if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
            TELEPHONY_LOGE("EndResponse return, GetInstance is nullptr");
            return;
        }
        DelayedSingleton<CellularCallRegister>::GetInstance()->ReportEventResultInfo(eventInfo);
        return;
    }
    GetCallDataResponse(event);
}

void CellularCallHandler::RejectResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("RejectResponse return, event is nullptr");
        return;
    }
    auto result = event->GetSharedObject<HRilRadioResponseInfo>();
    if (result == nullptr) {
        TELEPHONY_LOGE("RejectResponse return, result is nullptr");
        return;
    }
    if (result->error != HRilErrType::NONE) {
        TELEPHONY_LOGE("RejectResponse, return error, report to call_manager");
        CellularCallEventInfo eventInfo;
        eventInfo.eventType = CellularCallEventType::EVENT_REQUEST_RESULT_TYPE;
        eventInfo.eventId = RequestResultEventId::RESULT_REJECT_SEND_FAILED;
        if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
            TELEPHONY_LOGE("RejectResponse return, GetInstance is nullptr");
            return;
        }
        DelayedSingleton<CellularCallRegister>::GetInstance()->ReportEventResultInfo(eventInfo);
        return;
    }
    GetCallDataResponse(event);
}

void CellularCallHandler::AcceptResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("AcceptResponse return, event is nullptr");
        return;
    }
    auto result = event->GetSharedObject<HRilRadioResponseInfo>();
    if (result == nullptr) {
        TELEPHONY_LOGE("AcceptResponse return, result is nullptr");
        return;
    }
    if (result->error != HRilErrType::NONE) {
        TELEPHONY_LOGE("AcceptResponse, return error, report to call_manager");
        CellularCallEventInfo eventInfo;
        eventInfo.eventType = CellularCallEventType::EVENT_REQUEST_RESULT_TYPE;
        eventInfo.eventId = RequestResultEventId::RESULT_ACCEPT_SEND_FAILED;
        if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
            TELEPHONY_LOGE("AcceptResponse return, GetInstance is nullptr");
            return;
        }
        DelayedSingleton<CellularCallRegister>::GetInstance()->ReportEventResultInfo(eventInfo);
        return;
    }
    GetCallDataResponse(event);
}

void CellularCallHandler::HoldResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("HoldResponse return, event is nullptr");
        return;
    }
    auto result = event->GetSharedObject<HRilRadioResponseInfo>();
    if (result == nullptr) {
        TELEPHONY_LOGE("HoldResponse return, result is nullptr");
        return;
    }
    if (result->error != HRilErrType::NONE) {
        TELEPHONY_LOGE("HoldResponse, return error, report to call_manager");
        CellularCallEventInfo eventInfo;
        eventInfo.eventType = CellularCallEventType::EVENT_REQUEST_RESULT_TYPE;
        eventInfo.eventId = RequestResultEventId::RESULT_HOLD_SEND_FAILED;
        if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
            TELEPHONY_LOGE("HoldResponse return, GetInstance is nullptr");
            return;
        }
        DelayedSingleton<CellularCallRegister>::GetInstance()->ReportEventResultInfo(eventInfo);
        return;
    }
    GetCallDataResponse(event);
}

void CellularCallHandler::ActiveResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("ActiveResponse return, event is nullptr");
        return;
    }
    auto result = event->GetSharedObject<HRilRadioResponseInfo>();
    if (result == nullptr) {
        TELEPHONY_LOGE("ActiveResponse return, result is nullptr");
        return;
    }
    if (result->error != HRilErrType::NONE) {
        TELEPHONY_LOGE("ActiveResponse, return error, report to call_manager");
        CellularCallEventInfo eventInfo;
        eventInfo.eventType = CellularCallEventType::EVENT_REQUEST_RESULT_TYPE;
        eventInfo.eventId = RequestResultEventId::RESULT_ACTIVE_SEND_FAILED;
        if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
            TELEPHONY_LOGE("ActiveResponse return, GetInstance is nullptr");
            return;
        }
        DelayedSingleton<CellularCallRegister>::GetInstance()->ReportEventResultInfo(eventInfo);
        return;
    }
    GetCallDataResponse(event);
}

void CellularCallHandler::SwapResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("SwapResponse return, event is nullptr");
        return;
    }
    auto result = event->GetSharedObject<HRilRadioResponseInfo>();
    if (result == nullptr) {
        TELEPHONY_LOGE("SwapResponse return, result is nullptr");
        return;
    }
    if (result->error != HRilErrType::NONE) {
        TELEPHONY_LOGE("SwapResponse, return error, report to call_manager");
        CellularCallEventInfo eventInfo;
        eventInfo.eventType = CellularCallEventType::EVENT_REQUEST_RESULT_TYPE;
        eventInfo.eventId = RequestResultEventId::RESULT_SWAP_SEND_FAILED;
        if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
            TELEPHONY_LOGE("SwapResponse return, GetInstance is nullptr");
            return;
        }
        DelayedSingleton<CellularCallRegister>::GetInstance()->ReportEventResultInfo(eventInfo);
        return;
    }
    GetCallDataResponse(event);
}

void CellularCallHandler::JoinResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("JoinResponse return, event is nullptr");
        return;
    }
    auto result = event->GetSharedObject<HRilRadioResponseInfo>();
    if (result == nullptr) {
        TELEPHONY_LOGE("JoinResponse return, result is nullptr");
        return;
    }
    if (result->error != HRilErrType::NONE) {
        TELEPHONY_LOGE("JoinResponse, return error, report to call_manager");
        CellularCallEventInfo eventInfo;
        eventInfo.eventType = CellularCallEventType::EVENT_REQUEST_RESULT_TYPE;
        eventInfo.eventId = RequestResultEventId::RESULT_JOIN_SEND_FAILED;
        if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
            TELEPHONY_LOGE("JoinResponse return, GetInstance is nullptr");
            return;
        }
        DelayedSingleton<CellularCallRegister>::GetInstance()->ReportEventResultInfo(eventInfo);
        return;
    }
    GetCallDataResponse(event);
}

void CellularCallHandler::SplitResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("SplitResponse return, event is nullptr");
        return;
    }
    auto result = event->GetSharedObject<HRilRadioResponseInfo>();
    if (result == nullptr) {
        TELEPHONY_LOGE("SplitResponse return, result is nullptr");
        return;
    }
    if (result->error != HRilErrType::NONE) {
        TELEPHONY_LOGE("SplitResponse, return error, report to call_manager");
        CellularCallEventInfo eventInfo;
        eventInfo.eventType = CellularCallEventType::EVENT_REQUEST_RESULT_TYPE;
        eventInfo.eventId = RequestResultEventId::RESULT_SPLIT_SEND_FAILED;
        if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
            TELEPHONY_LOGE("SplitResponse return, GetInstance is nullptr");
            return;
        }
        DelayedSingleton<CellularCallRegister>::GetInstance()->ReportEventResultInfo(eventInfo);
        return;
    }
    GetCallDataResponse(event);
}

void CellularCallHandler::SupplementResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("SupplementResponse return, event is nullptr");
        return;
    }
    auto result = event->GetSharedObject<HRilRadioResponseInfo>();
    if (result == nullptr) {
        TELEPHONY_LOGE("SupplementResponse return, result is nullptr");
        return;
    }
    if (result->error != HRilErrType::NONE) {
        TELEPHONY_LOGE("SupplementResponse, return error, report to call_manager");
        CellularCallEventInfo eventInfo;
        eventInfo.eventType = CellularCallEventType::EVENT_REQUEST_RESULT_TYPE;
        eventInfo.eventId = RequestResultEventId::RESULT_SUPPLEMENT_SEND_FAILED;
        if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
            TELEPHONY_LOGE("SupplementResponse return, GetInstance is nullptr");
            return;
        }
        DelayedSingleton<CellularCallRegister>::GetInstance()->ReportEventResultInfo(eventInfo);
        return;
    }
    GetCallDataResponse(event);
}

void CellularCallHandler::SendDTMFResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGD("SendDTMFResponse ObserverHandler::RADIO_SEND_DTMF");
    CellularCallEventInfo eventInfo;
    eventInfo.eventType = CellularCallEventType::EVENT_REQUEST_RESULT_TYPE;
    eventInfo.eventId = RequestResultEventId::RESULT_TRANSMIT_DTMF_SUCCESS;
    if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("SendDTMFResponse return, GetInstance is nullptr");
        return;
    }
    DelayedSingleton<CellularCallRegister>::GetInstance()->ReportEventResultInfo(eventInfo);
}

void CellularCallHandler::StartDTMFResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGD("StartDTMFResponse ObserverHandler::RADIO_START_DTMF");
    CellularCallEventInfo eventInfo;
    eventInfo.eventType = CellularCallEventType::EVENT_REQUEST_RESULT_TYPE;
    eventInfo.eventId = RequestResultEventId::RESULT_INITIATE_DTMF_SUCCESS;
    if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("StartDTMFResponse return, GetInstance is nullptr");
        return;
    }
    DelayedSingleton<CellularCallRegister>::GetInstance()->ReportEventResultInfo(eventInfo);
}

void CellularCallHandler::StopDTMFResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGD("StopDTMFResponse ObserverHandler::RADIO_STOP_DTMF");
    CellularCallEventInfo eventInfo;
    eventInfo.eventType = CellularCallEventType::EVENT_REQUEST_RESULT_TYPE;
    eventInfo.eventId = RequestResultEventId::RESULT_CEASE_DTMF_SUCCESS;
    if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
        TELEPHONY_LOGE("StopDTMFResponse return, GetInstance is nullptr");
        return;
    }
    DelayedSingleton<CellularCallRegister>::GetInstance()->ReportEventResultInfo(eventInfo);
}

void CellularCallHandler::SetSlotId(int32_t id)
{
    slotId_ = id;
}

void CellularCallHandler::SetCSControl(std::shared_ptr<CSControl> pCSControl)
{
    pCSControl_ = pCSControl;
}

std::shared_ptr<CSControl> CellularCallHandler::GetCSControl()
{
    return pCSControl_;
}

int64_t CellularCallHandler::CurrentTimeMillis()
{
    int64_t timems =
        std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
    return timems;
}

bool CellularCallHandler::IsCanRequestCallsData()
{
    int64_t timems = CurrentTimeMillis();
    if ((timems - lastTime_) < FAST_DELAY_TIME) {
        return false;
    }
    lastTime_ = timems;
    return true;
}

void CellularCallHandler::DealCallsDataRequest(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGD("DealCallsDataRequest entry");
    if (!IsCanRequestCallsData()) {
        TELEPHONY_LOGD("DealCallsDataRequest return, IsCanRequestCallsData false");
        return;
    }
    lastCallsDataFlag_ = CurrentTimeMillis();
    CellularCallConnectionCS connectionCs;
    connectionCs.GetCsCallsDataRequest(slotId_, lastCallsDataFlag_);
}

void CellularCallHandler::AsynchronousRegister(const AppExecFwk::InnerEvent::Pointer &event)
{
    TELEPHONY_LOGD("CellularCallHandler::AsynchronousRegister entry.");
    CellularCallConnectionCS connectionCs;
    connectionCs.AsynchronousRegister();
}
} // namespace Telephony
} // namespace OHOS
