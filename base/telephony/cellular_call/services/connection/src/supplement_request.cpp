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

#include "supplement_request.h"
#include "observer_handler.h"

#include "cellular_call_service.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
int32_t SupplementRequest::InquireClipRequest()
{
    auto rilCore = GetCore(slotId_);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("SupplementRequest::InquireClipRequest return, error type: rilCore is nullptr.\n");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_GET_CALL_CLIP);
    if (event == nullptr) {
        TELEPHONY_LOGE("SupplementRequest::InquireClipRequest return, error type: event is nullptr.");
        return ERR_NULL_POINTER;
    }
    event->SetOwner(GetMMIHandler());
    rilCore->GetClip(event);
    return TELEPHONY_SUCCESS;
}

int32_t SupplementRequest::SetClirActivateRequest(int32_t action)
{
    auto rilCore = GetCore(slotId_);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("SetClirActivateRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_SET_CALL_CLIR);
    if (event == nullptr) {
        TELEPHONY_LOGE("SetClirActivateRequest return, error type: event is nullptr.");
        return ERR_NULL_POINTER;
    }
    event->SetOwner(GetMMIHandler());
    rilCore->SetClir(action, event);
    return TELEPHONY_SUCCESS;
}

int32_t SupplementRequest::SetClirDeactivateRequest(int32_t action)
{
    auto rilCore = GetCore(slotId_);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("SetClirDeactivateRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_SET_CALL_CLIR);
    if (event == nullptr) {
        TELEPHONY_LOGE("SetClirDeactivateRequest return, error type: event is nullptr.");
        return ERR_NULL_POINTER;
    }
    event->SetOwner(GetMMIHandler());
    rilCore->SetClir(action, event);
    return TELEPHONY_SUCCESS;
}

int32_t SupplementRequest::InquireClirRequest()
{
    auto rilCore = GetCore(slotId_);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("InquireClirRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_GET_CALL_CLIR);
    if (event == nullptr) {
        TELEPHONY_LOGE("InquireClirRequest return, error type: event is nullptr.");
        return ERR_NULL_POINTER;
    }
    event->SetOwner(GetMMIHandler());
    rilCore->GetClir(event);
    return TELEPHONY_SUCCESS;
}

int32_t SupplementRequest::InquireCallTransferRequest(int type)
{
    auto rilCore = GetCore(slotId_);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("InquireCallTransferRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_GET_CALL_FORWARD);
    if (event == nullptr) {
        TELEPHONY_LOGE("InquireCallTransferRequest return, error type: event is nullptr.");
        return ERR_NULL_POINTER;
    }
    event->SetOwner(GetMMIHandler());
    rilCore->GetCallForward(type, event);
    return TELEPHONY_SUCCESS;
}

int32_t SupplementRequest::SetCallTransferRequest(
    int32_t action, int32_t cause, const std::string &transferPhone, int32_t serviceInfoB)
{
    auto rilCore = GetCore(slotId_);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("SetCallTransferRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_SET_CALL_FORWARD);
    if (event == nullptr) {
        TELEPHONY_LOGE("SetCallTransferRequest return, error type: event is nullptr.");
        return ERR_NULL_POINTER;
    }
    event->SetOwner(GetMMIHandler());
    rilCore->SetCallForward(cause, action, transferPhone, serviceInfoB, event);
    TELEPHONY_LOGD("SetCallTransferRequest end");
    return TELEPHONY_SUCCESS;
}

int32_t SupplementRequest::InquireCallRestriction(const std::string &fac)
{
    auto rilCore = GetCore(slotId_);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("InquireCallRestriction return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_GET_CALL_RESTRICTION);
    if (event == nullptr) {
        TELEPHONY_LOGE("InquireCallRestriction return, error type: event is nullptr.");
        return ERR_NULL_POINTER;
    }
    event->SetOwner(GetMMIHandler());
    rilCore->GetCallRestriction(fac, event);
    return TELEPHONY_SUCCESS;
}

int32_t SupplementRequest::SetCallRestriction(std::string &fac, int32_t mode, std::string &pw)
{
    auto rilCore = GetCore(slotId_);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("SetCallRestriction return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_SET_CALL_RESTRICTION);
    if (event == nullptr) {
        TELEPHONY_LOGE("SetCallRestriction return, error type: event is nullptr.");
        return ERR_NULL_POINTER;
    }
    event->SetOwner(GetMMIHandler());
    rilCore->SetCallRestriction(fac, mode, pw, event);
    return TELEPHONY_SUCCESS;
}

int32_t SupplementRequest::SetCallWaitingRequest(bool activate)
{
    auto rilCore = GetCore(slotId_);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("SetCallWaitingRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_SET_CALL_WAIT);
    if (event == nullptr) {
        TELEPHONY_LOGE("SetCallWaitingRequest return, error type: event is nullptr.");
        return ERR_NULL_POINTER;
    }
    event->SetOwner(GetMMIHandler());
    rilCore->SetCallWait(activate, event);
    return TELEPHONY_SUCCESS;
}

int32_t SupplementRequest::InquireCallWaitingRequest()
{
    auto rilCore = GetCore(slotId_);
    if (rilCore == nullptr) {
        TELEPHONY_LOGE("InquireCallWaitingRequest return, error type: rilCore is nullptr.");
        return ERR_RESOURCE_UNAVAILABLE;
    }
    auto event = AppExecFwk::InnerEvent::Get(ObserverHandler::RADIO_GET_CALL_WAIT);
    if (event == nullptr) {
        TELEPHONY_LOGE("InquireCallWaitingRequest return, error type: event is nullptr.");
        return ERR_NULL_POINTER;
    }
    event->SetOwner(GetMMIHandler());
    rilCore->GetCallWait(event);
    return TELEPHONY_SUCCESS;
}

std::shared_ptr<Core> SupplementRequest::GetCore(int32_t slotId)
{
    return CoreManager::GetInstance().getCore(slotId);
}

std::shared_ptr<MMIHandler> SupplementRequest::GetMMIHandler() const
{
    auto callService = DelayedSingleton<CellularCallService>::GetInstance();
    if (callService == nullptr) {
        TELEPHONY_LOGE("GetMMIHandler return, error type: callService is nullptr.");
        return nullptr;
    }
    if (callService->GetHandler(slotId_) == nullptr) {
        TELEPHONY_LOGE("GetMMIHandler return, error type: GetHandler() is nullptr.");
        return nullptr;
    }
    if (callService->GetHandler(slotId_)->GetCSControl() == nullptr) {
        TELEPHONY_LOGE("GetMMIHandler return, error type: GetCSControl() is nullptr.");
        return nullptr;
    }
    return callService->GetHandler(slotId_)->GetCSControl()->GetMMIHandler();
}
} // namespace Telephony
} // namespace OHOS