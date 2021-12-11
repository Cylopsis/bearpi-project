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

#ifndef TELEPHONY_STATE_REGISTRY_STUB_H
#define TELEPHONY_STATE_REGISTRY_STUB_H

#include <map>

#include "iremote_stub.h"

#include "telephony_log_wrapper.h"
#include "i_telephony_state_notify.h"

namespace OHOS {
namespace Telephony {
class TelephonyStateRegistryStub : public IRemoteStub<ITelephonyStateNotify> {
public:
    TelephonyStateRegistryStub();
    ~TelephonyStateRegistryStub();

    int32_t OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

    virtual int32_t RegisterStateChange(const sptr<TelephonyObserverBroker> &telephonyObserver, int32_t simId,
        uint32_t mask, const std::u16string &package, bool notifyNow, pid_t pid) = 0;

    virtual int32_t UnregisterStateChange(int32_t simId, uint32_t mask, pid_t pid) = 0;

private:
    int32_t ReadData(MessageParcel &data, MessageParcel &reply, sptr<TelephonyObserverBroker> &callback);
    int32_t RegisterStateChange(const sptr<TelephonyObserverBroker> &telephonyObserver, int32_t simId,
        uint32_t mask, const std::u16string &package, bool isUpdate) override;
    int32_t UnregisterStateChange(int32_t simId, uint32_t mask) override;

private:
    using TelephonyStateFunc = int32_t (TelephonyStateRegistryStub::*)(MessageParcel &data, MessageParcel &reply);

    int32_t UpdateCallStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t UpdateCallStateForSimIdInner(MessageParcel &data, MessageParcel &reply);
    int32_t UpdateSignalInfoInner(MessageParcel &data, MessageParcel &reply);
    int32_t UpdateNetworkStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t UpdateSimStateInner(MessageParcel &data, MessageParcel &reply);
    int32_t RegisterStateChangeInner(MessageParcel &data, MessageParcel &reply);
    int32_t UnregisterStateChangeInner(MessageParcel &data, MessageParcel &reply);
    int32_t UpdateCellularDataConnectStateInner(MessageParcel &data, MessageParcel &reply);

private:
    std::map<uint32_t, TelephonyStateFunc> memberFuncMap_;
};
} // namespace Telephony
} // namespace OHOS
#endif // TELEPHONY_STATE_REGISTRY_STUB_H
