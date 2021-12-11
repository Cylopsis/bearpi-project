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

#include "cellular_call_stub.h"
#include "cellular_call_register.h"

#include "cellular_call_supplement.h"
#include "emergency_utils.h"
#include "i_call_status_callback.h"
#include "module_service_utils.h"

namespace OHOS {
namespace Telephony {
int CellularCallStub::OnRemoteRequest(
    uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
{
    std::u16string myDescriptor = CellularCallStub::GetDescriptor();
    std::u16string remoteDescriptor = data.ReadInterfaceToken();
    if (myDescriptor != remoteDescriptor) {
        TELEPHONY_LOGE("descriptor checked fail");
        return ERR_DESCRIPTOR_INVALID;
    }

    auto itFunc = requestFuncMap_.find(code);
    if (itFunc != requestFuncMap_.end()) {
        auto requestFunc = itFunc->second;
        if (requestFunc != nullptr) {
            return (this->*requestFunc)(data, reply);
        }
    }
    TELEPHONY_LOGW("CellularCallStub::OnRemoteRequest, default case, need check.");
    return IPCObjectStub::OnRemoteRequest(code, data, reply, option);
}

CellularCallStub::CellularCallStub()
{
    Init();
    TELEPHONY_LOGD("CellularCallStub::CellularCallStub");
    requestFuncMap_[DIAL] = &CellularCallStub::DialInner;
    requestFuncMap_[END] = &CellularCallStub::EndInner;
    requestFuncMap_[REJECT] = &CellularCallStub::RejectInner;
    requestFuncMap_[ANSWER] = &CellularCallStub::AnswerInner;
    requestFuncMap_[URGENT_CALL] = &CellularCallStub::IsUrgentCallInner;
    requestFuncMap_[HOLD] = &CellularCallStub::HoldInner;
    requestFuncMap_[ACTIVE] = &CellularCallStub::ActiveInner;
    requestFuncMap_[SWAP] = &CellularCallStub::SwapInner;
    requestFuncMap_[JOIN] = &CellularCallStub::JoinInner;
    requestFuncMap_[SPLIT] = &CellularCallStub::SplitInner;
    requestFuncMap_[CALL_SUPPLEMENT] = &CellularCallStub::CallSupplementInner;
    requestFuncMap_[REGISTER_CALLBACK] = &CellularCallStub::RegisterCallBackInner;
    requestFuncMap_[UNREGISTER_CALLBACK] = &CellularCallStub::UnRegisterCallBackInner;
    requestFuncMap_[INITIATE_DTMF] = &CellularCallStub::InitiateDTMFInner;
    requestFuncMap_[CEASE_DTMF] = &CellularCallStub::CeaseDTMFInner;
    requestFuncMap_[TRANSMIT_DTMF] = &CellularCallStub::TransmitDTMFInner;
    requestFuncMap_[TRANSMIT_DTMF_STRING] = &CellularCallStub::TransmitDTMFStringInner;
    requestFuncMap_[SET_CALL_TRANSFER] = &CellularCallStub::SetCallTransferInner;
    requestFuncMap_[INQUIRE_CALL_TRANSFER] = &CellularCallStub::InquireCallTransferInner;
    requestFuncMap_[SET_CALL_WAITING] = &CellularCallStub::SetCallWaitingInner;
    requestFuncMap_[INQUIRE_CALL_WAITING] = &CellularCallStub::InquireCallWaitingInner;
    requestFuncMap_[SET_CALL_RESTRICTION] = &CellularCallStub::SetCallRestrictionInner;
    requestFuncMap_[INQUIRE_CALL_RESTRICTION] = &CellularCallStub::InquireCallRestrictionInner;
}

CellularCallStub::~CellularCallStub()
{
    TELEPHONY_LOGD("CellularCallStub::~CellularCallStub");
    requestFuncMap_.clear();
}

int CellularCallStub::DialInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::DialInner DIAL");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("CellularCallStub::OnRemoteRequest:size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::OnRemoteRequest data size error");
        return TELEPHONY_FAIL;
    }
    CellularCallInfo *pCallInfo = (CellularCallInfo *)data.ReadRawData(sizeof(CellularCallInfo));
    if (pCallInfo == nullptr) {
        TELEPHONY_LOGE("DialInner return, pCallInfo is nullptr.");
        return ERR_PARAMETER_INVALID;
    }
    reply.WriteInt32(Dial(*pCallInfo));
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::EndInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::EndInner END");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("CellularCallStub::OnRemoteRequest:size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::OnRemoteRequest data size error");
        return TELEPHONY_FAIL;
    }
    CellularCallInfo *pCallInfo = (CellularCallInfo *)data.ReadRawData(sizeof(CellularCallInfo));
    if (pCallInfo == nullptr) {
        TELEPHONY_LOGE("EndInner return, pCallInfo is nullptr.");
        return ERR_PARAMETER_INVALID;
    }
    reply.WriteInt32(End(*pCallInfo));
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::RejectInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::RejectInner REJECT");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("CellularCallStub::OnRemoteRequest:size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::OnRemoteRequest data size error");
        return TELEPHONY_FAIL;
    }
    CellularCallInfo *pCallInfo = (CellularCallInfo *)data.ReadRawData(sizeof(CellularCallInfo));
    if (pCallInfo == nullptr) {
        TELEPHONY_LOGE("RejectInner return, pCallInfo is nullptr.");
        return ERR_PARAMETER_INVALID;
    }
    reply.WriteInt32(Reject(*pCallInfo));
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::AnswerInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::AnswerInner ANSWER");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("CellularCallStub::OnRemoteRequest:size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::OnRemoteRequest data size error");
        return TELEPHONY_FAIL;
    }
    CellularCallInfo *pCallInfo = (CellularCallInfo *)data.ReadRawData(sizeof(CellularCallInfo));
    if (pCallInfo == nullptr) {
        TELEPHONY_LOGE("AnswerInner return, pCallInfo is nullptr.");
        return ERR_PARAMETER_INVALID;
    }
    reply.WriteInt32(Answer(*pCallInfo));
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::IsUrgentCallInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::IsUrgentCallInner EMERGENCY_CALL entry.");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("IsUrgentCallInner request size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::IsUrgentCallInner data size error");
        return TELEPHONY_FAIL;
    }

    std::string phoneNum = data.ReadString();
    int32_t slotId = data.ReadInt32();
    int32_t errorCode = data.ReadInt32();
    reply.WriteInt32(IsUrgentCall(phoneNum, slotId, errorCode));
    reply.WriteInt32(errorCode);
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::RegisterCallBackInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::RegisterCallBackInner REGISTER_CALLBACK entry.");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD(
        "CellularCallStub RegisterCallBackInner::OnRemoteRequest:size=%{public}d, MAX_SIZE=%{public}d\n", size,
        MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::OnRemoteRequest data size error");
        return TELEPHONY_FAIL;
    }

    int32_t result = ERR_PARAMETER_INVALID;
    sptr<IRemoteObject> remote = data.ReadRemoteObject();
    if (remote == nullptr) {
        TELEPHONY_LOGE("CellularCallStub::RegisterCallBackInner REGISTER_CALLBACK, remote is nullptr.");
        reply.WriteInt32(result);
        return result;
    }
    sptr<ICallStatusCallback> callback = iface_cast<ICallStatusCallback>(remote);
    if (DelayedSingleton<CellularCallRegister>::GetInstance() == nullptr) {
        return result;
    }
    result = DelayedSingleton<CellularCallRegister>::GetInstance()->RegisterCallManagerCallBack(callback);
    reply.WriteInt32(result);
    return result;
}

