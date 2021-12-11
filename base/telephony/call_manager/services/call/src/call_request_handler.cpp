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

#include "call_request_handler.h"

#include <securec.h>
#include <string_ex.h>

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
CallRequestHandler::CallRequestHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner), callRequestProcessPtr_(nullptr)
{
    memberFuncMap_[CallRequestHandlerService::HANDLER_DIAL_CALL_REQUEST] = &CallRequestHandler::DialCallEvent;
    memberFuncMap_[CallRequestHandlerService::HANDLER_ANSWER_CALL_REQUEST] = &CallRequestHandler::AcceptCallEvent;
    memberFuncMap_[CallRequestHandlerService::HANDLER_REJECT_CALL_REQUEST] = &CallRequestHandler::RejectCallEvent;
    memberFuncMap_[CallRequestHandlerService::HANDLER_HANGUP_CALL_REQUEST] = &CallRequestHandler::HangUpCallEvent;
    memberFuncMap_[CallRequestHandlerService::HANDLER_HOLD_CALL_REQUEST] = &CallRequestHandler::HoldCallEvent;
    memberFuncMap_[CallRequestHandlerService::HANDLER_UNHOLD_CALL_REQUEST] = &CallRequestHandler::UnHoldCallEvent;
    memberFuncMap_[CallRequestHandlerService::HANDLER_SWAP_CALL_REQUEST] = &CallRequestHandler::SwitchCallEvent;
    memberFuncMap_[CallRequestHandlerService::HANDLER_COMBINE_CONFERENCE_REQUEST] =
        &CallRequestHandler::CombineConferenceEvent;
    memberFuncMap_[CallRequestHandlerService::HANDLER_SEPARATE_CONFERENCE_REQUEST] =
        &CallRequestHandler::SeparateConferenceEvent;
}

CallRequestHandler::~CallRequestHandler()
{
    if (callRequestProcessPtr_ != nullptr) {
        callRequestProcessPtr_ = nullptr;
    }
}

void CallRequestHandler::Init()
{
    callRequestProcessPtr_ = std::make_unique<CallRequestProcess>();
    if (callRequestProcessPtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestProcessPtr_ is null");
        return;
    }
}
void CallRequestHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("CallRequestHandler::ProcessEvent parameter error");
        return;
    }
    TELEPHONY_LOGD("CallRequestHandler inner event id obtained: %{public}u.", event->GetInnerEventId());
    auto itFunc = memberFuncMap_.find(event->GetInnerEventId());
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(event);
        }
    }
}

void CallRequestHandler::DialCallEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("CallRequestHandler::ProcessEvent parameter error");
        return;
    }
    if (callRequestProcessPtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestProcessPtr_ is nullptr");
        return;
    }
    callRequestProcessPtr_->DialRequest();
}

void CallRequestHandler::AcceptCallEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("CallRequestHandler::ProcessEvent parameter error");
        return;
    }
    auto object = event->GetUniqueObject<AnswerCallPara>();
    if (object == nullptr) {
        TELEPHONY_LOGE("object is nullptr!");
        return;
    }
    AnswerCallPara acceptPara = *object;
    if (callRequestProcessPtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestProcessPtr_ is nullptr");
        return;
    }
    callRequestProcessPtr_->AnswerRequest(acceptPara.callId, acceptPara.videoState);
}

void CallRequestHandler::RejectCallEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("CallRequestHandler::ProcessEvent parameter error");
        return;
    }
    auto object = event->GetUniqueObject<RejectCallPara>();
    if (object == nullptr) {
        TELEPHONY_LOGE("object is nullptr!");
        return;
    }
    RejectCallPara rejectPara = *object;
    if (callRequestProcessPtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestProcessPtr_ is nullptr");
        return;
    }
    std::string content(rejectPara.content);
    callRequestProcessPtr_->RejectRequest(rejectPara.callId, rejectPara.isSendSms, content);
}

void CallRequestHandler::HangUpCallEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("CallRequestHandler::ProcessEvent parameter error");
        return;
    }
    auto object = event->GetUniqueObject<int32_t>();
    if (object == nullptr) {
        TELEPHONY_LOGE("object is nullptr!");
        return;
    }
    int32_t callId = *object;
    if (callRequestProcessPtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestProcessPtr_ is nullptr");
        return;
    }
    callRequestProcessPtr_->HangUpRequest(callId);
}

