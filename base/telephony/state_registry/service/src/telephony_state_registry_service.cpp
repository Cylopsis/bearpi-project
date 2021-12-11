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

#include "telephony_state_registry_service.h"
#include "system_ability_definition.h"

#include "string_ex.h"

#include "state_registry_errors.h"

namespace OHOS {
namespace Telephony {
bool g_registerResult =
    SystemAbility::MakeAndRegisterAbility(DelayedSingleton<TelephonyStateRegistryService>::GetInstance().get());

TelephonyStateRegistryService::TelephonyStateRegistryService()
    : SystemAbility(TELEPHONY_STATE_REGISTRY_SYS_ABILITY_ID, true)
{
    TELEPHONY_LOGD("TelephonyStateRegistryService SystemAbility create");
    slotSize_ = 0;
    RegisterSubscriber();
}

TelephonyStateRegistryService::~TelephonyStateRegistryService()
{
    UnregisterSubscriber();
    stateRecords_.clear();
    callState_.clear();
    callIncomingNumber_.clear();
    signalInfos_.clear();
    searchNetworkState_.clear();
}

void TelephonyStateRegistryService::OnStart()
{
    std::lock_guard<std::mutex> guard(lock_);
    if (state_ == ServiceRunningState::STATE_RUNNING) {
        TELEPHONY_LOGE("Leave, FAILED, already running");
        return;
    }
    state_ = ServiceRunningState::STATE_RUNNING;
    bool ret = SystemAbility::Publish(DelayedSingleton<TelephonyStateRegistryService>::GetInstance().get());
    if (!ret) {
        TELEPHONY_LOGE("Leave, Failed to publish TelephonyStateRegistryService");
    }
    TELEPHONY_LOGD("TelephonyStateRegistryService start success.");
}

void TelephonyStateRegistryService::OnStop()
{
    TELEPHONY_LOGD("TelephonyStateRegistryService OnStop ");
    std::lock_guard<std::mutex> guard(lock_);
    state_ = ServiceRunningState::STATE_STOPPED;
}

void TelephonyStateRegistryService::Finalize()
{
    TELEPHONY_LOGD("TelephonyStateRegistryService Finalize");
}

void TelephonyStateRegistryService::OnDump()
{
    TELEPHONY_LOGD("TelephonyStateRegistryService OnDump");
}

int32_t TelephonyStateRegistryService::UpdateCellularDataConnectState(
    int32_t simId, int32_t dataState, int32_t networkType)
{
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST;
    if (VerifySimId(simId) &&
        (cellularDataConnectionState_[simId] != dataState ||
        cellularDataConnectionNetworkType_[simId] != networkType)) {
        for (size_t i = 0; i < stateRecords_.size(); i++) {
            TelephonyStateRegistryRecord record = stateRecords_[i];
            if (record.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_DATA_CONNECTION_STATE) &&
                (record.simId_ == simId) && record.telephonyObserver_ != nullptr) {
                record.telephonyObserver_->OnCellularDataConnectStateUpdated(dataState, networkType);
                result = TELEPHONY_SUCCESS;
            }
        }
        cellularDataConnectionState_[simId] = dataState;
        cellularDataConnectionNetworkType_[simId] = networkType;
    }
    SendCellularDataConnectStateChanged(simId, dataState, networkType);
    return result;
}

int32_t TelephonyStateRegistryService::UpdateCallState(int32_t callState, const std::u16string &number)
{
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST;
    int32_t simId = 1;
    for (size_t i = 0; i < stateRecords_.size(); i++) {
        TelephonyStateRegistryRecord record = stateRecords_[i];
        if (record.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE) &&
            record.telephonyObserver_ != nullptr) {
            std::u16string phoneNumberStr;
            if (record.IsCanReadCallHistory()) {
                phoneNumberStr = number;
            } else {
                phoneNumberStr = Str8ToStr16("");
            }
            record.telephonyObserver_->OnCallStateUpdated(callState, phoneNumberStr);
            result = TELEPHONY_SUCCESS;
        }
    }
    SendCallStateChanged(simId, callState, number);
    return result;
}

