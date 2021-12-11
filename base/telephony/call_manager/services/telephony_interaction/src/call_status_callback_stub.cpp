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

#include "call_status_callback_stub.h"

#include <securec.h>

#include "call_manager_errors.h"
#include "cellular_call_types.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
CallStatusCallbackStub::CallStatusCallbackStub()
{
    memberFuncMap_[UPDATE_CALL_INFO] = &CallStatusCallbackStub::OnUpdateCallReportInfoRequest;
    memberFuncMap_[UPDATE_CALLS_INFO] = &CallStatusCallbackStub::OnUpdateCallsReportInfoRequest;
    memberFuncMap_[UPDATE_DISCONNECTED_CAUSE] = &CallStatusCallbackStub::OnUpdateDisconnectedCauseRequest;
    memberFuncMap_[UPDATE_EVENT_RESULT_INFO] = &CallStatusCallbackStub::OnUpdateEventReportRequest;
    memberFuncMap_[UPDATE_GET_WAITING] = &CallStatusCallbackStub::OnUpdateGetWaitingResultRequest;
    memberFuncMap_[UPDATE_SET_WAITING] = &CallStatusCallbackStub::OnUpdateSetWaitingResultRequest;
    memberFuncMap_[UPDATE_GET_RESTRICTION] = &CallStatusCallbackStub::OnUpdateGetRestrictionResultRequest;
    memberFuncMap_[UPDATE_SET_RESTRICTION] = &CallStatusCallbackStub::OnUpdateSetRestrictionResultRequest;
    memberFuncMap_[UPDATE_GET_TRANSFER] = &CallStatusCallbackStub::OnUpdateGetTransferResultRequest;
    memberFuncMap_[UPDATE_SET_TRANSFER] = &CallStatusCallbackStub::OnUpdateSetTransferResultRequest;
    memberFuncMap_[UPDATE_GET_CALL_CLIP] = &CallStatusCallbackStub::OnUpdateGetCallClipResultRequest;
    memberFuncMap_[UPDATE_GET_CALL_CLIR] = &CallStatusCallbackStub::OnUpdateGetCallClirResultRequest;
    memberFuncMap_[UPDATE_SET_CALL_CLIR] = &CallStatusCallbackStub::OnUpdateSetCallClirResultRequest;
}

CallStatusCallbackStub::~CallStatusCallbackStub()
{
    memberFuncMap_.clear();
}

int CallStatusCallbackStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string myDescriptor = CallStatusCallbackStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (myDescriptor != remoteDescriptor) {
        TELEPHONY_LOGE("descriptor checked failed");
        return TELEPHONY_BAD_TYPE;
    }
    TELEPHONY_LOGD("OnReceived, cmd = %u", code);

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data, reply);
        }
    }
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

