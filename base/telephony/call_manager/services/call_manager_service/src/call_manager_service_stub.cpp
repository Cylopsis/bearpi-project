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

#include "call_manager_service_stub.h"

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

#include "message_option.h"
#include "message_parcel.h"

#include "call_control_manager.h"
#include "cellular_call_types.h"

namespace OHOS {
namespace Telephony {
CallManagerServiceStub::CallManagerServiceStub()
{
    memberFuncMap_[INTERFACE_REGISTER_CALLBACK] = &CallManagerServiceStub::RegisterCallBackRequest;
    memberFuncMap_[INTERFACE_DIAL_CALL] = &CallManagerServiceStub::DialCallRequest;
    memberFuncMap_[INTERFACE_ANSWER_CALL] = &CallManagerServiceStub::AcceptCallRequest;
    memberFuncMap_[INTERFACE_REJECT_CALL] = &CallManagerServiceStub::RejectCallRequest;
    memberFuncMap_[INTERFACE_HOLD_CALL] = &CallManagerServiceStub::HoldCallRequest;
    memberFuncMap_[INTERFACE_UNHOLD_CALL] = &CallManagerServiceStub::UnHoldCallRequest;
    memberFuncMap_[INTERFACE_DISCONNECT_CALL] = &CallManagerServiceStub::HangUpCallRequest;
    memberFuncMap_[INTERFACE_GET_CALL_STATE] = &CallManagerServiceStub::GetCallStateRequest;
    memberFuncMap_[INTERFACE_SWAP_CALL] = &CallManagerServiceStub::SwitchCallRequest;
    memberFuncMap_[INTERFACE_HAS_CALL] = &CallManagerServiceStub::HasCallRequest;
    memberFuncMap_[INTERFACE_IS_NEW_CALL_ALLOWED] = &CallManagerServiceStub::IsNewCallAllowedRequest;
    memberFuncMap_[INTERFACE_IS_RINGING] = &CallManagerServiceStub::IsRingingRequest;
    memberFuncMap_[INTERFACE_IS_EMERGENCY_CALL] = &CallManagerServiceStub::IsInEmergencyCallRequest;
    memberFuncMap_[INTERFACE_IS_EMERGENCY_NUMBER] = &CallManagerServiceStub::IsEmergencyPhoneNumberRequest;
    memberFuncMap_[INTERFACE_IS_FORMAT_NUMBER] = &CallManagerServiceStub::FormatPhoneNumberRequest;
    memberFuncMap_[INTERFACE_IS_FORMAT_NUMBER_E164] = &CallManagerServiceStub::FormatPhoneNumberToE164Request;
    memberFuncMap_[INTERFACE_COMBINE_CONFERENCE] = &CallManagerServiceStub::JoinConferenceRequest;
    memberFuncMap_[INTERFACE_START_DTMF] = &CallManagerServiceStub::StartDtmfRequest;
    memberFuncMap_[INTERFACE_SEND_DTMF] = &CallManagerServiceStub::SendDtmfRequest;
    memberFuncMap_[INTERFACE_STOP_DTMF] = &CallManagerServiceStub::StopDtmfRequest;
    memberFuncMap_[INTERFACE_SEND_DTMF_BUNCH] = &CallManagerServiceStub::SendDtmfBunchRequest;
    memberFuncMap_[INTERFACE_GET_CALL_WAITING] = &CallManagerServiceStub::GetCallWaitingRequest;
    memberFuncMap_[INTERFACE_SET_CALL_WAITING] = &CallManagerServiceStub::SetCallWaitingRequest;
    memberFuncMap_[INTERFACE_GET_MAINID] = &CallManagerServiceStub::GetMainCallIdRequest;
    memberFuncMap_[INTERFACE_GET_SUBCALL_LIST_ID] = &CallManagerServiceStub::GetSubCallIdListRequest;
    memberFuncMap_[INTERFACE_GET_CALL_LIST_ID_FOR_CONFERENCE] =
        &CallManagerServiceStub::GetCallIdListForConferenceRequest;
}

CallManagerServiceStub::~CallManagerServiceStub()
{
    memberFuncMap_.clear();
}

int CallManagerServiceStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string myDescriptor = CallManagerServiceStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (myDescriptor != remoteDescriptor) {
        TELEPHONY_LOGE("descriptor checked fail !");
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

int32_t CallManagerServiceStub::RegisterCallBackRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_FAIL;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        TELEPHONY_LOGE("callback ptr is nullptr.");
        reply.WriteInt32(result);
        return result;
    }
    sptr<ICallAbilityCallback> callback = iface_cast<ICallAbilityCallback>(remote);
    result = RegisterCallBack(callback);
    reply.WriteInt32(result);
    return result;
}

int32_t CallManagerServiceStub::DialCallRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_FAIL;
    std::u16string callNumber = data.ReadString16();
    int32_t accountId = data.ReadInt32();
    int32_t videoState = data.ReadInt32();
    int32_t dialScene = data.ReadInt32();
    int32_t dialType = data.ReadInt32();
    AppExecFwk::PacMap dialInfo;
    dialInfo.PutIntValue("accountId", accountId);
    dialInfo.PutIntValue("videoState", videoState);
    dialInfo.PutIntValue("dialScene", dialScene);
    dialInfo.PutIntValue("dialType", dialType);
    if (!data.ContainFileDescriptors()) {
        TELEPHONY_LOGW("sent raw data is less than 32k");
    }
    if (callNumber.length() > ACCOUNT_NUMBER_MAX_LENGTH) {
        TELEPHONY_LOGE("the account number length exceeds the limit");
        return CALL_MANAGER_PHONE_NUMBER_INVALID;
    }
    result = DialCall(callNumber, dialInfo);
    TELEPHONY_LOGD("result:%{public}d", result);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return result;
}