int CellularCallStub::UnRegisterCallBackInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::UnRegisterCallBackInner entry.");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD(
        "CellularCallStub UnRegisterCallBackInner::OnRemoteRequest:size=%{public}d, MAX_SIZE=%{public}d\n", size,
        MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::OnRemoteRequest data size error");
        return TELEPHONY_FAIL;
    }
    int result = DelayedSingleton<CellularCallRegister>::GetInstance()->UnRegisterCallManagerCallBack();
    reply.WriteInt32(result);
    return result;
}

int CellularCallStub::HoldInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::HoldInner");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("CellularCallStub::OnRemoteRequest:size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::OnRemoteRequest data size error");
        return TELEPHONY_FAIL;
    }
    reply.WriteInt32(Hold());
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::ActiveInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::ActiveInner");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("CellularCallStub::OnRemoteRequest:size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::OnRemoteRequest data size error");
        return TELEPHONY_FAIL;
    }
    reply.WriteInt32(Active());
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::SwapInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::SwapInner start");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("CellularCallStub::OnRemoteRequest:size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::OnRemoteRequest data size error");
        return TELEPHONY_FAIL;
    }
    reply.WriteInt32(Swap());
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::JoinInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::JoinInner");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("CellularCallStub::JoinInner size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::JoinInner data size error");
        return TELEPHONY_FAIL;
    }

    reply.WriteInt32(Join());
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::SplitInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::SplitInner");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("CellularCallStub::SplitInner size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::SplitInner data size error");
        return TELEPHONY_FAIL;
    }

    std::string splitString = data.ReadString();
    int32_t index = data.ReadInt32();
    reply.WriteInt32(Split(splitString, index));
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::CallSupplementInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::CallSupplementInner");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("CallSupplementInner size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CallSupplementInner data size error");
        return TELEPHONY_FAIL;
    }

    CallSupplementType type = static_cast<CallSupplementType>(data.ReadInt32());
    reply.WriteInt32(CallSupplement(type));
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::InitiateDTMFInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::InitiateDTMFInner");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("CellularCallStub::InitiateDTMFInner size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::InitiateDTMFInner data size error");
        return TELEPHONY_FAIL;
    }

    char *pDTMF = (char *)(data.ReadCString());
    if (pDTMF == nullptr) {
        TELEPHONY_LOGE("InitiateDTMFInner return, pDTMF is nullptr.");
        return ERR_PARAMETER_INVALID;
    }
    std::string phoneNum = data.ReadString();
    reply.WriteInt32(InitiateDTMF(*pDTMF, phoneNum));
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::CeaseDTMFInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::CeaseDTMFInner");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("CellularCallStub::CeaseDTMFInner size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::CeaseDTMFInner data size error");
        return TELEPHONY_FAIL;
    }

    std::string phoneNum = data.ReadString();
    reply.WriteInt32(CeaseDTMF(phoneNum));
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::TransmitDTMFInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::TransmitDTMFInner");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("TransmitDTMFInner size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::TransmitDTMFInner data size error");
        return TELEPHONY_FAIL;
    }

    char *pDTMF = (char *)(data.ReadCString());
    if (pDTMF == nullptr) {
        TELEPHONY_LOGE("TransmitDTMFInner return, pDTMF is nullptr.");
        return ERR_PARAMETER_INVALID;
    }
    std::string phoneNum = data.ReadString();
    reply.WriteInt32(TransmitDTMF(*pDTMF, phoneNum));
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::TransmitDTMFStringInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::TransmitDTMFStringInner");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("TransmitDTMFStringInner size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::TransmitDTMFStringInner data size error");
        return TELEPHONY_FAIL;
    }

    std::string dtmfCodeStr = data.ReadString();
    std::string phoneNum = data.ReadString();
    int32_t switchOn = data.ReadInt32();
    int32_t switchOff = data.ReadInt32();
    reply.WriteInt32(TransmitDTMFString(dtmfCodeStr, phoneNum, switchOn, switchOff));
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::SetCallTransferInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::SetCallTransferInner");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("SetCallTransferInner size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::SetCallTransferInner data size error");
        return TELEPHONY_FAIL;
    }

    CallTransferInfo *pCTInfo = (CallTransferInfo *)data.ReadRawData(sizeof(CallTransferInfo));
    if (pCTInfo == nullptr) {
        TELEPHONY_LOGE("SetCallTransferInner return, pCTInfo is nullptr.");
        return ERR_PARAMETER_INVALID;
    }
    int32_t slotId = data.ReadInt32();
    reply.WriteInt32(SetCallTransfer(*pCTInfo, slotId));
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::InquireCallTransferInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::InquireCallTransferInner");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("InquireCallTransferInner size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::InquireCallTransferInner data size error");
        return TELEPHONY_FAIL;
    }

    CallTransferType type = static_cast<CallTransferType>(data.ReadInt32());
    int32_t slotId = data.ReadInt32();
    reply.WriteInt32(InquireCallTransfer(type, slotId));
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallStub::SetCallWaitingInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::SetCallWaitingInner");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("SetCallWaitingInner size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::SetCallWaitingInner data size error");
        return TELEPHONY_FAIL;
    }

    bool enable = data.ReadBool();
    int32_t slotId = data.ReadInt32();
    reply.WriteInt32(SetCallWaiting(enable, slotId));
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallStub::InquireCallWaitingInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::InquireCallWaitingInner");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("InquireCallWaitingInner size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::InquireCallWaitingInner data size error");
        return TELEPHONY_FAIL;
    }
    int32_t slotId = data.ReadInt32();
    TELEPHONY_LOGD("CellularCallStub::InquireCallWaitingInner data.ReadInt32()");
    reply.WriteInt32(InquireCallWaiting(slotId));
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallStub::SetCallRestrictionInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::SetCallRestrictionInner");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("SetCallRestrictionInner size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::SetCallRestrictionInner data size error");
        return TELEPHONY_FAIL;
    }

    CallRestrictionInfo *pCRInfo = (CallRestrictionInfo *)data.ReadRawData(sizeof(CallRestrictionInfo));
    if (pCRInfo == nullptr) {
        TELEPHONY_LOGE("SetCallRestrictionInner return, pCRInfo is nullptr.");
        return ERR_PARAMETER_INVALID;
    }
    int32_t slotId = data.ReadInt32();
    reply.WriteInt32(SetCallRestriction(*pCRInfo, slotId));
    return TELEPHONY_SUCCESS;
}

