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

#include "cellular_call_service.h"
#include "system_ability_definition.h"
#include "module_service_utils.h"
#include "cs_control.h"

namespace OHOS {
namespace Telephony {
bool g_registerResult =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<CellularCallService>::GetInstance().get());

CellularCallService::CellularCallService() : SystemAbility(TELEPHONY_CELLULAR_CALL_SYS_ABILITY_ID, true)
{
    state_ = ServiceRunningState::STATE_STOPPED;
}

CellularCallService::~CellularCallService()
{
    state_ = ServiceRunningState::STATE_STOPPED;
}

bool CellularCallService::Init()
{
    TELEPHONY_LOGD("CellularCallService Init");
    eventLoop_ = AppExecFwk::EventRunner::Create("CellularCallService");
    if (eventLoop_ == nullptr) {
        TELEPHONY_LOGE("failed to create EventRunner");
        return false;
    }
    CreateHandler();
    AsynchronousRegister();
    TELEPHONY_LOGD("init success");
    return true;
}

void CellularCallService::OnStart()
{
    TELEPHONY_LOGD("CellularCallService OnStart");
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        TELEPHONY_LOGE("CellularCallService has already started.");
        return;
    }
    if (!Init()) {
        TELEPHONY_LOGE("failed to init service.");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    if (eventLoop_ != nullptr) {
        eventLoop_->Run();
    }
    bool ret = Publish(DelayedSingleton<CellularCallService>::GetInstance().get());
    if (!ret) {
        TELEPHONY_LOGE("CellularCallService::OnStart Publish failed!");
    }
    TELEPHONY_LOGD("CellularCallService start success.");
}

void CellularCallService::OnStop()
{
    TELEPHONY_LOGD("CellularCallService stop service");
    if (eventLoop_ != nullptr) {
        eventLoop_.reset();
    }

    state_ = ServiceRunningState::STATE_STOPPED;

    HandlerResetUnRegister();
}

void CellularCallService::RegisterHandler()
{
    TELEPHONY_LOGI("connect core service RegisterHandler start  ");
    for (uint32_t i = 0; i < CONNECT_MAX_TRY_COUNT; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(CONNECT_SERVICE_WAIT_TIME));
        int32_t slot = CoreManager::DEFAULT_SLOT_ID;
        auto core = CoreManager::GetInstance().getCore(slot);
        TELEPHONY_LOGI("connect core service count: %{public}d", i);
        if (core != nullptr && core->IsInitCore()) {
            RegisterCoreServiceHandler();
            break;
        }
        TELEPHONY_LOGI("connect core service RegisterHandler null or not init ");
    }
    TELEPHONY_LOGI("connect core service RegisterHandler end ");
}

void CellularCallService::OnDump()
{
    TELEPHONY_LOGD("CellularCallService OnDump");
}

std::shared_ptr<CellularCallHandler> CellularCallService::GetHandler(int32_t slot)
{
    return handlerMap_[slot];
}

void CellularCallService::CreateHandler()
{
    ModuleServiceUtils obtain;
    std::vector<int32_t> slotVector = obtain.GetSlotInfo();
    for (auto &it : slotVector) {
        auto handler = std::make_shared<CellularCallHandler>(eventLoop_);
        handler->SetSlotId(it);
        handler->SetCSControl(GetCsControl(it));
        handlerMap_.insert(std::make_pair(it, handler));
    }
}

void CellularCallService::HandlerResetUnRegister()
{
    std::map<int, std::shared_ptr<CellularCallHandler>>::iterator it;
    for (it = handlerMap_.begin(); it != handlerMap_.end(); ++it) {
        int32_t slot = it->first;
        auto handler = it->second;
        if (handler != nullptr) {
            handler.reset();
        }
        auto rilCore = CoreManager::GetInstance().getCore(slot);
        if (rilCore == nullptr) {
            TELEPHONY_LOGE("CellularCallService::HandlerResetUnRegister, rilCore is nullptr");
            return;
        }
        rilCore->UnRegisterPhoneNotify(handler, ObserverHandler::RADIO_CALL_STATE);
        rilCore->UnRegisterPhoneNotify(handler, ObserverHandler::RADIO_AVAIL);
        rilCore->UnRegisterPhoneNotify(handler, ObserverHandler::RADIO_NOT_AVAIL);

        if (GetCsControl(slot) == nullptr) {
            TELEPHONY_LOGE("CellularCallService::HandlerResetUnRegister, csControl is nullptr");
            return;
        }
        GetCsControl(slot)->ReleaseAllConnection();
    }
}

void CellularCallService::RegisterCoreServiceHandler()
{
    std::map<int, std::shared_ptr<CellularCallHandler>>::iterator it;
    for (it = handlerMap_.begin(); it != handlerMap_.end(); ++it) {
        int32_t slot = it->first;
        auto handler = it->second;
        auto rilCore = CoreManager::GetInstance().getCore(slot);
        if (rilCore == nullptr) {
            TELEPHONY_LOGE("RegisterHandler, rilCore is nullptr");
            return;
        }
        if (handler != nullptr) {
            TELEPHONY_LOGE("RegisterHandler, RegisterPhoneNotify handler");
            rilCore->RegisterPhoneNotify(handler, ObserverHandler::RADIO_CALL_STATE, nullptr);
            rilCore->RegisterPhoneNotify(handler, ObserverHandler::RADIO_AVAIL, nullptr);
            rilCore->RegisterPhoneNotify(handler, ObserverHandler::RADIO_NOT_AVAIL, nullptr);
        }
    }
}

void CellularCallService::AsynchronousRegister()
{
    TELEPHONY_LOGD("CellularCallService::AsynchronousRegister entry");
    int64_t delayTime_ = 1000;
    int32_t slot = CoreManager::DEFAULT_SLOT_ID;
    auto handler = handlerMap_[slot];
    handler->SendEvent(handler->ASYNCHRONOUS_REGISTER_ID, delayTime_, CellularCallHandler::Priority::HIGH);
}
} // namespace Telephony
} // namespace OHOS