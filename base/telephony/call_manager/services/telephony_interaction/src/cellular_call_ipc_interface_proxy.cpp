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

#include "cellular_call_ipc_interface_proxy.h"

#include <thread>

#include "iservice_registry.h"
#include "system_ability.h"
#include "system_ability_definition.h"

#include "call_manager_errors.h"
#include "cellular_call_death_recipient.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
#ifdef RECONNECT_MAX_TRY_COUNT
constexpr uint32_t CONNECT_MAX_TRY_COUNT = 5;
#endif
CellularCallIpcInterfaceProxy::CellularCallIpcInterfaceProxy()
    : systemAbilityId_(TELEPHONY_CELLULAR_CALL_SYS_ABILITY_ID), cellularCallCallbackPtr_(nullptr),
      cellularCallInterfacePtr_(nullptr), connectState_(false)
{}

CellularCallIpcInterfaceProxy::~CellularCallIpcInterfaceProxy()
{
    unInit();
}

void CellularCallIpcInterfaceProxy::Init(int32_t systemAbilityId)
{
    systemAbilityId_ = systemAbilityId;

    int32_t result = ConnectService();
    if (result != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("connect service failed,errCode: %{public}X", result);
        Timer::start(CONNECT_SERVICE_WAIT_TIME, CellularCallIpcInterfaceProxy::task);
        return;
    }
    TELEPHONY_LOGI("connected to cellular call service successfully!");
}

void CellularCallIpcInterfaceProxy::unInit()
{
    DisconnectService();
}

void CellularCallIpcInterfaceProxy::task()
{
    int32_t ret = DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->ConnectService();
    if (ret != TELEPHONY_SUCCESS) {
        return;
    }
    DelayedSingleton<CellularCallIpcInterfaceProxy>::GetInstance()->ThreadExit();
}

