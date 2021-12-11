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

#include "call_ability_callback_stub.h"

#include <securec.h>

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
CallAbilityCallbackStub::CallAbilityCallbackStub()
{
    memberFuncMap_[UPDATE_CALL_STATE_INFO] = &CallAbilityCallbackStub::OnUpdateCallStateInfoRequest;
    memberFuncMap_[UPDATE_CALL_EVENT] = &CallAbilityCallbackStub::OnUpdateCallEventRequest;
    memberFuncMap_[UPDATE_CALL_SUPPLEMENT_REQUEST] = &CallAbilityCallbackStub::OnUpdateSupplementResultRequest;
}

CallAbilityCallbackStub::~CallAbilityCallbackStub()
{
    memberFuncMap_.clear();
}

int CallAbilityCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string myDescriptor = CallAbilityCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (myDescriptor != remoteDescriptor) {
        TELEPHONY_LOGE("descriptor checked failed");
        return TELEPHONY_BAD_TYPE;
    }
    TELEPHONY_LOGD("OnReceived, cmd = %{public}u", code);

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t CallAbilityCallbackStub::OnUpdateCallStateInfoRequest(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGE("start");
    int32_t result = TELEPHONY_SUCCESS;
    const CallAttributeInfo *parcelPtr = nullptr;
    int32_t len = data.ReadInt32();
    if (len <= 0) {
        TELEPHONY_LOGE("Invalid parameter, len = %{public}d", len);
        return TELEPHONY_FAIL;
    }
    if (!data.ContainFileDescriptors()) {
        TELEPHONY_LOGW("sent raw data is less than 32k");
    }
    if ((parcelPtr = reinterpret_cast<const CallAttributeInfo *>(data.ReadRawData(len))) == nullptr) {
        TELEPHONY_LOGE("reading raw data failed, length = %{public}d", len);
        return TELEPHONY_FAIL;
    }

    result = OnCallDetailsChange(*parcelPtr);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("writing parcel failed");
        return TELEPHONY_FAIL;
    }
    TELEPHONY_LOGE("end");
    return TELEPHONY_SUCCESS;
}

int32_t CallAbilityCallbackStub::OnUpdateCallEventRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_SUCCESS;
    const CallEventInfo *parcelPtr = nullptr;
    int32_t len = data.ReadInt32();
    if (len <= 0) {
        TELEPHONY_LOGE("Invalid parameter, len = %{public}d", len);
        return TELEPHONY_FAIL;
    }
    if (!data.ContainFileDescriptors()) {
        TELEPHONY_LOGW("sent raw data is less than 32k");
    }
    if ((parcelPtr = reinterpret_cast<const CallEventInfo *>(data.ReadRawData(len))) == nullptr) {
        TELEPHONY_LOGE("reading raw data failed, length = %d", len);
        return TELEPHONY_FAIL;
    }

    result = OnCallEventChange(*parcelPtr);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("writing parcel failed");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallAbilityCallbackStub::OnUpdateSupplementResultRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_SUCCESS;
    AppExecFwk::PacMap resultInfo;
    CallResultReportId reportId = static_cast<CallResultReportId>(data.ReadInt32());
    resultInfo.PutIntValue("result", data.ReadInt32());
    switch (reportId) {
        case CallResultReportId::GET_CALL_WAITING_REPORT_ID:
        case CallResultReportId::GET_CALL_RESTRICTION_REPORT_ID:
            resultInfo.PutIntValue("status", data.ReadInt32());
            resultInfo.PutIntValue("classCw", data.ReadInt32());
            break;
        case CallResultReportId::GET_CALL_TRANSFER_REPORT_ID:
            resultInfo.PutIntValue("status", data.ReadInt32());
            resultInfo.PutIntValue("classx", data.ReadInt32());
            resultInfo.PutStringValue("number", data.ReadString());
            resultInfo.PutIntValue("type", data.ReadInt32());
            break;
        case CallResultReportId::GET_CALL_CLIP_ID:
            resultInfo.PutIntValue("action", data.ReadInt32());
            resultInfo.PutIntValue("clipStat", data.ReadInt32());
            break;
        case CallResultReportId::GET_CALL_CLIR_ID:
            resultInfo.PutIntValue("action", data.ReadInt32());
            resultInfo.PutIntValue("clirStat", data.ReadInt32());
            break;
        default:
            break;
    }
    if (!data.ContainFileDescriptors()) {
        TELEPHONY_LOGW("sent raw data is less than 32k");
    }
    result = OnSupplementResult(reportId, resultInfo);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("writing parcel failed");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}
} // namespace Telephony
} // namespace OHOS