int32_t TelephonyStateRegistryService::UpdateCallStateForSimId(
    int32_t simId, int32_t callId, int32_t callState, const std::u16string &incomingNumber)
{
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST;
    std::u16string incomingNumberStr = incomingNumber;
    if (VerifySimId(simId)) {
        callState_[simId] = callState;
        callIncomingNumber_[simId] = incomingNumber;
        for (size_t i = 0; i < stateRecords_.size(); i++) {
            TelephonyStateRegistryRecord record = stateRecords_[i];
            if (record.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE) &&
                (record.simId_ == simId) && record.telephonyObserver_ != nullptr) {
                incomingNumberStr = GetCallIncomingNumberForSimId(record, simId);
                record.telephonyObserver_->OnCallStateUpdated(callState, incomingNumberStr);
                result = TELEPHONY_SUCCESS;
            }
        }
    }
    SendCallStateChanged(simId, callState, incomingNumberStr);
    return result;
}

int32_t TelephonyStateRegistryService::UpdateSimState(int32_t simId, int32_t state, const std::u16string &reason)
{
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST;
    if (VerifySimId(simId)) {
        simState_[simId] = state;
        simReason_[simId] = reason;
        for (size_t i = 0; i < stateRecords_.size(); i++) {
            TelephonyStateRegistryRecord record = stateRecords_[i];
            if (record.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_SIM_STATE) &&
                (record.simId_ == simId) && record.telephonyObserver_ != nullptr) {
                record.telephonyObserver_->OnSimStateUpdated(state, reason);
                result = TELEPHONY_SUCCESS;
            }
        }
    }
    SendSimStateChanged(simId, state, reason);
    return result;
}

int32_t TelephonyStateRegistryService::UpdateSignalInfo(
    int32_t simId, const std::vector<sptr<SignalInformation>> &vec)
{
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST;
    if (VerifySimId(simId)) {
        signalInfos_[simId] = vec;
        for (size_t i = 0; i < stateRecords_.size(); i++) {
            TelephonyStateRegistryRecord record = stateRecords_[i];
            if (record.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS) &&
                (record.simId_ == simId) && record.telephonyObserver_ != nullptr) {
                record.telephonyObserver_->OnSignalInfoUpdated(vec);
                result = TELEPHONY_SUCCESS;
            }
        }
    }
    SendSignalInfoChanged(simId, vec);
    return result;
}

int32_t TelephonyStateRegistryService::UpdateNetworkState(int32_t simId, const sptr<NetworkState> &networkState)
{
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_STATE_REGISTRY_DATA_NOT_EXIST;
    if (VerifySimId(simId)) {
        searchNetworkState_[simId] = networkState;
        for (size_t i = 0; i < stateRecords_.size(); i++) {
            TelephonyStateRegistryRecord record = stateRecords_[i];
            if (record.IsExistStateListener(TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE) &&
                (record.simId_ == simId) && record.telephonyObserver_ != nullptr) {
                record.telephonyObserver_->OnNetworkStateUpdated(networkState);
                result = TELEPHONY_SUCCESS;
            }
        }
    }
    SendNetworkStateChanged(simId, networkState);
    TELEPHONY_LOGD("TelephonyStateRegistryService::NotifyNetworkStateUpdated end");
    return result;
}

int32_t TelephonyStateRegistryService::RegisterStateChange(const sptr<TelephonyObserverBroker> &telephonyObserver,
    int32_t simId, uint32_t mask, const std::u16string &package, bool isUpdate, pid_t pid)
{
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_STATE_REGISTRY_DATA_EXIST;
    bool isExist = false;
    TelephonyStateRegistryRecord record;
    for (size_t i = 0; i < stateRecords_.size(); i++) {
        record = stateRecords_[i];
        if (record.simId_ == simId && record.mask_ == mask && record.pid_ == pid) {
            isExist = true;
            break;
        }
    }

    if (!isExist) {
        record.pid_ = pid;
        record.simId_ = simId;
        record.mask_ = mask;
        record.package_ = package;
        record.telephonyObserver_ = telephonyObserver;
        stateRecords_.push_back(record);
        result = TELEPHONY_SUCCESS;
    }

    if (isUpdate && VerifySimId(simId)) {
        UpdateData(record, mask, simId);
    }
    return result;
}