int32_t CallManagerServiceStub::AcceptCallRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t callId = data.ReadInt32();
    int32_t videoState = data.ReadInt32();
    int32_t result = AnswerCall(callId, videoState);
    TELEPHONY_LOGD("result:%{public}d", result);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return result;
}

int32_t CallManagerServiceStub::RejectCallRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t callId = data.ReadInt32();
    bool isSendSms = data.ReadBool();
    std::u16string content = data.ReadString16();
    int32_t result = RejectCall(callId, isSendSms, content);
    TELEPHONY_LOGD("result:%{public}d", result);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return result;
}

int32_t CallManagerServiceStub::HangUpCallRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t callId = data.ReadInt32();
    int32_t result = HangUpCall(callId);
    TELEPHONY_LOGD("result:%{public}d", result);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return result;
}

int32_t CallManagerServiceStub::GetCallStateRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = GetCallState();
    TELEPHONY_LOGD("result:%{public}d", result);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerServiceStub::HoldCallRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t callId = data.ReadInt32();
    int32_t result = HoldCall(callId);
    TELEPHONY_LOGD("result:%{public}d", result);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return result;
}

int32_t CallManagerServiceStub::UnHoldCallRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t callId = data.ReadInt32();
    int32_t result = UnHoldCall(callId);
    TELEPHONY_LOGD("result:%{public}d", result);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return result;
}

int32_t CallManagerServiceStub::SwitchCallRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t callId = data.ReadInt32();
    int32_t result = SwitchCall(callId);
    TELEPHONY_LOGD("result:%{public}d", result);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return result;
}

