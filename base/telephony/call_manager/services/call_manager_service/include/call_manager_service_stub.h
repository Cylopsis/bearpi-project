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

#ifndef CALL_MANAGER_SERVICE_STUB_H
#define CALL_MANAGER_SERVICE_STUB_H

#include <map>

#include "iremote_object.h"
#include "iremote_stub.h"

#include "i_call_manager_service.h"

namespace OHOS {
namespace Telephony {
class CallManagerServiceStub : public IRemoteStub<ICallManagerService> {
public:
    CallManagerServiceStub();
    ~CallManagerServiceStub();
    virtual int OnRemoteRequest(
        uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option) override;

private:
    using CallManagerServiceFunc = int32_t (CallManagerServiceStub::*)(MessageParcel &data, MessageParcel &reply);

    int32_t RegisterCallBackRequest(MessageParcel &data, MessageParcel &reply);
    int32_t DialCallRequest(MessageParcel &data, MessageParcel &reply);
    int32_t AcceptCallRequest(MessageParcel &data, MessageParcel &reply);
    int32_t RejectCallRequest(MessageParcel &data, MessageParcel &reply);
    int32_t HangUpCallRequest(MessageParcel &data, MessageParcel &reply);
    int32_t GetCallStateRequest(MessageParcel &data, MessageParcel &reply);
    int32_t HoldCallRequest(MessageParcel &data, MessageParcel &reply);
    int32_t UnHoldCallRequest(MessageParcel &data, MessageParcel &reply);
    int32_t SwitchCallRequest(MessageParcel &data, MessageParcel &reply);
    int32_t HasCallRequest(MessageParcel &data, MessageParcel &reply);
    int32_t IsNewCallAllowedRequest(MessageParcel &data, MessageParcel &reply);
    int32_t IsRingingRequest(MessageParcel &data, MessageParcel &reply);
    int32_t IsInEmergencyCallRequest(MessageParcel &data, MessageParcel &reply);
    int32_t StartDtmfRequest(MessageParcel &data, MessageParcel &reply);
    int32_t SendDtmfRequest(MessageParcel &data, MessageParcel &reply);
    int32_t StopDtmfRequest(MessageParcel &data, MessageParcel &reply);
    int32_t SendDtmfBunchRequest(MessageParcel &data, MessageParcel &reply);
    int32_t GetCallWaitingRequest(MessageParcel &data, MessageParcel &reply);
    int32_t SetCallWaitingRequest(MessageParcel &data, MessageParcel &reply);
    int32_t JoinConferenceRequest(MessageParcel &data, MessageParcel &reply);
    int32_t IsEmergencyPhoneNumberRequest(MessageParcel &data, MessageParcel &reply);
    int32_t FormatPhoneNumberRequest(MessageParcel &data, MessageParcel &reply);
    int32_t FormatPhoneNumberToE164Request(MessageParcel &data, MessageParcel &reply);
    int32_t GetMainCallIdRequest(MessageParcel &data, MessageParcel &reply);
    int32_t GetSubCallIdListRequest(MessageParcel &data, MessageParcel &reply);
    int32_t GetCallIdListForConferenceRequest(MessageParcel &data, MessageParcel &reply);
    std::map<uint32_t, CallManagerServiceFunc> memberFuncMap_;
};
} // namespace Telephony
} // namespace OHOS

#endif // CALL_MANAGER_SERVICE_STUB_H