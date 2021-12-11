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

#include "napi_call_ability_handler.h"

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
NapiCallAbilityHandler::NapiCallAbilityHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{}

NapiCallAbilityHandler::~NapiCallAbilityHandler()
{
    if (napiCallAbilityCallbackPtr_) {
        napiCallAbilityCallbackPtr_ = nullptr;
    }
}

void NapiCallAbilityHandler::Init()
{
    napiCallAbilityCallbackPtr_ = DelayedSingleton<NapiCallAbilityCallback>::GetInstance();
    if (napiCallAbilityCallbackPtr_ == nullptr) {
        TELEPHONY_LOGE("napiCallAbilityCallbackPtr_ is null");
        return;
    }
}

void NapiCallAbilityHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        TELEPHONY_LOGE("NapiCallAbilityHandler::ProcessEvent parameter error");
        return;
    }

    TELEPHONY_LOGD(
        "NapiCallAbilityHandler::ProcessEvent inner event id obtained: %{public}u.", event->GetInnerEventId());
    switch (event->GetInnerEventId()) {
        case NapiCallAbilityHandlerService::NAPI_HANDLER_UPDATE_CALL_INFO: {
            auto object = event->GetUniqueObject<CallAttributeInfo>();
            if (object == nullptr) {
                TELEPHONY_LOGE("object is nullptr!");
                break;
            }
            CallAttributeInfo info = *object;
            if (napiCallAbilityCallbackPtr_ == nullptr) {
                TELEPHONY_LOGE("napiCallAbilityCallbackPtr is nullptr");
                return;
            }
            napiCallAbilityCallbackPtr_->UpdateCallStateInfo(info);
            break;
        }
        case NapiCallAbilityHandlerService::NAPI_HANDLER_UPDATE_CALL_EVENT: {
            auto object = event->GetUniqueObject<CallEventInfo>();
            if (object == nullptr) {
                TELEPHONY_LOGE("object is nullptr!");
                break;
            }
            CallEventInfo info = *object;
            if (napiCallAbilityCallbackPtr_ == nullptr) {
                TELEPHONY_LOGE("napiCallAbilityCallbackPtr is nullptr");
                return;
            }
            napiCallAbilityCallbackPtr_->UpdateCallEvent(info);
            break;
        }
        default: {
            break;
        }
    }
    TELEPHONY_LOGD("NapiCallAbilityHandler::ProcessEvent Leave");
}

NapiCallAbilityHandlerService::NapiCallAbilityHandlerService() : eventLoop_(nullptr), handler_(nullptr) {}

NapiCallAbilityHandlerService::~NapiCallAbilityHandlerService()
{
    if (eventLoop_ != nullptr) {
        eventLoop_->Stop();
        eventLoop_ = nullptr;
    }
    if (handler_ != nullptr) {
        handler_ = nullptr;
    }
}

void NapiCallAbilityHandlerService::Start()
{
    eventLoop_ = AppExecFwk::EventRunner::Create("NapiCallAbilityHandlerService");
    if (eventLoop_.get() == nullptr) {
        TELEPHONY_LOGE("failed to create EventRunner");
        return;
    }
    handler_ = std::make_shared<NapiCallAbilityHandler>(eventLoop_);
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("failed to create NapiCallAbilityHandler");
        return;
    }
    handler_->Init();
    eventLoop_->Run();
    return;
}

int32_t NapiCallAbilityHandlerService::UpdateCallStateInfoHandler(const CallAttributeInfo &info)
{
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("handler_ is nullptr");
        return TELEPHONY_FAIL;
    }
    std::unique_ptr<CallAttributeInfo> para = std::make_unique<CallAttributeInfo>();
    if (para.get() == nullptr) {
        TELEPHONY_LOGE("make_unique CallAttributeInfo failed!");
        return TELEPHONY_FAIL;
    }
    *para = info;
    std::lock_guard<std::mutex> lock(mutex_);
    bool ret = handler_->SendEvent(NAPI_HANDLER_UPDATE_CALL_INFO, std::move(para));
    if (!ret) {
        TELEPHONY_LOGE("SendEvent failed! status update failed, state:%{public}d", info.callState);
    }
    return TELEPHONY_SUCCESS;
}

int32_t NapiCallAbilityHandlerService::UpdateCallEventHandler(const CallEventInfo &info)
{
    if (handler_.get() == nullptr) {
        TELEPHONY_LOGE("handler_ is nullptr");
        return TELEPHONY_FAIL;
    }
    std::unique_ptr<CallEventInfo> para = std::make_unique<CallEventInfo>();
    if (para.get() == nullptr) {
        TELEPHONY_LOGE("make_unique CallEventInfo failed!");
        return TELEPHONY_FAIL;
    }
    *para = info;
    std::lock_guard<std::mutex> lock(mutex_);
    bool ret = handler_->SendEvent(NAPI_HANDLER_UPDATE_CALL_EVENT, std::move(para));
    if (!ret) {
        TELEPHONY_LOGE("SendEvent failed! call event update failed, eventId:%{public}d", info.eventId);
    }
    return TELEPHONY_SUCCESS;
}

int32_t NapiCallAbilityHandlerService::UpdateSupplementInfoHandler(
    CallResultReportId reportId, AppExecFwk::PacMap &resultInfo)
{
    return TELEPHONY_SUCCESS;
}
} // namespace Telephony
} // namespace OHOS