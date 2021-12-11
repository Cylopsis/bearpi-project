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

#include "call_ability_handler.h"

#include <string_ex.h>

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
CallAbilityHandler::CallAbilityHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{}

CallAbilityHandler::~CallAbilityHandler() {}

void CallAbilityHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("CallAbilityHandler::ProcessEvent parameter error");
        return;
    }
    TELEPHONY_LOGD("ProcessEvent inner event id obtained: %{public}u.", event->GetInnerEventId());
    switch (event->GetInnerEventId()) {
        case CallAbilityHandlerService::HANDLER_REPORT_CALL_STATE_INFO: {
            auto object = event->GetUniqueObject<CallAttributeInfo>();
            if (object == nullptr) {
                TELEPHONY_LOGE("object is nullptr!");
                return;
            }
            CallAttributeInfo info = *object;
            TELEPHONY_LOGD("ReportCallStateInfo");
            DelayedSingleton<CallAbilityReportIpcProxy>::GetInstance()->ReportCallStateInfo(info);
            break;
        }
        case CallAbilityHandlerService::HANDLER_REPORT_CALL_EVENT: {
            auto object = event->GetUniqueObject<CallEventInfo>();
            if (object == nullptr) {
                TELEPHONY_LOGE("object is nullptr!");
                return;
            }
            CallEventInfo info = *object;
            TELEPHONY_LOGD("ReportCallEvent");
            DelayedSingleton<CallAbilityReportIpcProxy>::GetInstance()->ReportCallEvent(info);
            break;
        }
        default: {
            TELEPHONY_LOGE("unknown data!");
            break;
        }
    }
}

CallAbilityHandlerService::CallAbilityHandlerService() : eventLoop_(nullptr), handler_(nullptr) {}

CallAbilityHandlerService::~CallAbilityHandlerService()
{
    if (eventLoop_ != nullptr) {
        eventLoop_->Stop();
        eventLoop_ = nullptr;
    }
    if (handler_ != nullptr) {
        handler_ = nullptr;
    }
}

void CallAbilityHandlerService::Start()
{
    eventLoop_ = AppExecFwk::EventRunner::Create("CallAbilityHandlerService");
    if (eventLoop_.get() == nullptr) {
        TELEPHONY_LOGE("failed to create EventRunner");
        return;
    }
    handler_ = std::make_shared<CallAbilityHandler>(eventLoop_);
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("failed to create CallAbilityHandler");
        return;
    }
    eventLoop_->Run();
}

void CallAbilityHandlerService::CallStateUpdated(
    sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState)
{
    if (callObjectPtr == nullptr) {
        TELEPHONY_LOGE("callObjectPtr is nullptr!");
        return;
    }
    CallAttributeInfo info;
    callObjectPtr->GetCallAttributeInfo(info);
    ReportCallStateInfo(info);
}

void CallAbilityHandlerService::CallEventUpdated(CallEventInfo &info)
{
    ReportCallEvent(info);
}

int32_t CallAbilityHandlerService::ReportCallStateInfo(CallAttributeInfo &info)
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
    handler_->SendEvent(HANDLER_REPORT_CALL_STATE_INFO, std::move(para));
    return TELEPHONY_SUCCESS;
}

int32_t CallAbilityHandlerService::ReportCallEvent(CallEventInfo &info)
{
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("failed to create EventRunner");
        return TELEPHONY_FAIL;
    }
    std::unique_ptr<CallEventInfo> para = std::make_unique<CallEventInfo>();
    if (para.get() == nullptr) {
        TELEPHONY_LOGE("make_unique CallEventInfo failed!");
        return TELEPHONY_FAIL;
    }
    *para = info;
    std::lock_guard<std::mutex> lock(mutex_);
    handler_->SendEvent(HANDLER_REPORT_CALL_EVENT, std::move(para));
    return TELEPHONY_SUCCESS;
}
} // namespace Telephony
} // namespace OHOS