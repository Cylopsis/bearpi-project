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

#include "cellular_call_info_handler.h"

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
CellularCallInfoHandler::CellularCallInfoHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{
    memberFuncMap_[CellularCallInfoHandlerService::HANDLER_UPDATE_CELLULAR_CALL_INFO] =
        &CellularCallInfoHandler::ReportCallInfo;
    memberFuncMap_[CellularCallInfoHandlerService::HANDLER_UPDATE_CELLULAR_CS_CALL_INFO] =
        &CellularCallInfoHandler::ReportCallsInfo;
    memberFuncMap_[CellularCallInfoHandlerService::HANDLER_UPDATE_CELLULAR_EVENT_RESULT_INFO] =
        &CellularCallInfoHandler::ReportEventInfo;
}

CellularCallInfoHandler::~CellularCallInfoHandler()
{
    if (callStatusManagerPtr_) {
        callStatusManagerPtr_ = nullptr;
    }
}

void CellularCallInfoHandler::Init()
{
    callStatusManagerPtr_ = std::make_unique<CallStatusManager>();
    if (callStatusManagerPtr_ == nullptr) {
        TELEPHONY_LOGE("callStatusManagerPtr_ is null");
        return;
    }
    callStatusManagerPtr_->Init();
}

void CellularCallInfoHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("CellularCallInfoHandler::ProcessEvent parameter error");
        return;
    }
    TELEPHONY_LOGD("CellularCallInfoHandler::ProcessEvent inner event id: %{public}u.", event->GetInnerEventId());
    auto itFunc = memberFuncMap_.find(event->GetInnerEventId());
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(event);
        }
    }
}

void CellularCallInfoHandler::ReportCallInfo(const AppExecFwk::InnerEvent::Pointer &event)
{
    auto object = event->GetUniqueObject<CallReportInfo>();
    if (object == nullptr) {
        TELEPHONY_LOGE("object is nullptr!");
        return;
    }
    CallReportInfo info = *object;
    if (callStatusManagerPtr_ == nullptr) {
        TELEPHONY_LOGE("callStatusManagerPtr_ is nullptr");
        return;
    }
    int32_t ret = callStatusManagerPtr_->HandleCallReportInfo(info);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("HandleCallReportInfo failed! ret:%{public}d", ret);
        return;
    }
    TELEPHONY_LOGD("HandleCallReportInfo success!");
}

void CellularCallInfoHandler::ReportCallsInfo(const AppExecFwk::InnerEvent::Pointer &event)
{
    auto object = event->GetUniqueObject<CallsReportInfo>();
    if (object == nullptr) {
        TELEPHONY_LOGE("object is nullptr!");
        return;
    }
    CallsReportInfo info = *object;
    if (callStatusManagerPtr_ == nullptr) {
        TELEPHONY_LOGE("callStatusManagerPtr_ is nullptr");
        return;
    }
    int32_t ret = callStatusManagerPtr_->HandleCallsReportInfo(info);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("HandleCallsReportInfo failed! ret:%{public}d", ret);
        return;
    }
    TELEPHONY_LOGD("HandleCallsReportInfo success!");
}

void CellularCallInfoHandler::ReportEventInfo(const AppExecFwk::InnerEvent::Pointer &event)
{
    auto object = event->GetUniqueObject<CellularCallEventInfo>();
    if (object == nullptr) {
        TELEPHONY_LOGE("object is nullptr!");
        return;
    }
    CellularCallEventInfo info = *object;
    if (callStatusManagerPtr_ == nullptr) {
        TELEPHONY_LOGE("callStatusManagerPtr_ is nullptr");
        return;
    }
    int32_t ret = callStatusManagerPtr_->HandleEventResultReportInfo(info);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("HandleEventResultReportInfo failed! ret:%{public}d", ret);
        return;
    }
    TELEPHONY_LOGD("HandleEventResultReportInfo success!");
}

CellularCallInfoHandlerService::CellularCallInfoHandlerService() : eventLoop_(nullptr), handler_(nullptr) {}

CellularCallInfoHandlerService::~CellularCallInfoHandlerService()
{
    if (eventLoop_ != nullptr) {
        eventLoop_->Stop();
        eventLoop_ = nullptr;
    }
    if (handler_ != nullptr) {
        handler_ = nullptr;
    }
}

void CellularCallInfoHandlerService::Start()
{
    eventLoop_ = AppExecFwk::EventRunner::Create("CellularCallInfoHandlerService");
    if (eventLoop_.get() == nullptr) {
        TELEPHONY_LOGE("failed to create EventRunner");
        return;
    }
    handler_ = std::make_shared<CellularCallInfoHandler>(eventLoop_);
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("failed to create CellularCallInfoHandler");
        return;
    }
    handler_->Init();
    eventLoop_->Run();
    return;
}

int32_t CellularCallInfoHandlerService::UpdateCallReportInfo(const CallReportInfo &info)
{
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("handler_ is nullptr");
        return TELEPHONY_FAIL;
    }
    std::unique_ptr<CallReportInfo> para = std::make_unique<CallReportInfo>();
    if (para.get() == nullptr) {
        TELEPHONY_LOGE("make_unique CallReportInfo failed!");
        return TELEPHONY_FAIL;
    }
    *para = info;
    std::lock_guard<std::mutex> lock(mutex_);
    bool ret = handler_->SendEvent(HANDLER_UPDATE_CELLULAR_CALL_INFO, std::move(para));
    if (!ret) {
        TELEPHONY_LOGE("SendEvent failed! status update failed, state:%{public}d", info.state);
    }
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallInfoHandlerService::UpdateCallsReportInfo(const CallsReportInfo &info)
{
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("handler_ is nullptr");
        return TELEPHONY_FAIL;
    }
    std::unique_ptr<CallsReportInfo> para = std::make_unique<CallsReportInfo>();
    if (para.get() == nullptr) {
        TELEPHONY_LOGE("make_unique CallsReportInfo failed!");
        return TELEPHONY_FAIL;
    }
    *para = info;
    std::lock_guard<std::mutex> lock(mutex_);
    bool ret = handler_->SendEvent(HANDLER_UPDATE_CELLULAR_CS_CALL_INFO, std::move(para));
    if (!ret) {
        TELEPHONY_LOGE("SendEvent failed! status update failed, slotId:%{public}d", info.slotId);
    }
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallInfoHandlerService::UpdateEventResultInfo(const CellularCallEventInfo &info)
{
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("handler_ is nullptr");
        return TELEPHONY_FAIL;
    }
    std::unique_ptr<CellularCallEventInfo> para = std::make_unique<CellularCallEventInfo>();
    if (para.get() == nullptr) {
        TELEPHONY_LOGE("make_unique CellularCallEventInfo failed!");
        return TELEPHONY_FAIL;
    }
    *para = info;
    std::lock_guard<std::mutex> lock(mutex_);
    bool ret = handler_->SendEvent(HANDLER_UPDATE_CELLULAR_EVENT_RESULT_INFO, std::move(para));
    if (!ret) {
        TELEPHONY_LOGE("SendEvent failed! eventType:%{public}d, eventId:%{public}d", info.eventType, info.eventId);
    }
    return TELEPHONY_SUCCESS;
}
} // namespace Telephony
} // namespace OHOS
