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

#include "report_call_state_handler.h"

#include <string_ex.h>

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
ReportCallStateHandler::ReportCallStateHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner),
      reportCallStateClientPtr_(DelayedSingleton<CallStateReportProxy>::GetInstance())
{}

ReportCallStateHandler::~ReportCallStateHandler()
{
    if (reportCallStateClientPtr_ != nullptr) {
        reportCallStateClientPtr_ = nullptr;
    }
}

void ReportCallStateHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("ReportCallStateHandler::ProcessEvent parameter error");
        return;
    }
    TELEPHONY_LOGD("ProcessEvent inner event id obtained: %{public}u.", event->GetInnerEventId());
    switch (event->GetInnerEventId()) {
        case ReportCallStateHandlerService::HANDLER_REPORT_CALL_STATE: {
            auto object = event->GetUniqueObject<CallAttributeInfo>();
            if (object == nullptr) {
                TELEPHONY_LOGE("object is nullptr!");
                return;
            }
            int32_t callState = (int32_t)(*object).callState;
            std::string str((*object).accountNumber);
            std::u16string accountNumber = Str8ToStr16(str);
            if (reportCallStateClientPtr_ == nullptr) {
                TELEPHONY_LOGE("reportCallStateClientPtr_ is nullptr!");
                return;
            }
            TELEPHONY_LOGD("ReportCallState");
            reportCallStateClientPtr_->ReportCallState(callState, accountNumber);
            break;
        }
        case ReportCallStateHandlerService::HANDLER_REPORT_CALL_STATE_FOR_CALLID: {
            auto object = event->GetUniqueObject<CallAttributeInfo>();
            if (object == nullptr) {
                TELEPHONY_LOGE("object is nullptr!");
                return;
            }
            int32_t subId = (int32_t)(*object).accountId;
            int32_t callId = (int32_t)(*object).callId;
            int32_t callState = (int32_t)(*object).callState;
            std::string str((*object).accountNumber);
            std::u16string incomingNumber = Str8ToStr16(str);
            if (reportCallStateClientPtr_ == nullptr) {
                TELEPHONY_LOGE("reportCallStateClientPtr_ is nullptr!");
                return;
            }
            TELEPHONY_LOGD("ReportCallStateForCallId");
            reportCallStateClientPtr_->ReportCallStateForCallId(subId, callId, callState, incomingNumber);
            break;
        }
        default: {
            TELEPHONY_LOGE("unknown data!");
            break;
        }
    }
}

ReportCallStateHandlerService::ReportCallStateHandlerService() : eventLoop_(nullptr), handler_(nullptr) {}

ReportCallStateHandlerService::~ReportCallStateHandlerService()
{
    if (eventLoop_ != nullptr) {
        eventLoop_->Stop();
        eventLoop_ = nullptr;
    }
    if (handler_ != nullptr) {
        handler_ = nullptr;
    }
}

void ReportCallStateHandlerService::Start()
{
    eventLoop_ = AppExecFwk::EventRunner::Create("ReportCallStateHandlerService");
    if (eventLoop_.get() == nullptr) {
        TELEPHONY_LOGE("failed to create EventRunner");
        return;
    }
    handler_ = std::make_shared<ReportCallStateHandler>(eventLoop_);
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("failed to create ReportCallStateHandler");
        return;
    }
    eventLoop_->Run();
    return;
}

void ReportCallStateHandlerService::CallStateUpdated(
    sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState)
{
    if (callObjectPtr == nullptr) {
        TELEPHONY_LOGE("callObjectPtr is nullptr!");
        return;
    }
    CallAttributeInfo info;
    callObjectPtr->GetCallAttributeInfo(info);
    if (info.callState == TelCallState::CALL_STATUS_INCOMING) {
        ReportCallStateForCallId(info);
    } else {
        ReportCallState(info);
    }
}

int32_t ReportCallStateHandlerService::ReportCallState(CallAttributeInfo &info)
{
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("failed to create EventRunner");
        return TELEPHONY_FAIL;
    }
    std::unique_ptr<CallAttributeInfo> para = std::make_unique<CallAttributeInfo>();
    if (para.get() == nullptr) {
        TELEPHONY_LOGE("make_unique CallAttributeInfo failed!");
        return TELEPHONY_FAIL;
    }
    *para = info;
    std::lock_guard<std::mutex> lock(mutex_);
    handler_->SendEvent(HANDLER_REPORT_CALL_STATE, std::move(para));
    return TELEPHONY_SUCCESS;
}

int32_t ReportCallStateHandlerService::ReportCallStateForCallId(CallAttributeInfo &info)
{
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("failed to create EventRunner");
        return TELEPHONY_FAIL;
    }
    std::unique_ptr<CallAttributeInfo> para = std::make_unique<CallAttributeInfo>();
    if (para.get() == nullptr) {
        TELEPHONY_LOGE("make_unique CallAttributeInfo failed!");
        return TELEPHONY_FAIL;
    }
    *para = info;
    std::lock_guard<std::mutex> lock(mutex_);
    handler_->SendEvent(HANDLER_REPORT_CALL_STATE_FOR_CALLID, std::move(para));
    return TELEPHONY_SUCCESS;
}
} // namespace Telephony
} // namespace OHOS