int32_t CellularCallIpcInterfaceProxy::ConnectService()
{
    Utils::UniqueWriteGuard<Utils::RWLock> guard(rwClientLock_);
    if (cellularCallInterfacePtr_ != nullptr) {
        return TELEPHONY_SUCCESS;
    }
    sptr<ISystemAbilityManager> managerPtr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (managerPtr == nullptr) {
        return TELEPHONY_FAIL;
    }
    sptr<CellularCallInterface> cellularCallInterfacePtr = nullptr;
    sptr<IRemoteObject> iRemoteObjectPtr = managerPtr->GetSystemAbility(systemAbilityId_);
    if (iRemoteObjectPtr == nullptr) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    cellularCallInterfacePtr = iface_cast<CellularCallInterface>(iRemoteObjectPtr);
    if (!cellularCallInterfacePtr) {
        return TELEPHONY_LOCAL_PTR_NULL;
    }
    std::weak_ptr<CellularCallIpcInterfaceProxy> weakPtr = shared_from_this();
    auto deathCallback = [weakPtr](const wptr<IRemoteObject> &object) {
        auto sharedPtr = weakPtr.lock();
        if (sharedPtr) {
            sharedPtr->OnDeath();
        }
    };
    cellularCallRecipient_ = (std::make_unique<CellularCallDeathRecipient>(deathCallback)).release();
    if (cellularCallRecipient_ == nullptr) {
        return TELEPHONY_LOCAL_PTR_NULL;
    }
    if (!iRemoteObjectPtr->AddDeathRecipient(cellularCallRecipient_)) {
        return TELEPHONY_ADD_DEATH_RECIPIENT_FAIL;
    }
    cellularCallInterfacePtr_ = cellularCallInterfacePtr;
    int32_t ret = RegisterCallBackFun();
    if (ret != TELEPHONY_SUCCESS) {
        return ret;
    }
    connectState_ = true;
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallIpcInterfaceProxy::RegisterCallBackFun()
{
    if (cellularCallInterfacePtr_ == nullptr) {
        return TELEPHONY_LOCAL_PTR_NULL;
    }
    cellularCallCallbackPtr_ = (std::make_unique<CallStatusCallback>()).release();
    if (cellularCallCallbackPtr_ == nullptr) {
        Clean();
        return TELEPHONY_FAIL;
    }
    int32_t ret = cellularCallInterfacePtr_->RegisterCallManagerCallBack(cellularCallCallbackPtr_);
    if (ret != TELEPHONY_SUCCESS) {
        Clean();
        return TELEPHONY_REGISTER_CALLBACK_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

void CellularCallIpcInterfaceProxy::DisconnectService()
{
    Clean();
}

int32_t CellularCallIpcInterfaceProxy::ReConnectService()
{
    if (cellularCallInterfacePtr_ == nullptr) {
        TELEPHONY_LOGI("try to reconnect cellular call service now...");
        int32_t result = ConnectService();
        if (result != TELEPHONY_SUCCESS) {
            TELEPHONY_LOGE("Connect service: %{public}d", result);
            return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
        }
    }
    return TELEPHONY_SUCCESS;
}

void CellularCallIpcInterfaceProxy::OnDeath()
{
    Clean();
    NotifyDeath();
}

void CellularCallIpcInterfaceProxy::Clean()
{
    Utils::UniqueWriteGuard<Utils::RWLock> guard(rwClientLock_);
    if (cellularCallRecipient_ != nullptr) {
        cellularCallRecipient_.clear();
        cellularCallRecipient_ = nullptr;
    }
    if (cellularCallInterfacePtr_ != nullptr) {
        cellularCallInterfacePtr_.clear();
        cellularCallInterfacePtr_ = nullptr;
    }
    if (cellularCallCallbackPtr_ != nullptr) {
        cellularCallCallbackPtr_.clear();
        cellularCallCallbackPtr_ = nullptr;
    }
    connectState_ = false;
}

void CellularCallIpcInterfaceProxy::NotifyDeath()
{
    TELEPHONY_LOGI("service is dead, connect again");
#ifdef RECONNECT_MAX_TRY_COUNT
    for (uint32_t i = 0; i < CONNECT_MAX_TRY_COUNT; i++) {
        std::this_thread::sleep_for(std::chrono::milliseconds(CONNECT_SERVICE_WAIT_TIME));
        int32_t result = ConnectService();
        if (result != TELEPHONY_SUCCESS) {
            TELEPHONY_LOGI("connect cellular call service successful");
            return;
        }
    }
    TELEPHONY_LOGI("connect cellular call service failed");
#endif
    Timer::start(CONNECT_SERVICE_WAIT_TIME, CellularCallIpcInterfaceProxy::task);
}

int CellularCallIpcInterfaceProxy::Dial(const CellularCallInfo &callInfo)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->Dial(callInfo);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("dial failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::End(const CellularCallInfo &callInfo)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->End(callInfo);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("ending call failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::Reject(const CellularCallInfo &callInfo)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->Reject(callInfo);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("rejecting call failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::Answer(const CellularCallInfo &callInfo)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->Answer(callInfo);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("answering call failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::Hold(const CellularCallInfo &callInfo)
{
    ReConnectService();
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->Hold();
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("answering call failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::Active(const CellularCallInfo &callInfo)
{
    ReConnectService();
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->Active();
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("answering call failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::Swap()
{
    ReConnectService();
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->Swap();
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("answering call failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::IsUrgentCall(const std::string &phoneNum, int32_t slotId, int32_t &errorCode)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    return cellularCallInterfacePtr_->IsUrgentCall(phoneNum, slotId, errorCode);
}

int CellularCallIpcInterfaceProxy::Join()
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->Join();
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Join failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::Split(const std::string &splitString, int32_t index)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->Split(splitString, index);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Split failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::InitiateDTMF(char cDTMFCode, const std::string &phoneNum)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->InitiateDTMF(cDTMFCode, phoneNum);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("InitiateDTMF failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::CeaseDTMF(const std::string &phoneNum)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->CeaseDTMF(phoneNum);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("CeaseDTMF failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::TransmitDTMF(char cDTMFCode, const std::string &phoneNum)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->TransmitDTMF(cDTMFCode, phoneNum);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("TransmitDTMF failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::TransmitDTMFString(const std::string &dtmfCodeStr, const std::string &phoneNum,
    PhoneNetType phoneNetType, int32_t switchOn, int32_t switchOff)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->TransmitDTMFString(dtmfCodeStr, phoneNum, switchOn, switchOff);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("TransmitDTMFString failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::SetCallTransfer(const CallTransferInfo &info, int32_t slotId)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->SetCallTransfer(info, slotId);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("SetCallForward failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::InquireCallTransfer(CallTransferType type, int32_t slotId)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->InquireCallTransfer(type, slotId);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("InquireCallForward failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::SetCallWaiting(bool activate, int32_t slotId)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->SetCallWaiting(activate, slotId);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("SetCallWaiting failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::InquireCallWaiting(int32_t slotId)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->InquireCallWaiting(slotId);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("InquireCallWaiting failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::SetCallRestriction(const CallRestrictionInfo &info, int32_t slotId)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->SetCallRestriction(info, slotId);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("SetCallBarring failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::InquireCallRestriction(CallRestrictionType facType, int32_t slotId)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->InquireCallRestriction(facType, slotId);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("InquireCallBarring failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int CellularCallIpcInterfaceProxy::RegisterCallBack(const sptr<ICallStatusCallback> &callback)
{
    if (ReConnectService() != TELEPHONY_SUCCESS) {
        return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    int errCode = cellularCallInterfacePtr_->RegisterCallManagerCallBack(callback);
    if (errCode != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("registerCallBack failed, errcode:%{public}d", errCode);
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}
} // namespace Telephony
} // namespace OHOS