void CallRequestHandler::HoldCallEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("CallRequestHandler::ProcessEvent parameter error");
        return;
    }
    auto object = event->GetUniqueObject<int32_t>();
    if (object == nullptr) {
        TELEPHONY_LOGE("object is nullptr!");
        return;
    }
    int32_t callId = *object;
    if (callRequestProcessPtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestProcessPtr_ is nullptr");
        return;
    }
    callRequestProcessPtr_->HoldRequest(callId);
}

void CallRequestHandler::UnHoldCallEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("CallRequestHandler::ProcessEvent parameter error");
        return;
    }
    auto object = event->GetUniqueObject<int32_t>();
    if (object == nullptr) {
        TELEPHONY_LOGE("object is nullptr!");
        return;
    }
    int32_t callId = *object;
    if (callRequestProcessPtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestProcessPtr_ is nullptr");
        return;
    }
    callRequestProcessPtr_->UnHoldRequest(callId);
}

void CallRequestHandler::SwitchCallEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("CallRequestHandler::ProcessEvent parameter error");
        return;
    }
    auto object = event->GetUniqueObject<int32_t>();
    if (object == nullptr) {
        TELEPHONY_LOGE("object is nullptr!");
        return;
    }
    int32_t callId = *object;
    if (callRequestProcessPtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestProcessPtr_ is nullptr");
        return;
    }
    callRequestProcessPtr_->SwitchRequest(callId);
}

void CallRequestHandler::CombineConferenceEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("CallRequestHandler::ProcessEvent parameter error");
        return;
    }
    auto object = event->GetUniqueObject<int32_t>();
    if (object == nullptr) {
        TELEPHONY_LOGE("object is nullptr!");
        return;
    }
    int32_t mainCallId = *object;
    if (callRequestProcessPtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestProcessPtr_ is nullptr");
        return;
    }
    callRequestProcessPtr_->CombineConferenceRequest(mainCallId);
}

void CallRequestHandler::SeparateConferenceEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("CallRequestHandler::ProcessEvent parameter error");
        return;
    }
    auto object = event->GetUniqueObject<int32_t>();
    if (object == nullptr) {
        TELEPHONY_LOGE("object is nullptr!");
        return;
    }
    int32_t callId = *object;
    if (callRequestProcessPtr_ == nullptr) {
        TELEPHONY_LOGE("callRequestProcessPtr_ is nullptr");
        return;
    }
    callRequestProcessPtr_->SeparateConferenceRequest(callId);
}

CallRequestHandlerService::CallRequestHandlerService() : eventLoop_(nullptr), handler_(nullptr) {}

CallRequestHandlerService::~CallRequestHandlerService()
{
    if (eventLoop_ != nullptr) {
        eventLoop_->Stop();
        eventLoop_ = nullptr;
    }
    if (handler_ != nullptr) {
        handler_ = nullptr;
    }
}

void CallRequestHandlerService::Start()
{
    eventLoop_ = AppExecFwk::EventRunner::Create("CallRequestHandler");
    if (eventLoop_.get() == nullptr) {
        TELEPHONY_LOGE("failed to create EventRunner");
        return;
    }
    handler_ = std::make_shared<CallRequestHandler>(eventLoop_);
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("failed to create CallRequestHandler");
        return;
    }
    handler_->Init();
    eventLoop_->Run();
    return;
}