int32_t CellularCallStub::InquireCallRestrictionInner(MessageParcel &data, MessageParcel &reply)
{
    TELEPHONY_LOGD("CellularCallStub::InquireCallRestrictionInner");
    int32_t size = data.ReadInt32();
    TELEPHONY_LOGD("InquireCallRestrictionInner size=%{public}d, MAX_SIZE=%{public}d\n", size, MAX_SIZE);
    size = ((size > MAX_SIZE) ? 0 : size);
    if (size <= 0) {
        TELEPHONY_LOGE("CellularCallStub::InquireCallRestrictionInner data size error");
        return TELEPHONY_FAIL;
    }

    CallRestrictionType facType = static_cast<CallRestrictionType>(data.ReadInt32());
    int32_t slotId = data.ReadInt32();
    reply.WriteInt32(InquireCallRestriction(facType, slotId));
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::Dial(const CellularCallInfo &callInfo)
{
    if (!IsValidSlotId(callInfo.slotId)) {
        TELEPHONY_LOGD("CellularCallStub::Dial return, invalid slot id");
        return ERR_INVALID_SLOT_ID;
    }
    if (TYPE_CS == callInfo.callType) {
        auto control = GetCsControl(slotId_);
        if (control == nullptr) {
            TELEPHONY_LOGD("CellularCallStub::Dial return, control is nullptr");
            return ERR_NULL_POINTER;
        }
        return control->Dial(callInfo);
    } else if (TYPE_IMS == callInfo.callType) {
    }
    TELEPHONY_LOGE("CellularCallStub::Dial return, call type error.");
    return ERR_PARAMETER_INVALID;
}

int CellularCallStub::End(const CellularCallInfo &callInfo)
{
    if (!IsValidSlotId(callInfo.slotId)) {
        TELEPHONY_LOGD("CellularCallStub::End return, invalid slot id");
        return ERR_INVALID_SLOT_ID;
    }
    if (TYPE_CS == callInfo.callType) {
        auto control = GetCsControl(slotId_);
        if (control == nullptr) {
            TELEPHONY_LOGD("CellularCallStub::End return, control is nullptr");
            return ERR_NULL_POINTER;
        }
        return control->End(callInfo);
    } else if (TYPE_IMS == callInfo.callType) {
    }
    TELEPHONY_LOGE("CellularCallStub::End return, call type error.");
    return ERR_PARAMETER_INVALID;
}

int CellularCallStub::Reject(const CellularCallInfo &callInfo)
{
    if (!IsValidSlotId(callInfo.slotId)) {
        TELEPHONY_LOGD("CellularCallStub::Reject return, invalid slot id");
        return ERR_INVALID_SLOT_ID;
    }
    if (TYPE_CS == callInfo.callType) {
        auto control = GetCsControl(slotId_);
        if (control == nullptr) {
            TELEPHONY_LOGD("CellularCallStub::Reject return, control is nullptr");
            return ERR_NULL_POINTER;
        }
        return control->Reject(callInfo);
    } else if (TYPE_IMS == callInfo.callType) {
    }
    TELEPHONY_LOGE("CellularCallStub::Reject return, call type error.");
    return ERR_PARAMETER_INVALID;
}

int CellularCallStub::Answer(const CellularCallInfo &callInfo)
{
    if (!IsValidSlotId(callInfo.slotId)) {
        TELEPHONY_LOGD("CellularCallStub::Answer return, invalid slot id");
        return ERR_INVALID_SLOT_ID;
    }
    if (TYPE_CS == callInfo.callType) {
        auto control = GetCsControl(slotId_);
        if (control == nullptr) {
            TELEPHONY_LOGD("CellularCallStub::Answer return, control is nullptr");
            return ERR_NULL_POINTER;
        }
        return control->Answer(callInfo);
    } else if (TYPE_IMS == callInfo.callType) {
    }
    TELEPHONY_LOGE("CellularCallStub::Answer return, call type error.");
    return ERR_PARAMETER_INVALID;
}

int CellularCallStub::RegisterCallManagerCallBack(const sptr<ICallStatusCallback> &callback)
{
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::UnRegisterCallManagerCallBack()
{
    return TELEPHONY_SUCCESS;
}

int CellularCallStub::Hold()
{
    auto control = GetCsControl(slotId_);
    if (control == nullptr) {
        TELEPHONY_LOGD("CellularCallStub::Hold return, control is nullptr");
        return ERR_NULL_POINTER;
    }
    return control->Hold();
}

int CellularCallStub::Active()
{
    auto control = GetCsControl(slotId_);
    if (control == nullptr) {
        TELEPHONY_LOGD("CellularCallStub::Active return, control is nullptr");
        return ERR_NULL_POINTER;
    }
    return control->Active();
}

int CellularCallStub::Swap()
{
    auto control = GetCsControl(slotId_);
    if (control == nullptr) {
        TELEPHONY_LOGD("CellularCallStub::Swap return, control is nullptr");
        return ERR_NULL_POINTER;
    }
    return control->Swap();
}

int CellularCallStub::Join()
{
    auto control = GetCsControl(slotId_);
    if (control == nullptr) {
        TELEPHONY_LOGD("CellularCallStub::Join return, control is nullptr");
        return ERR_NULL_POINTER;
    }
    return control->Join();
}

int CellularCallStub::Split(const std::string &splitString, int32_t index)
{
    auto control = GetCsControl(slotId_);
    if (control == nullptr) {
        TELEPHONY_LOGD("CellularCallStub::Split return, control is nullptr");
        return ERR_NULL_POINTER;
    }
    return control->Split(splitString, index);
}

int CellularCallStub::CallSupplement(CallSupplementType type)
{
    auto control = GetCsControl(slotId_);
    if (control == nullptr) {
        TELEPHONY_LOGD("CallSupplement return, control is nullptr");
        return ERR_NULL_POINTER;
    }
    return control->CallSupplement(type);
}

int CellularCallStub::InitiateDTMF(char cDTMFCode, const std::string &phoneNum)
{
    auto control = GetCsControl(slotId_);
    if (control == nullptr) {
        TELEPHONY_LOGD("CellularCallStub::InitiateDTMF return, control is nullptr");
        return ERR_NULL_POINTER;
    }
    return control->InitiateDTMF(cDTMFCode, phoneNum);
}

int CellularCallStub::CeaseDTMF(const std::string &phoneNum)
{
    auto control = GetCsControl(slotId_);
    if (control == nullptr) {
        TELEPHONY_LOGD("CellularCallStub::CeaseDTMF return, control is nullptr");
        return ERR_NULL_POINTER;
    }
    return control->CeaseDTMF(phoneNum);
}

int CellularCallStub::TransmitDTMF(char cDTMFCode, const std::string &phoneNum)
{
    auto control = GetCsControl(slotId_);
    if (control == nullptr) {
        TELEPHONY_LOGD("CellularCallStub::TransmitDTMF return, control is nullptr");
        return ERR_NULL_POINTER;
    }
    return control->TransmitDTMF(cDTMFCode, phoneNum);
}

int32_t CellularCallStub::TransmitDTMFString(
    const std::string &dtmfCodeStr, const std::string &phoneNum, int32_t switchOn, int32_t switchOff)
{
    auto control = GetCsControl(slotId_);
    if (control == nullptr) {
        TELEPHONY_LOGD("CellularCallStub::TransmitDTMFString return, control is nullptr");
        return ERR_NULL_POINTER;
    }
    return control->TransmitDTMFString(dtmfCodeStr, switchOn, switchOff);
}

int32_t CellularCallStub::SetCallTransfer(const CallTransferInfo &cTInfo, int32_t slotId)
{
    if (!IsValidSlotId(slotId)) {
        TELEPHONY_LOGD("CellularCallStub::SetCallTransfer return, invalid slot id");
        return ERR_INVALID_SLOT_ID;
    }
    CellularCallSupplement cellularCallSupplement;
    return cellularCallSupplement.SetCallTransfer(cTInfo);
}

int32_t CellularCallStub::InquireCallTransfer(CallTransferType type, int32_t slotId)
{
    TELEPHONY_LOGD("CellularCallStub::InquireCallTransfer");
    if (!IsValidSlotId(slotId)) {
        TELEPHONY_LOGD("CellularCallStub::InquireCallTransfer return, invalid slot id");
        return ERR_INVALID_SLOT_ID;
    }
    CellularCallSupplement cellularCallSupplement;
    return cellularCallSupplement.InquireCallTransfer(type);
}

void CellularCallStub::Init()
{
    ModuleServiceUtils obtain;
    std::vector<int32_t> slotVector = obtain.GetSlotInfo();
    for (auto &it : slotVector) {
        auto control = std::make_shared<CSControl>();
        control->SetSlotId(it);
        controlMap_.insert(std::make_pair(it, control));
    }
}

std::shared_ptr<CSControl> CellularCallStub::GetCsControl(int32_t slotId)
{
    return controlMap_[slotId];
}

int32_t CellularCallStub::SetCallWaiting(bool activate, int32_t slotId)
{
    if (!IsValidSlotId(slotId)) {
        TELEPHONY_LOGD("CellularCallStub::SetCallWaiting return, invalid slot id");
        return ERR_INVALID_SLOT_ID;
    }
    CellularCallSupplement cellularCallSupplement;
    return cellularCallSupplement.SetCallWaiting(activate);
}

int32_t CellularCallStub::InquireCallWaiting(int32_t slotId)
{
    TELEPHONY_LOGD("CellularCallStub::InquireCallWaiting");
    if (!IsValidSlotId(slotId)) {
        TELEPHONY_LOGD("CellularCallStub::InquireCallWaiting return, invalid slot id");
        return ERR_INVALID_SLOT_ID;
    }
    CellularCallSupplement cellularCallSupplement;
    return cellularCallSupplement.InquireCallWaiting();
}

int32_t CellularCallStub::SetCallRestriction(const CallRestrictionInfo &crInfo, int32_t slotId)
{
    TELEPHONY_LOGD("CellularCallStub::SetCallRestriction");
    if (!IsValidSlotId(slotId)) {
        TELEPHONY_LOGD("CellularCallStub::SetCallRestriction return, invalid slot id");
        return ERR_INVALID_SLOT_ID;
    }
    CellularCallSupplement cellularCallSupplement;
    return cellularCallSupplement.SetCallRestriction(crInfo);
}

int32_t CellularCallStub::InquireCallRestriction(CallRestrictionType facType, int32_t slotId)
{
    TELEPHONY_LOGD("CellularCallStub::InquireCallRestriction");
    if (!IsValidSlotId(slotId)) {
        TELEPHONY_LOGD("CellularCallStub::InquireCallRestriction return, invalid slot id");
        return ERR_INVALID_SLOT_ID;
    }
    CellularCallSupplement cellularCallSupplement;
    return cellularCallSupplement.InquireCallRestriction(facType);
}

int CellularCallStub::IsUrgentCall(const std::string &phoneNum, int32_t slotId, int32_t &errorCode)
{
    if (!IsValidSlotId(slotId)) {
        TELEPHONY_LOGD("CellularCallStub::IsUrgentCall return, invalid slot id");
        errorCode = ERR_INVALID_SLOT_ID;
        return false;
    }
    EmergencyUtils emergencyUtils;
    return emergencyUtils.IsEmergencyCall(phoneNum, slotId);
}

bool CellularCallStub::IsValidSlotId(int32_t slotId) const
{
    return slotId == CoreManager::DEFAULT_SLOT_ID;
}
} // namespace Telephony
} // namespace OHOS