int32_t CallStatusCallbackStub::OnUpdateCallReportInfoRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_SUCCESS;
    const CallReportInfo *parcelPtr = nullptr;
    int32_t len = data.ReadInt32();
    if (len <= TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Invalid parameter, len = %{public}d", len);
        return TELEPHONY_FAIL;
    }
    if (!data.ContainFileDescriptors()) {
        TELEPHONY_LOGW("sent raw data is less than 32k");
    }
    if ((parcelPtr = reinterpret_cast<const CallReportInfo *>(data.ReadRawData(len))) == nullptr) {
        TELEPHONY_LOGE("reading raw data failed, length = %d", len);
        return TELEPHONY_FAIL;
    }

    result = OnUpdateCallReportInfo(*parcelPtr);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("writing parcel failed");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusCallbackStub::OnUpdateCallsReportInfoRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_SUCCESS;
    int32_t cnt = data.ReadInt32();
    if (cnt <= 0) {
        TELEPHONY_LOGE("invalid parameter, cnt = %{public}d", cnt);
        return TELEPHONY_FAIL;
    }
    TELEPHONY_LOGI("call list size:%{public}d", cnt);
    CallsReportInfo callReportInfo;
    int32_t len = 0;
    const CallReportInfo *parcelPtr = nullptr;
    for (int i = 0; i < cnt; i++) {
        len = data.ReadInt32();
        if (len <= 0) {
            TELEPHONY_LOGE("invalid parameter, len = %d", len);
            return TELEPHONY_FAIL;
        }
        if (!data.ContainFileDescriptors()) {
            TELEPHONY_LOGW("sent raw data is less than 32k");
        }

        if ((parcelPtr = reinterpret_cast<const CallReportInfo *>(data.ReadRawData(len))) == nullptr) {
            TELEPHONY_LOGE("reading raw data failed, length = %d", len);
            if (reply.WriteInt32(0)) {
                TELEPHONY_LOGE("writing parcel failed");
            }
            return TELEPHONY_FAIL;
        }

        callReportInfo.callVec.push_back(*parcelPtr);
        TELEPHONY_LOGD("accountId:%d,state:%d", parcelPtr->accountId, parcelPtr->state);
    }
    callReportInfo.slotId = data.ReadInt32();
    TELEPHONY_LOGD("slotId:%d", callReportInfo.slotId);
    result = OnUpdateCallsReportInfo(callReportInfo);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("writing parcel failed");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusCallbackStub::OnUpdateDisconnectedCauseRequest(MessageParcel &data, MessageParcel &reply)
{
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusCallbackStub::OnUpdateEventReportRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_SUCCESS;
    const CellularCallEventInfo *parcelPtr = nullptr;
    int32_t len = data.ReadInt32();
    if (len <= TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Invalid parameter, len = %{public}d", len);
        return TELEPHONY_FAIL;
    }
    if (!data.ContainFileDescriptors()) {
        TELEPHONY_LOGW("sent raw data is less than 32k");
    }
    if ((parcelPtr = reinterpret_cast<const CellularCallEventInfo *>(data.ReadRawData(len))) == nullptr) {
        TELEPHONY_LOGE("reading raw data failed, length = %d", len);
        return TELEPHONY_FAIL;
    }

    result = OnUpdateEventResultInfo(*parcelPtr);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("writing parcel failed");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusCallbackStub::OnUpdateGetWaitingResultRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_SUCCESS;
    const CallWaitResponse *parcelPtr = nullptr;
    int32_t len = data.ReadInt32();
    if (len <= TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Invalid parameter, len = %{public}d", len);
        return TELEPHONY_FAIL;
    }
    if (!data.ContainFileDescriptors()) {
        TELEPHONY_LOGW("sent raw data is less than 32k");
    }
    if ((parcelPtr = reinterpret_cast<const CallWaitResponse *>(data.ReadRawData(len))) == nullptr) {
        TELEPHONY_LOGE("reading raw data failed, length = %d", len);
        return TELEPHONY_FAIL;
    }

    result = OnUpdateGetWaitingResult(*parcelPtr);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("writing parcel failed");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusCallbackStub::OnUpdateSetWaitingResultRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_SUCCESS;
    if (!data.ContainFileDescriptors()) {
        TELEPHONY_LOGW("sent raw data is less than 32k");
    }
    int32_t callWaitResult = data.ReadInt32();
    result = OnUpdateSetWaitingResult(callWaitResult);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("writing parcel failed");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusCallbackStub::OnUpdateGetRestrictionResultRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_SUCCESS;
    const CallRestrictionResponse *parcelPtr = nullptr;
    int32_t len = data.ReadInt32();
    if (len <= TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Invalid parameter, len = %{public}d", len);
        return TELEPHONY_FAIL;
    }
    if (!data.ContainFileDescriptors()) {
        TELEPHONY_LOGW("sent raw data is less than 32k");
    }
    if ((parcelPtr = reinterpret_cast<const CallRestrictionResponse *>(data.ReadRawData(len))) == nullptr) {
        TELEPHONY_LOGE("reading raw data failed, length = %d", len);
        return TELEPHONY_FAIL;
    }
    result = OnUpdateGetRestrictionResult(*parcelPtr);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("writing parcel failed");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusCallbackStub::OnUpdateSetRestrictionResultRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t error = TELEPHONY_SUCCESS;
    int32_t result = TELEPHONY_SUCCESS;
    if (!data.ContainFileDescriptors()) {
        TELEPHONY_LOGW("sent raw data is less than 32k");
    }
    result = data.ReadInt32();
    error = OnUpdateSetRestrictionResult(result);
    if (!reply.WriteInt32(error)) {
        TELEPHONY_LOGE("writing parcel failed");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusCallbackStub::OnUpdateGetTransferResultRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_SUCCESS;
    const CallTransferResponse *parcelPtr = nullptr;
    int32_t len = data.ReadInt32();
    if (len <= TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Invalid parameter, len = %{public}d", len);
        return TELEPHONY_FAIL;
    }
    if (!data.ContainFileDescriptors()) {
        TELEPHONY_LOGW("sent raw data is less than 32k");
    }
    if ((parcelPtr = reinterpret_cast<const CallTransferResponse *>(data.ReadRawData(len))) == nullptr) {
        TELEPHONY_LOGE("reading raw data failed, length = %d", len);
        return TELEPHONY_FAIL;
    }
    result = OnUpdateGetTransferResult(*parcelPtr);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("writing parcel failed");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusCallbackStub::OnUpdateSetTransferResultRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t error = TELEPHONY_SUCCESS;
    int32_t result = TELEPHONY_SUCCESS;
    if (!data.ContainFileDescriptors()) {
        TELEPHONY_LOGW("sent raw data is less than 32k");
    }
    result = data.ReadInt32();
    error = OnUpdateSetTransferResult(result);
    if (!reply.WriteInt32(error)) {
        TELEPHONY_LOGE("writing parcel failed");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusCallbackStub::OnUpdateGetCallClipResultRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_SUCCESS;
    const ClipResponse *parcelPtr = nullptr;
    int32_t len = data.ReadInt32();
    if (len <= TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Invalid parameter, len = %{public}d", len);
        return TELEPHONY_FAIL;
    }
    if (!data.ContainFileDescriptors()) {
        TELEPHONY_LOGW("sent raw data is less than 32k");
    }
    if ((parcelPtr = reinterpret_cast<const ClipResponse *>(data.ReadRawData(len))) == nullptr) {
        TELEPHONY_LOGE("reading raw data failed, length = %d", len);
        return TELEPHONY_FAIL;
    }
    result = OnUpdateGetCallClipResult(*parcelPtr);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("writing parcel failed");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusCallbackStub::OnUpdateGetCallClirResultRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_SUCCESS;
    const ClirResponse *parcelPtr = nullptr;
    int32_t len = data.ReadInt32();
    if (len <= TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("Invalid parameter, len = %{public}d", len);
        return TELEPHONY_FAIL;
    }
    if (!data.ContainFileDescriptors()) {
        TELEPHONY_LOGW("sent raw data is less than 32k");
    }
    if ((parcelPtr = reinterpret_cast<const ClirResponse *>(data.ReadRawData(len))) == nullptr) {
        TELEPHONY_LOGE("reading raw data failed, length = %d", len);
        return TELEPHONY_FAIL;
    }
    result = OnUpdateGetCallClirResult(*parcelPtr);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("writing parcel failed");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusCallbackStub::OnUpdateSetCallClirResultRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t error = TELEPHONY_SUCCESS;
    int32_t result = TELEPHONY_SUCCESS;
    if (!data.ContainFileDescriptors()) {
        TELEPHONY_LOGW("sent raw data is less than 32k");
    }
    result = data.ReadInt32();
    error = OnUpdateSetCallClirResult(result);
    if (!reply.WriteInt32(error)) {
        TELEPHONY_LOGE("writing parcel failed");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}
} // namespace Telephony
} // namespace OHOS