int32_t CallRequestHandlerService::DialCall()
{
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("handler_ is nullptr");
        return TELEPHONY_FAIL;
    }

    std::lock_guard<std::mutex> lock(mutex_);
    if (!handler_->SendEvent(HANDLER_DIAL_CALL_REQUEST)) {
        TELEPHONY_LOGE("send dial event failed!");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallRequestHandlerService::AnswerCall(int32_t callId, int32_t videoState)
{
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("handler_ is nullptr");
        return TELEPHONY_FAIL;
    }
    std::unique_ptr<AnswerCallPara> para = std::make_unique<AnswerCallPara>();
    if (para.get() == nullptr) {
        TELEPHONY_LOGE("make_unique AnswerCallPara failed!");
        return TELEPHONY_FAIL;
    }
    para->callId = callId;
    para->videoState = videoState;
    std::lock_guard<std::mutex> lock(mutex_);
    if (!handler_->SendEvent(HANDLER_ANSWER_CALL_REQUEST, std::move(para))) {
        TELEPHONY_LOGE("send accept event failed!");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallRequestHandlerService::RejectCall(int32_t callId, bool isSendSms, std::string &content)
{
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("handler_ is nullptr");
        return TELEPHONY_FAIL;
    }
    std::unique_ptr<RejectCallPara> para = std::make_unique<RejectCallPara>();
    if (para.get() == nullptr) {
        TELEPHONY_LOGE("make_unique RejectCallPara failed!");
        return TELEPHONY_FAIL;
    }
    para->callId = callId;
    para->isSendSms = isSendSms;
    (void)memset_s(para->content, REJECT_CALL_MSG_MAX_LEN + 1, 0, REJECT_CALL_MSG_MAX_LEN + 1);
    if (para->isSendSms &&
        memcpy_s(para->content, REJECT_CALL_MSG_MAX_LEN, content.c_str(), REJECT_CALL_MSG_MAX_LEN) != 0) {
        TELEPHONY_LOGE("memcpy_s rejectCall content failed!");
        return TELEPHONY_MEMCPY_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!handler_->SendEvent(HANDLER_REJECT_CALL_REQUEST, std::move(para))) {
        TELEPHONY_LOGE("send reject event failed!");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallRequestHandlerService::HangUpCall(int32_t callId)
{
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("handler_ is nullptr");
        return TELEPHONY_FAIL;
    }
    std::unique_ptr<int32_t> para = std::make_unique<int32_t>(callId);
    if (para.get() == nullptr) {
        TELEPHONY_LOGE("make_unique callId failed!");
        return TELEPHONY_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!handler_->SendEvent(HANDLER_HANGUP_CALL_REQUEST, std::move(para))) {
        TELEPHONY_LOGE("send hung up event failed!");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallRequestHandlerService::HoldCall(int32_t callId)
{
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("handler_ is nullptr");
        return TELEPHONY_FAIL;
    }
    std::unique_ptr<int32_t> para = std::make_unique<int32_t>(callId);
    if (para.get() == nullptr) {
        TELEPHONY_LOGE("make_unique callId failed!");
        return TELEPHONY_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!handler_->SendEvent(HANDLER_HOLD_CALL_REQUEST, std::move(para))) {
        TELEPHONY_LOGE("send hold event failed!");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallRequestHandlerService::UnHoldCall(int32_t callId)
{
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("handler_ is nullptr");
        return TELEPHONY_FAIL;
    }
    std::unique_ptr<int32_t> para = std::make_unique<int32_t>(callId);
    if (para.get() == nullptr) {
        TELEPHONY_LOGE("make_unique callId failed!");
        return TELEPHONY_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!handler_->SendEvent(HANDLER_UNHOLD_CALL_REQUEST, std::move(para))) {
        TELEPHONY_LOGE("send unHold event failed!");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallRequestHandlerService::SwitchCall(int32_t callId)
{
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("handler_ is nullptr");
        return TELEPHONY_FAIL;
    }
    std::unique_ptr<int32_t> para = std::make_unique<int32_t>(callId);
    if (para.get() == nullptr) {
        TELEPHONY_LOGE("make_unique callId failed!");
        return TELEPHONY_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!handler_->SendEvent(HANDLER_SWAP_CALL_REQUEST, std::move(para))) {
        TELEPHONY_LOGE("send swap event failed!");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallRequestHandlerService::CombineConference(int32_t mainCallId)
{
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("handler_ is nullptr");
        return TELEPHONY_FAIL;
    }
    std::unique_ptr<int32_t> para = std::make_unique<int32_t>(mainCallId);
    if (para.get() == nullptr) {
        TELEPHONY_LOGE("make_unique mainCallId failed!");
        return TELEPHONY_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!handler_->SendEvent(HANDLER_COMBINE_CONFERENCE_REQUEST, std::move(para))) {
        TELEPHONY_LOGE("send CombineConference event failed!");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallRequestHandlerService::SeparateConference(int32_t callId)
{
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("handler_ is nullptr");
        return TELEPHONY_FAIL;
    }
    std::unique_ptr<int32_t> para = std::make_unique<int32_t>(callId);
    if (para.get() == nullptr) {
        TELEPHONY_LOGE("make_unique callId failed!");
        return TELEPHONY_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (!handler_->SendEvent(HANDLER_SEPARATE_CONFERENCE_REQUEST, std::move(para))) {
        TELEPHONY_LOGE("send SeparateConference event failed!");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}
} // namespace Telephony
} // namespace OHOS