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

#include "telephony_state_registry_stub.h"

#include <utility>

#include "ipc_skeleton.h"
#include "string_ex.h"

#include "telephony_errors.h"

namespace OHOS {
namespace Telephony {
TelephonyStateRegistryStub::TelephonyStateRegistryStub()
{
    memberFuncMap_[SIM_STATE] = &TelephonyStateRegistryStub::UpdateSimStateInner;
    memberFuncMap_[SIGNAL_INFO] = &TelephonyStateRegistryStub::UpdateSignalInfoInner;
    memberFuncMap_[NET_WORK_STATE] = &TelephonyStateRegistryStub::UpdateNetworkStateInner;
    memberFuncMap_[CALL_STATE] = &TelephonyStateRegistryStub::UpdateCallStateInner;
    memberFuncMap_[CALL_STATE_FOR_ID] = &TelephonyStateRegistryStub::UpdateCallStateForSimIdInner;
    memberFuncMap_[ADD_OBSERVER] = &TelephonyStateRegistryStub::RegisterStateChangeInner;
    memberFuncMap_[REMOVE_OBSERVER] = &TelephonyStateRegistryStub::UnregisterStateChangeInner;
}

TelephonyStateRegistryStub::~TelephonyStateRegistryStub()
{
    memberFuncMap_.clear();
}

int32_t TelephonyStateRegistryStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    TELEPHONY_LOGD("TelephonyStateRegistryStub::OnRemoteRequest start##code = %{public}u\n", code);
    std::u16string myToken = TelephonyStateRegistryStub::GetDescriptor();
    std::u16string remoteToken = data.ReadInterfaceToken();
    if (myToken != remoteToken) {
        TELEPHONY_LOGE("TelephonyStateRegistryStub::OnRemoteRequest end##descriptor checked fail");
        return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    }
    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    int ret = IPCObjectStub::OnRemoteRequest(code, data, reply, option);
    TELEPHONY_LOGD("TelephonyStateRegistryStub::OnRemoteRequest end##ret=%{public}d\n", ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::UpdateCallStateInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t callState = data.ReadInt32();
    std::u16string phoneNumber = data.ReadString16();
    int32_t ret = UpdateCallState(callState, phoneNumber);
    TELEPHONY_LOGD("TelephonyStateRegistryStub::UpdateCallStateInner end##ret=%{public}d\n", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::UpdateSimStateInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t simId = data.ReadInt32();
    int32_t state = data.ReadInt32();
    std::u16string reason = data.ReadString16();
    int32_t ret = UpdateSimState(simId, state, reason);
    TELEPHONY_LOGD("TelephonyStateRegistryStub::UpdateSimStateInner end##ret=%{public}d\n", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::UpdateCallStateForSimIdInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t simId = data.ReadInt32();
    int32_t callId = data.ReadInt32();
    int32_t callState = data.ReadInt32();
    std::u16string incomingNumber = data.ReadString16();
    int32_t ret = UpdateCallStateForSimId(simId, callId, callState, incomingNumber);
    TELEPHONY_LOGD("TelephonyStateRegistryStub::UpdateCallStateForSimIdInner end##ret=%{public}d\n", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::UpdateSignalInfoInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = TELEPHONY_SUCCESS;
    int32_t simId = data.ReadInt32();
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("TelephonyStateRegistryStub::UpdateSignalInfoInner size=%{public}d\n", size);
    size = ((size > MAX_SIGNAL_NUM) ? 0 : size);
    if (size <= 0) {
        ret = TELEPHONY_FAIL;
        return ret;
    }
    std::vector<sptr<SignalInformation>> result;
    SignalInformation::NetworkType type;
    for (int i = 0; i < size; ++i) {
        type = static_cast<SignalInformation::NetworkType>(data.ReadInt32());
        switch (type) {
            case SignalInformation::NetworkType::GSM: {
                TELEPHONY_LOGD("TelephonyStateRegistryStub::UpdateSignalInfoInner NetworkType::GSM\n");
                std::unique_ptr<GsmSignalInformation> signal = std::make_unique<GsmSignalInformation>();
                if (signal != nullptr) {
                    signal->ReadFromParcel(data);
                    result.emplace_back(signal.release());
                }
                break;
            }
            case SignalInformation::NetworkType::CDMA: {
                TELEPHONY_LOGD("TelephonyStateRegistryStub::UpdateSignalInfoInner NetworkType::CDMA\n");
                std::unique_ptr<CdmaSignalInformation> signal = std::make_unique<CdmaSignalInformation>();
                if (signal != nullptr) {
                    signal->ReadFromParcel(data);
                    result.emplace_back(signal.release());
                }
                break;
            }
            default:
                break;
        }
    }
    ret = UpdateSignalInfo(simId, result);
    TELEPHONY_LOGD("TelephonyStateRegistryStub::UpdateSignalInfoInner end##ret=%{public}d\n", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::UpdateNetworkStateInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = TELEPHONY_SUCCESS;
    int32_t simId = data.ReadInt32();
    sptr<NetworkState> result = NetworkState::Unmarshalling(data);
    if (result == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryStub::UpdateNetworkStateInner GetNetworkStatus  is null\n");
        ret = TELEPHONY_FAIL;
        return ret;
    }
    ret = UpdateNetworkState(simId, result);
    TELEPHONY_LOGD("TelephonyStateRegistryStub::UpdateNetworkStateInner end##ret=%{public}d\n", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::RegisterStateChangeInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t ret = TELEPHONY_SUCCESS;
    int32_t simId = data.ReadInt32();
    int32_t mask = data.ReadInt32();
    bool notifyNow = data.ReadBool();
    std::u16string callingPackage = data.ReadString16();
    sptr<TelephonyObserverBroker> callback = nullptr;
    ret = ReadData(data, reply, callback);
    if (ret != TELEPHONY_SUCCESS) {
        reply.WriteInt32(ret);
        return ret;
    }
    ret = RegisterStateChange(callback, simId, mask, callingPackage, notifyNow);
    TELEPHONY_LOGD("TelephonyStateRegistryStub::RegisterStateChangeInner end##ret=%{public}d\n", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::UnregisterStateChangeInner(MessageParcel &data, MessageParcel &reply)
{
    int32_t simId = data.ReadInt32();
    int32_t mask = data.ReadInt32();
    int32_t ret = UnregisterStateChange(simId, mask);
    TELEPHONY_LOGD("TelephonyStateRegistryStub::UnregisterStateChangeInner end##ret=%{public}d\n", ret);
    reply.WriteInt32(ret);
    return ret;
}

int32_t TelephonyStateRegistryStub::ReadData(
    MessageParcel &data, MessageParcel &reply, sptr<TelephonyObserverBroker> &callback)
{
    int32_t result = TELEPHONY_SUCCESS;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryStub::ReadData  remote is nullptr.\n");
        result = TELEPHONY_FAIL;
        reply.WriteInt32(result);
        return result;
    }
    callback = iface_cast<TelephonyObserverBroker>(remote);
    if (callback == nullptr) {
        TELEPHONY_LOGE("TelephonyStateRegistryStub::ReadData callback is nullptr.\n");
        result = TELEPHONY_FAIL;
        reply.WriteInt32(result);
        return result;
    }
    return result;
}

int32_t TelephonyStateRegistryStub::RegisterStateChange(const sptr<TelephonyObserverBroker> &telephonyObserver,
    int32_t simId, uint32_t mask, const std::u16string &package, bool isUpdate)
{
    return RegisterStateChange(telephonyObserver, simId, mask, package, isUpdate, IPCSkeleton::GetCallingPid());
}

int32_t TelephonyStateRegistryStub::UnregisterStateChange(int32_t simId, uint32_t mask)
{
    return UnregisterStateChange(simId, mask, IPCSkeleton::GetCallingPid());
}
} // namespace Telephony
} // namespace OHOS