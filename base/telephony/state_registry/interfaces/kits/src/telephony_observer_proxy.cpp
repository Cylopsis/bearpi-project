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

#include "telephony_observer_proxy.h"

#include "parcel.h"
#include "string_ex.h"

namespace OHOS {
namespace Telephony {
TelephonyObserverProxy::TelephonyObserverProxy(const sptr<IRemoteObject> &impl)
    : IRemoteProxy<TelephonyObserverBroker>(impl)
{}

void TelephonyObserverProxy::OnCallStateUpdated(int32_t callState, const std::u16string &phoneNumber)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);
    dataParcel.WriteInt32(callState);
    dataParcel.WriteString16(phoneNumber);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnCallStateUpdated remote is null.\n");
        return;
    }
    int code = remote->SendRequest(ON_CALL_STATE_UPDATED, dataParcel, replyParcel, option);
    TELEPHONY_LOGD("TelephonyObserverProxy::OnCallStateUpdated end ##error: %{public}d.\n", code);
};

void TelephonyObserverProxy::OnSimStateUpdated(int32_t state, const std::u16string &reason)
{
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    MessageOption option;
    option.SetFlags(MessageOption::TF_ASYNC);
    dataParcel.WriteInt32(state);
    dataParcel.WriteString16(reason);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnSimStateUpdated remote is null.\n");
        return;
    }
    int code = remote->SendRequest(ON_SIM_STATE_UPDATED, dataParcel, replyParcel, option);
    TELEPHONY_LOGD("TelephonyObserverProxy::OnSimStateUpdated end ##error: %{public}d.\n", code);
}

void TelephonyObserverProxy::OnSignalInfoUpdated(const std::vector<sptr<SignalInformation>> &vec)
{
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    option.SetFlags(MessageOption::TF_ASYNC);
    int32_t size = vec.size();
    if (size <= 0 || size > MAX_SIGNAL_NUM) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnSignalInfoUpdated size is error.\n");
        return;
    }
    dataParcel.WriteInt32(size);
    for (const auto &v : vec) {
        v->Marshalling(dataParcel);
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnSignalInfoUpdated remote is null.\n");
        return;
    }
    int code = remote->SendRequest(ON_SIGNAL_INFO_UPDATED, dataParcel, replyParcel, option);
    TELEPHONY_LOGD("TelephonyObserverProxy::OnSignalInfoUpdated##error: %{public}d.\n", code);
}

void TelephonyObserverProxy::OnNetworkStateUpdated(const sptr<NetworkState> &networkState)
{
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    option.SetFlags(MessageOption::TF_ASYNC);
    if (networkState != nullptr) {
        networkState->Marshalling(dataParcel);
    }
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnNetworkStateUpdated remote is null.\n");
        return;
    }
    int code = remote->SendRequest(ON_NETWORK_STATE_UPDATED, dataParcel, replyParcel, option);
    TELEPHONY_LOGD("TelephonyObserverProxy::OnNetworkStateUpdated##error: %{public}d.\n", code);
}

void TelephonyObserverProxy::OnCellularDataConnectStateUpdated(int32_t dataState, int32_t networkType)
{
    MessageOption option;
    MessageParcel dataParcel;
    MessageParcel replyParcel;
    option.SetFlags(MessageOption::TF_ASYNC);
    dataParcel.WriteInt32(dataState);
    dataParcel.WriteInt32(networkType);
    sptr<IRemoteObject> remote = Remote();
    if (remote == nullptr) {
        TELEPHONY_LOGE("TelephonyObserverProxy::OnCellularDataConnectStateUpdated remote is null.\n");
        return;
    }
    int code = remote->SendRequest(ON_CELLULAR_DATA_CONNECT_STATE_UPDATED, dataParcel, replyParcel, option);
    TELEPHONY_LOGD("TelephonyObserverProxy::OnCellularDataConnectStateUpdated##error: %{public}d.\n", code);
}
} // namespace Telephony
} // namespace OHOS