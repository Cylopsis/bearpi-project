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

#include "call_state_report_proxy.h"

#include <string_ex.h>

#include "iservice_registry.h"
#include "system_ability.h"
#include "system_ability_definition.h"

#include "call_manager_errors.h"
#include "call_manager_inner_type.h"
#include "state_registry_death_recipient.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
CallStateReportProxy::CallStateReportProxy()
    : systemAbilityId_(TELEPHONY_STATE_REGISTRY_SYS_ABILITY_ID), telephonyStateNotifyPtr_(nullptr),
      stateRegistryRecipient_(nullptr), connectState_(false)
{}

CallStateReportProxy::~CallStateReportProxy()
{
    UnInit();
}

int32_t CallStateReportProxy::Init(int32_t systemAbilityId)
{
    systemAbilityId_ = systemAbilityId;
    TELEPHONY_LOGI("init reportCallStateClient");

    int32_t result = ConnectService();
    if (result != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("connect state registry service failed! errCode:%{public}d", result);
        Timer::start(CONNECT_SERVICE_WAIT_TIME, CallStateReportProxy::task);
        return result;
    }
    TELEPHONY_LOGI("connected to state registry service successfully");
    return result;
}

void CallStateReportProxy::UnInit()
{
    DisconnectService();
}

void CallStateReportProxy::task()
{
    int32_t ret = DelayedSingleton<CallStateReportProxy>::GetInstance()->ConnectService();
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Connect service failed! ret:%{public}d", ret);
        return;
    }
    DelayedSingleton<CallStateReportProxy>::GetInstance()->ThreadExit();
}

int32_t CallStateReportProxy::ReportCallState(int32_t callState, std::u16string phoneNumber)
{
    int ret = TELEPHONY_FAIL;
    if (telephonyStateNotifyPtr_ == nullptr) {
        TELEPHONY_LOGI(
            "telephony state notify service haven't connected,try to connect telephony state notify service "
            "now...");
        int32_t result = ConnectService();
        if (result != TELEPHONY_SUCCESS) {
            TELEPHONY_LOGE("Connect service: %{public}X", result);
            return result;
        }
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (telephonyStateNotifyPtr_ != nullptr) {
        ret = telephonyStateNotifyPtr_->UpdateCallState(callState, phoneNumber);
        if (ret != TELEPHONY_SUCCESS) {
            TELEPHONY_LOGE("notifyCallStateUpdated failed, errcode:%{public}d", ret);
            return ret;
        }
        TELEPHONY_LOGD("report call state:%d", callState);
    }
    return ret;
}

int32_t CallStateReportProxy::ReportCallStateForCallId(
    int32_t simId, int32_t callId, int32_t callState, std::u16string incomingNumber)
{
    int ret = TELEPHONY_FAIL;
    if (telephonyStateNotifyPtr_ == nullptr) {
        TELEPHONY_LOGI(
            "telephony state notify service haven't connected,try to connect telephony state notify service "
            "now...");
        int32_t result = ConnectService();
        if (result != TELEPHONY_SUCCESS) {
            TELEPHONY_LOGE("connect service: %{public}X", result);
            return result;
        }
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (telephonyStateNotifyPtr_ != nullptr) {
        ret = telephonyStateNotifyPtr_->UpdateCallStateForSimId(simId, callId, callState, incomingNumber);
        if (ret != TELEPHONY_SUCCESS) {
            TELEPHONY_LOGE("NotifyCallStateUpdated failed, errcode:%{public}d", ret);
            return ret;
        }
        TELEPHONY_LOGD("report call state:%d, callId:%d", callState, callId);
    }
    return ret;
}

int32_t CallStateReportProxy::ConnectService()
{
    int32_t ret = TELEPHONY_FAIL;
    Utils::UniqueWriteGuard<Utils::RWLock> guard(rwClientLock_);
    if (telephonyStateNotifyPtr_ != nullptr) {
        return ret;
    }

    sptr<ISystemAbilityManager> managerPtr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (managerPtr == nullptr) {
        return ret;
    }
    sptr<ITelephonyStateNotify> telephonyStateNotifyPtr = nullptr;
    sptr<IRemoteObject> iRemoteObjectPtr = managerPtr->GetSystemAbility(systemAbilityId_);
    if (iRemoteObjectPtr == nullptr) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    telephonyStateNotifyPtr = iface_cast<ITelephonyStateNotify>(iRemoteObjectPtr);
    if (telephonyStateNotifyPtr == nullptr) {
        return ret;
    }
#ifdef SUBSCRIBE_DEATH_NOTICE
    std::weak_ptr<CallStateReportProxy> weakPtr = shared_from_this();
    auto deathCallback = [weakPtr](const wptr<IRemoteObject> &object) {
        auto sharedPtr = weakPtr.lock();
        if (sharedPtr) {
            sharedPtr->OnDeath();
        }
    };
    stateRegistryRecipient_ = (std::make_unique<StateRegistryDeathRecipient>(deathCallback)).release();
    if (stateRegistryRecipient_ == nullptr) {
        TELEPHONY_LOGI("create StateRegistryDeathRecipient object failed!");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
    if (!iRemoteObjectPtr->AddDeathRecipient(stateRegistryRecipient_)) {
        TELEPHONY_LOGI("failed to subscribing to the state registry service death notification");
        return TELEPHONY_ADD_DEATH_RECIPIENT_FAIL;
    }
    TELEPHONY_LOGI("Succeeded in subscribing to the state registry service death notification");
#endif
    telephonyStateNotifyPtr_ = telephonyStateNotifyPtr;
    TELEPHONY_LOGD("Connect state registry service success!");
    return TELEPHONY_SUCCESS;
}

void CallStateReportProxy::DisconnectService()
{
    Clean();
}

void CallStateReportProxy::ReConnectService()
{
    if (telephonyStateNotifyPtr_ == nullptr) {
        TELEPHONY_LOGI(
            "telephony state notify service haven't connected,try to connect telephony state "
            "notify "
            "service now...");
        int32_t result = ConnectService();
        if (result != TELEPHONY_SUCCESS) {
            TELEPHONY_LOGE("Connect service: %{public}X", result);
            return;
        }
    }
}

void CallStateReportProxy::OnDeath()
{
    Clean();
    NotifyDeath();
}

void CallStateReportProxy::Clean()
{
    Utils::UniqueWriteGuard<Utils::RWLock> guard(rwClientLock_);
    if (stateRegistryRecipient_ != nullptr) {
        stateRegistryRecipient_.clear();
        stateRegistryRecipient_ = nullptr;
    }
    if (telephonyStateNotifyPtr_ != nullptr) {
        telephonyStateNotifyPtr_.clear();
        telephonyStateNotifyPtr_ = nullptr;
    }
    connectState_ = false;
}

void CallStateReportProxy::NotifyDeath()
{
    TELEPHONY_LOGI("service is dead, connect again");
    Timer::start(CONNECT_SERVICE_WAIT_TIME, CallStateReportProxy::task);
}
} // namespace Telephony
} // namespace OHOS