int32_t CallManagerServiceStub::HasCallRequest(MessageParcel &data, MessageParcel &reply)
{
    bool result = HasCall();
    if (!reply.WriteBool(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerServiceStub::IsNewCallAllowedRequest(MessageParcel &data, MessageParcel &reply)
{
    bool result = IsNewCallAllowed();
    if (!reply.WriteBool(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerServiceStub::IsRingingRequest(MessageParcel &data, MessageParcel &reply)
{
    bool result = IsRinging();
    TELEPHONY_LOGD("result:%{public}d", result);
    if (!reply.WriteBool(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerServiceStub::IsInEmergencyCallRequest(MessageParcel &data, MessageParcel &reply)
{
    bool result = IsInEmergencyCall();
    TELEPHONY_LOGD("result:%{public}d", result);
    if (!reply.WriteBool(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerServiceStub::StartDtmfRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_FAIL;
    int32_t callId = data.ReadInt32();
    char str = data.ReadInt8();
    result = StartDtmf(callId, str);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerServiceStub::SendDtmfRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_FAIL;
    int32_t callId = data.ReadInt32();
    char str = data.ReadInt8();
    result = SendDtmf(callId, str);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerServiceStub::SendDtmfBunchRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_FAIL;
    int32_t callId = data.ReadInt32();
    std::u16string str = data.ReadString16();
    int32_t on = data.ReadInt32();
    int32_t off = data.ReadInt32();
    result = SendBurstDtmf(callId, str, on, off);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerServiceStub::StopDtmfRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_FAIL;
    int32_t callId = data.ReadInt32();
    result = StopDtmf(callId);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerServiceStub::GetCallWaitingRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_FAIL;
    int32_t slotId = data.ReadInt32();
    result = GetCallWaiting(slotId);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerServiceStub::SetCallWaitingRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t result = TELEPHONY_FAIL;
    int32_t slotId = data.ReadInt32();
    bool activate = data.ReadBool();
    result = SetCallWaiting(slotId, activate);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerServiceStub::JoinConferenceRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t mainCallId = data.ReadInt32();
    int32_t result = CombineConference(mainCallId);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerServiceStub::IsEmergencyPhoneNumberRequest(MessageParcel &data, MessageParcel &reply)
{
    std::u16string callNumber = data.ReadString16();
    int32_t slotId = data.ReadInt32();
    int32_t errorCode = TELEPHONY_SUCCESS;
    bool result = IsEmergencyPhoneNumber(callNumber, slotId, errorCode);
    TELEPHONY_LOGD("result:%{public}d", result);
    if (!reply.WriteBool(result) || !reply.WriteInt32(errorCode)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerServiceStub::FormatPhoneNumberRequest(MessageParcel &data, MessageParcel &reply)
{
    std::u16string callNumber = data.ReadString16();
    std::u16string countryCode = data.ReadString16();
    std::u16string formatNumber;
    int32_t result = FormatPhoneNumber(callNumber, countryCode, formatNumber);
    TELEPHONY_LOGD("result:%{public}d", result);
    if (!reply.WriteString16(formatNumber) || !reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerServiceStub::FormatPhoneNumberToE164Request(MessageParcel &data, MessageParcel &reply)
{
    std::u16string callNumber = data.ReadString16();
    std::u16string countryCode = data.ReadString16();
    std::u16string formatNumber;
    int32_t result = FormatPhoneNumberToE164(callNumber, countryCode, formatNumber);
    TELEPHONY_LOGD("result:%{public}d", result);
    if (!reply.WriteString16(formatNumber) || !reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerServiceStub::GetMainCallIdRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t callId = data.ReadInt32();
    int32_t result = GetMainCallId(callId);
    if (!reply.WriteInt32(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerServiceStub::GetSubCallIdListRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t callId = data.ReadInt32();
    std::vector<std::u16string> result = GetSubCallIdList(callId);
    if (!reply.WriteString16Vector(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallManagerServiceStub::GetCallIdListForConferenceRequest(MessageParcel &data, MessageParcel &reply)
{
    int32_t callId = data.ReadInt32();
    std::vector<std::u16string> result = GetCallIdListForConference(callId);
    if (!reply.WriteString16Vector(result)) {
        TELEPHONY_LOGE("fail to write parcel");
        return TELEPHONY_WRITE_REPLY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}
} // namespace Telephony
} // namespace OHOS