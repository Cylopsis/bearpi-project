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

#include "mmi_handler.h"
#include "cellular_call_data_struct.h"
#include "cellular_call_supplement.h"
#include "hril_call_parcel.h"
#include "hril_types.h"
#include "mmi_code_utils.h"
#include "observer_handler.h"

namespace OHOS {
namespace Telephony {
MMIHandler::MMIHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner) : AppExecFwk::EventHandler(runner)
{
    requestFuncMap_[MMIHandlerId::EVENT_MMI_Id] = &MMIHandler::GetMMIResponse;
    requestFuncMap_[ObserverHandler::RADIO_GET_CALL_WAIT] = &MMIHandler::GetCallWaitResponse;
    requestFuncMap_[ObserverHandler::RADIO_SET_CALL_WAIT] = &MMIHandler::SetCallWaitResponse;
    requestFuncMap_[ObserverHandler::RADIO_GET_CALL_FORWARD] = &MMIHandler::GetCallTransferResponse;
    requestFuncMap_[ObserverHandler::RADIO_SET_CALL_FORWARD] = &MMIHandler::SetCallTransferResponse;
    requestFuncMap_[ObserverHandler::RADIO_GET_CALL_CLIP] = &MMIHandler::GetClipResponse;
    requestFuncMap_[ObserverHandler::RADIO_GET_CALL_CLIR] = &MMIHandler::GetClirResponse;
    requestFuncMap_[ObserverHandler::RADIO_SET_CALL_CLIR] = &MMIHandler::SetClirResponse;
    requestFuncMap_[ObserverHandler::RADIO_GET_CALL_RESTRICTION] = &MMIHandler::GetCallRestrictionResponse;
    requestFuncMap_[ObserverHandler::RADIO_SET_CALL_RESTRICTION] = &MMIHandler::SetCallRestrictionResponse;
}

void MMIHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("MMIHandler::ProcessEvent, event is nullptr");
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

void MMIHandler::GetMMIResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("GetMMIResponse, event is nullptr");
        return;
    }
    std::unique_ptr<MMICodeUtils> mmiCodeUtils = event->GetUniqueObject<MMICodeUtils>();
    if (mmiCodeUtils == nullptr) {
        TELEPHONY_LOGE("MMIHandler::ProcessEvent, mmiCodeUtils is nullptr");
        return;
    }
    mmiCodeUtils->ExecuteMmiCode();
}

void MMIHandler::GetCallWaitResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("GetCallWaitResponse, event is nullptr");
        return;
    }
    TELEPHONY_LOGD("MMIHandler::GetCallWaitResponse, RADIO_GET_CALL_WAIT");
    auto callWaitingInfo = event->GetSharedObject<CallWaitResult>();
    if (callWaitingInfo == nullptr) {
        TELEPHONY_LOGE("MMIHandler::GetCallWaitResponse, callWaitingInfo is nullptr");
        return;
    }
    CellularCallSupplement supplement;
    supplement.EventInquireCallWait(*callWaitingInfo);
}

void MMIHandler::SetCallWaitResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("SetCallWaitResponse, event is nullptr");
        return;
    }
    TELEPHONY_LOGD("SetCallWaitResponse, RADIO_SET_CALL_WAIT");
    auto result = event->GetSharedObject<HRilRadioResponseInfo>();
    CellularCallSupplement supplement;
    supplement.EventSetCallWait(*result);
}

void MMIHandler::GetClirResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("GetClirResponse, event is nullptr");
        return;
    }
    auto getClirResult = event->GetSharedObject<GetClirResult>();
    if (getClirResult == nullptr) {
        TELEPHONY_LOGE("GetClirResponse, getClirResult is nullptr");
        return;
    }
    CellularCallSupplement supplement;
    supplement.EventGetClir(*getClirResult);
}

void MMIHandler::SetClirResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("SetClirResponse, event is nullptr");
        return;
    }
    auto result = event->GetSharedObject<HRilRadioResponseInfo>();
    if (result == nullptr) {
        TELEPHONY_LOGE("SetClirResponse, result is nullptr");
        return;
    }
    CellularCallSupplement supplement;
    supplement.EventSetClir(*result);
}

void MMIHandler::GetClipResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("GetClipResponse, event is nullptr");
        return;
    }
    auto getClipResult = event->GetSharedObject<GetClipResult>();
    if (getClipResult == nullptr) {
        TELEPHONY_LOGE("GetClipResponse, getClipResult is nullptr");
        return;
    }
    CellularCallSupplement supplement;
    supplement.EventGetClip(*getClipResult);
}

void MMIHandler::GetCallTransferResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("GetCallTransferResponse, event is nullptr");
        return;
    }
    auto cFQueryResult = event->GetSharedObject<CallForwardQueryResult>();
    if (cFQueryResult == nullptr) {
        TELEPHONY_LOGE("GetCallTransferResponse, cFQueryResult is nullptr");
        return;
    }
    CellularCallSupplement supplement;
    supplement.EventInquireCallTransfer(*cFQueryResult);
}

void MMIHandler::SetCallTransferResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("SetCallTransferResponse, event is nullptr");
        return;
    }
    auto result = event->GetSharedObject<HRilRadioResponseInfo>();
    if (result == nullptr) {
        TELEPHONY_LOGE("SetCallTransferResponse, result is nullptr");
        return;
    }
    CellularCallSupplement supplement;
    supplement.EventSetCallTransfer(*result);
}

void MMIHandler::GetCallRestrictionResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("GetCallRestrictionResponse, event is nullptr");
        return;
    }
    auto result = event->GetSharedObject<CallRestrictionResult>();
    if (result == nullptr) {
        TELEPHONY_LOGE("GetCallRestrictionResponse, result is nullptr");
        return;
    }
    CellularCallSupplement supplement;
    supplement.EventGetCallRestriction(*result);
}

void MMIHandler::SetCallRestrictionResponse(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("SetCallRestrictionResponse, event is nullptr");
        return;
    }
    auto result = event->GetSharedObject<HRilRadioResponseInfo>();
    if (result == nullptr) {
        TELEPHONY_LOGE("SetCallRestrictionResponse, result is nullptr");
        return;
    }
    CellularCallSupplement supplement;
    supplement.EventSetCallRestriction(*result);
}
} // namespace Telephony
} // namespace OHOS