int32_t TelephonyStateRegistryService::UnregisterStateChange(int32_t simId, uint32_t mask, pid_t pid)
{
    std::lock_guard<std::mutex> guard(lock_);
    int32_t result = TELEPHONY_STATE_UNREGISTRY_DATA_NOT_EXIST;
    std::vector<TelephonyStateRegistryRecord>::iterator it;
    for (it = stateRecords_.begin(); it != stateRecords_.end(); ++it) {
        if (it->simId_ == simId && it->mask_ == mask && it->pid_ == pid) {
            stateRecords_.erase(it);
            result = TELEPHONY_SUCCESS;
            break;
        }
    }
    return result;
}

bool TelephonyStateRegistryService::VerifySimId(int simId)
{
    return simId >= 0;
}

std::u16string TelephonyStateRegistryService::GetCallIncomingNumberForSimId(
    TelephonyStateRegistryRecord record, int32_t simId)
{
    if (record.IsCanReadCallHistory()) {
        return callIncomingNumber_[simId];
    } else {
        return Str8ToStr16("");
    }
}

void TelephonyStateRegistryService::UpdateData(
    const TelephonyStateRegistryRecord &record, uint32_t mask, int32_t simId)
{
    TELEPHONY_LOGD("TelephonyStateRegistryService::RegisterStateChange##Notify start");
    if ((mask & TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE) != 0) {
        std::u16string phoneNumber = GetCallIncomingNumberForSimId(record, simId);
        TELEPHONY_LOGD("RegisterStateChange##Notify-OBSERVER_MASK_CALL_STATE");
        record.telephonyObserver_->OnCallStateUpdated(callState_[simId], phoneNumber);
    }
    if ((mask & TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS) != 0) {
        TELEPHONY_LOGD("RegisterStateChange##Notify-OBSERVER_MASK_SIGNAL_STRENGTHS");
        record.telephonyObserver_->OnSignalInfoUpdated(signalInfos_[simId]);
    }

    if ((mask & TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE) != 0) {
        TELEPHONY_LOGD("RegisterStateChange##Notify-OBSERVER_MASK_NETWORK_STATE");
        record.telephonyObserver_->OnNetworkStateUpdated(searchNetworkState_[simId]);
    }

    if ((mask & TelephonyObserverBroker::OBSERVER_MASK_SIM_STATE) != 0) {
        TELEPHONY_LOGD("RegisterStateChange##Notify-OBSERVER_MASK_SIM_STATE");
        record.telephonyObserver_->OnSimStateUpdated(simState_[simId], simReason_[simId]);
    }
}

bool TelephonyStateRegistryService::PublishSimFileEvent(
    const AAFwk::Want &want, int eventCode, const std::string &eventData)
{
    EventFwk::CommonEventData data;
    data.SetWant(want);
    data.SetCode(eventCode);
    data.SetData(eventData);
    EventFwk::CommonEventPublishInfo publishInfo;
    publishInfo.SetOrdered(true);
    bool publishResult = EventFwk::CommonEventManager::PublishCommonEvent(data, publishInfo, nullptr);
    TELEPHONY_LOGD("PublishSimFileEvent end###publishResult = %{public}d\n", publishResult);
    return publishResult;
}

void TelephonyStateRegistryService::UnregisterSubscriber()
{
    if (stateSubscriber_ != nullptr) {
        bool subscribeResult = EventFwk::CommonEventManager::UnSubscribeCommonEvent(stateSubscriber_);
        TELEPHONY_LOGD("UnregisterSubscriber end###subscribeResult = %{public}d\n", subscribeResult);
    }
    TELEPHONY_LOGD("UnregisterSubscriber end..\n");
}

