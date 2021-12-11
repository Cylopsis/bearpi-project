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

#ifndef CALL_ABILITY_CALLBACK_STUB_H
#define CALL_ABILITY_CALLBACK_STUB_H

#include <map>

#include "iremote_object.h"
#include "iremote_stub.h"

#include "i_call_ability_callback.h"

namespace OHOS {
namespace Telephony {
class CallAbilityCallbackStub : public IRemoteStub<ICallAbilityCallback> {
public:
    CallAbilityCallbackStub();
    virtual ~CallAbilityCallbackStub();
    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    using CallAbilityCallbackFunc = int32_t (CallAbilityCallbackStub::*)(MessageParcel &data, MessageParcel &reply);

    int32_t OnUpdateCallStateInfoRequest(MessageParcel &data, MessageParcel &reply);
    int32_t OnUpdateCallEventRequest(MessageParcel &data, MessageParcel &reply);
    int32_t OnUpdateSupplementResultRequest(MessageParcel &data, MessageParcel &reply);

    std::map<uint32_t, CallAbilityCallbackFunc> memberFuncMap_;
};
} // namespace Telephony
} // namespace OHOS

#endif
