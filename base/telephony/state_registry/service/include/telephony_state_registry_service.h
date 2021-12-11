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

#ifndef TELEPHONY_STATE_REGISTRY_SERVICE_H
#define TELEPHONY_STATE_REGISTRY_SERVICE_H

#include <map>
#include <mutex>
#include <string>

#include "singleton.h"
#include "system_ability.h"
#include "common_event_manager.h"
#include "want.h"

#include "telephony_state_registry_record.h"
#include "telephony_state_registry_stub.h"

namespace OHOS {
namespace Telephony {
enum class ServiceRunningState { STATE_STOPPED, STATE_RUNNING };
class StateSubscriber : public EventFwk::CommonEventSubscriber {
public:
    StateSubscriber(const EventFwk::CommonEventSubscribeInfo &subscriberInfo)
        : CommonEventSubscriber(subscriberInfo)
    {}
    ~StateSubscriber() = default;
    void OnReceiveEvent(const EventFwk::CommonEventData &data);
};
class TelephonyStateRegistryService : public SystemAbility,
                                      public TelephonyStateRegistryStub,
                                      public std::enable_shared_from_this<TelephonyStateRegistryService> {
    DECLARE_DELAYED_SINGLETON(TelephonyStateRegistryService)
    DECLARE_SYSTEM_ABILITY(TelephonyStateRegistryService)
public:
    void OnStart() override;
    void OnStop() override;
    void OnDump() override;

    int32_t UpdateCellularDataConnectState(int32_t simId, int32_t dataState, int32_t networkType) override;
    int32_t UpdateCallState(int32_t callState, const std::u16string &phoneNumber) override;
    int32_t UpdateCallStateForSimId(
        int32_t simId, int32_t callId, int32_t callState, const std::u16string &incomingNumber) override;
    int32_t UpdateSignalInfo(int32_t simId, const std::vector<sptr<SignalInformation>> &vec) override;
    int32_t UpdateNetworkState(int32_t simId, const sptr<NetworkState> &networkState) override;
    int32_t UpdateSimState(int32_t simId, int32_t state, const std::u16string &reason) override;
    int32_t RegisterStateChange(const sptr<TelephonyObserverBroker> &telephonyObserver, int32_t simId,
        uint32_t mask, const std::u16string &callingPackage, bool notifyNow, pid_t pid) override;
    int32_t UnregisterStateChange(int32_t simId, uint32_t mask, pid_t pid) override;

private:
    void Finalize();
    void UpdateData(const TelephonyStateRegistryRecord &record, uint32_t mask, int32_t simId);

private:
    bool VerifySimId(int32_t simId);
    std::u16string GetCallIncomingNumberForSimId(TelephonyStateRegistryRecord record, int32_t simId);
    bool PublishSimFileEvent(const AAFwk::Want &want, int32_t eventCode, const std::string &eventData);
    void RegisterSubscriber();
    void UnregisterSubscriber();
    void SendCallStateChanged(int32_t simId, int32_t state, const std::u16string &number);
    void SendCellularDataConnectStateChanged(
        int32_t simId, int32_t dataState, int32_t networkType, const std::u16string &apnType);
    void SendSignalInfoChanged(int32_t simId, const std::vector<sptr<SignalInformation>> &vec);
    void SendNetworkStateChanged(int32_t simId, const sptr<NetworkState> &networkState);
    void SendSimStateChanged(int32_t simId, int32_t state, const std::u16string &reason);
    void SendCellularDataConnectStateChanged(int32_t simId, int32_t dataState, int32_t networkType);

private:
    ServiceRunningState state_ = ServiceRunningState::STATE_STOPPED;
    std::mutex lock_;
    int32_t slotSize_;
    std::map<int32_t, int32_t> callState_;
    std::map<int32_t, std::u16string> callIncomingNumber_;
    std::map<int32_t, std::vector<sptr<SignalInformation>>> signalInfos_;
    std::map<int32_t, sptr<NetworkState>> searchNetworkState_;
    std::vector<TelephonyStateRegistryRecord> stateRecords_;
    std::shared_ptr<StateSubscriber> stateSubscriber_;
    std::map<int32_t, int32_t> simState_;
    std::map<int32_t, std::u16string> simReason_;
    std::map<int32_t, int32_t> cellularDataConnectionState_;
    std::map<int32_t, int32_t> cellularDataConnectionNetworkType_;

private:
    const std::string CALL_STATE_CHANGE_ACTION = "com.hos.action.CALL_STATE_CHANGE";
    const std::string SEARCH_NET_WORK_STATE_CHANGE_ACTION = "com.hos.action.SEARCH_NET_WORK_STATE_CHANGE";
    const std::string SEARCH_SIGNAL_INFO_CHANGE_ACTION = "com.hos.action.SEARCH_SIGNAL_INFO_CHANGE";
    const std::string CELLULAR_DATA_STATE_CHANGE_ACTION = "com.hos.action.CELLULAR_DATA_STATE_CHANGE";
    const std::string SIM_STATE_CHANGE_ACTION = "com.hos.action.SIM_STATE_CHANGE";
};
} // namespace Telephony
} // namespace OHOS
#endif // TELEPHONY_STATE_REGISTRY_SERVICE_H