void TelephonyStateRegistryService::RegisterSubscriber()
{
    EventFwk::MatchingSkills matchingSkills;
    matchingSkills.AddEvent(SPN_INFO_UPDATED_ACTION);
    EventFwk::CommonEventSubscribeInfo subscriberInfo(matchingSkills);
    stateSubscriber_ = std::make_shared<StateSubscriber>(subscriberInfo);
    bool subscribeResult = EventFwk::CommonEventManager::SubscribeCommonEvent(stateSubscriber_);
    TELEPHONY_LOGD("RegisterSubscriber end###subscribeResult = %{public}d\n", subscribeResult);
}

void StateSubscriber::OnReceiveEvent(const EventFwk::CommonEventData &data)
{
    AAFwk::Want want = data.GetWant();
    std::string action = data.GetWant().GetAction();
    TELEPHONY_LOGD("Subscriber::OnReceiveEvent action = %{public}s\n", action.c_str());

    int msgCode = GetCode();
    std::string msgData = GetData();
    TELEPHONY_LOGD(
        "Subscriber::OnReceiveEvent msgData = %{public}s,msgCode = %{public}d\n", msgData.c_str(), msgCode);
}

void TelephonyStateRegistryService::SendCallStateChanged(int32_t simId, int32_t state, const std::u16string &number)
{
    AAFwk::Want want;
    want.SetParam("simId", simId);
    want.SetParam("state", state);
    want.SetParam("number", Str16ToStr8(number));
    want.SetAction(CALL_STATE_CHANGE_ACTION);
    int32_t eventCode = 1;
    std::string eventData("callStateChanged");
    PublishSimFileEvent(want, eventCode, eventData);
}

void TelephonyStateRegistryService::SendCellularDataConnectStateChanged(
    int32_t simId, int32_t dataState, int32_t networkType)
{
    AAFwk::Want want;
    want.SetParam("simId", simId);
    want.SetParam("dataState", dataState);
    want.SetParam("networkType", networkType);
    want.SetAction(CELLULAR_DATA_STATE_CHANGE_ACTION);
    int32_t eventCode = 1;
    std::string eventData("connectStateChanged");
    PublishSimFileEvent(want, eventCode, eventData);
}

void TelephonyStateRegistryService::SendSimStateChanged(int32_t simId, int32_t state, const std::u16string &reason)
{
    AAFwk::Want want;
    want.SetParam("simId", simId);
    want.SetParam("reason", Str16ToStr8(reason));
    want.SetParam("state", state);
    want.SetAction(SIM_STATE_CHANGE_ACTION);
    int32_t eventCode = 1;
    std::string eventData("simStateChanged");
    PublishSimFileEvent(want, eventCode, eventData);
}

void TelephonyStateRegistryService::SendSignalInfoChanged(
    int32_t simId, const std::vector<sptr<SignalInformation>> &vec)
{
    AAFwk::Want want;
    want.SetParam("simId", simId);
    want.SetAction(SEARCH_SIGNAL_INFO_CHANGE_ACTION);
    std::vector<std::string> contentStr;
    for (size_t i = 0; i < vec.size(); i++) {
        sptr<SignalInformation> signal = vec[i];
        if (signal != nullptr) {
            contentStr.push_back(signal->ToString());
        }
    }
    want.SetParam("signalInfos", contentStr);
    int32_t eventCode = 1;
    std::string eventData("signalInfoChanged");
    PublishSimFileEvent(want, eventCode, eventData);
}

void TelephonyStateRegistryService::SendNetworkStateChanged(int32_t simId, const sptr<NetworkState> &networkState)
{
    AAFwk::Want want;
    want.SetParam("simId", simId);
    want.SetAction(SEARCH_NET_WORK_STATE_CHANGE_ACTION);
    int32_t eventCode = 1;
    if (networkState != nullptr) {
        want.SetParam("networkState", networkState->ToString());
    }
    std::string eventData("networkStateChanged");
    PublishSimFileEvent(want, eventCode, eventData);
}
} // namespace Telephony
} // namespace OHOS