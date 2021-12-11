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

#ifndef CALL_MANAGER_CONNECT_H
#define CALL_MANAGER_CONNECT_H
#include <string>
#include <map>
#include <list>
#include <mutex>

#include "iservice_registry.h"
#include "system_ability.h"
#include "system_ability_definition.h"
#include "if_system_ability_manager.h"
#include "pac_map.h"
#include "refbase.h"
#include "rwlock.h"
#include "iremote_object.h"
#include "iremote_stub.h"

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"
#include "call_manager_service_proxy.h"
#include "i_call_ability_callback.h"

namespace OHOS {
namespace Telephony {
constexpr int32_t invalidCallId = -1;
class CallInfoManager {
public:
    static int32_t CallDetailsChange(const CallAttributeInfo &info)
    {
        TELEPHONY_LOGE("CallDetailsChange Start");
        bool newCallFlag = true;
        std::lock_guard<std::mutex> lock(mutex_);
        std::list<CallAttributeInfo>::iterator it = callInfoList_.begin();
        for (; it != callInfoList_.end(); it++) {
            if ((*it).callId == info.callId) {
                newCallFlag = false;
                if (info.callState == TelCallState::CALL_STATUS_DISCONNECTED) {
                    callInfoList_.erase(it);
                    break;
                }
                if ((*it).callState != info.callState) {
                    *it = info;
                    break;
                }
            }
        }
        if (newCallFlag) {
            callInfoList_.push_back(info);
        }

        if (info.callState == TelCallState::CALL_STATUS_DIALING) {
            OffHookStateUpdate();
        }
        if (info.callState == TelCallState::CALL_STATUS_ALERTING) {
            OffHookStateUpdate();
        }
        if (info.callState == TelCallState::CALL_STATUS_ACTIVE) {
            OffHookStateUpdate();
        }
        if (info.callState == TelCallState::CALL_STATUS_HOLDING) {
            OffHookStateUpdate();
        }
        TELEPHONY_LOGE("CallDetailsChange leave");
        return TELEPHONY_SUCCESS;
    }

    static void OffHookStateUpdate()
    {
        offHookMutex_.unlock();
    }

    static void WaitForOffHookState()
    {
        offHookMutex_.lock();
        offHookMutex_.lock();
        offHookMutex_.unlock();
    }

    static int32_t GetCallId(std::string &number)
    {
        int32_t callId = invalidCallId;
        std::lock_guard<std::mutex> lock(mutex_);
        std::list<CallAttributeInfo>::iterator it = callInfoList_.begin();
        for (; it != callInfoList_.end(); it++) {
            if (strcmp((*it).accountNumber, number.c_str()) == 0) {
                callId = (*it).callId;
                break;
            }
        }
        return callId;
    }

private:
    static std::list<CallAttributeInfo> callInfoList_;
    static std::mutex mutex_;
    static std::mutex offHookMutex_;
};

std::list<CallAttributeInfo> CallInfoManager::callInfoList_;
std::mutex CallInfoManager::mutex_;
std::mutex CallInfoManager::offHookMutex_;

class CallAbilityCallbackStub : public IRemoteStub<ICallAbilityCallback> {
public:
    CallAbilityCallbackStub()
    {
        memberFuncMap_[UPDATE_CALL_STATE_INFO] = &CallAbilityCallbackStub::OnUpdateCallStateInfoRequest;
        memberFuncMap_[UPDATE_CALL_EVENT] = &CallAbilityCallbackStub::OnUpdateCallEventRequest;
        memberFuncMap_[UPDATE_CALL_SUPPLEMENT_REQUEST] = &CallAbilityCallbackStub::OnUpdateSupplementResultRequest;
    }

    ~CallAbilityCallbackStub()
    {
        memberFuncMap_.clear();
    }

    int OnRemoteRequest(uint32_t code, MessageParcel &data, MessageParcel &reply, MessageOption &option)
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

private:
    using CallAbilityCallbackFunc = int32_t (CallAbilityCallbackStub::*)(MessageParcel &data, MessageParcel &reply);

    int32_t OnUpdateCallStateInfoRequest(MessageParcel &data, MessageParcel &reply)
    {
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

        int32_t result = OnCallDetailsChange(*parcelPtr);
        if (!reply.WriteInt32(result)) {
            TELEPHONY_LOGE("writing parcel failed");
            return TELEPHONY_FAIL;
        }
        return TELEPHONY_SUCCESS;
    }

    int32_t OnUpdateCallEventRequest(MessageParcel &data, MessageParcel &reply)
    {
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

        int32_t result = OnCallEventChange(*parcelPtr);
        if (!reply.WriteInt32(result)) {
            TELEPHONY_LOGE("writing parcel failed");
            return TELEPHONY_FAIL;
        }
        return TELEPHONY_SUCCESS;
    }

    int32_t OnUpdateSupplementResultRequest(MessageParcel &data, MessageParcel &reply)
    {
        AppExecFwk::PacMap resultInfo;
        CallResultReportId reportId = static_cast<CallResultReportId>(data.ReadInt32());
        resultInfo.PutIntValue("result", data.ReadInt32());
        switch (reportId) {
            case CallResultReportId::GET_CALL_WAITING_REPORT_ID:
            case CallResultReportId::GET_CALL_RESTRICTION_REPORT_ID:
                resultInfo.PutIntValue("status", data.ReadInt32());
                resultInfo.PutIntValue("classCw", data.ReadInt32());
                break;
            default:
                break;
        }
        if (!data.ContainFileDescriptors()) {
            TELEPHONY_LOGW("sent raw data is less than 32k");
        }
        int32_t result = OnSupplementResult(reportId, resultInfo);
        if (!reply.WriteInt32(result)) {
            TELEPHONY_LOGE("writing parcel failed");
            return TELEPHONY_FAIL;
        }
        return TELEPHONY_SUCCESS;
    }

private:
    std::map<uint32_t, CallAbilityCallbackFunc> memberFuncMap_;
};

class CallAbilityCallback : public CallAbilityCallbackStub {
public:
    explicit CallAbilityCallback() {}
    ~CallAbilityCallback() {}

    int32_t OnCallDetailsChange(const CallAttributeInfo &info)
    {
        return CallInfoManager::CallDetailsChange(info);
    }

    int32_t OnCallEventChange(const CallEventInfo &info)
    {
        TELEPHONY_LOGI("UpdateCallEvent success!");
        return TELEPHONY_SUCCESS;
    }

    int32_t OnSupplementResult(CallResultReportId reportId, AppExecFwk::PacMap &resultInfo)
    {
        TELEPHONY_LOGI("OnSupplementResult success!");
        return TELEPHONY_SUCCESS;
    }
};

class CallManagerConnect {
public:
    CallManagerConnect()
    {
        callAbilityCallbackPtr_ = nullptr;
        callManagerServicePtr_ = nullptr;
        systemAbilityId_ = TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID;
    }

    ~CallManagerConnect()
    {
        if (callManagerServicePtr_) {
            callManagerServicePtr_.clear();
            callManagerServicePtr_ = nullptr;
        }
    }

    int32_t Init(int32_t systemAbilityId)
    {
        TELEPHONY_LOGD("Enter CallManagerIpcClient::Init,systemAbilityId:%d\n", systemAbilityId);
        systemAbilityId_ = systemAbilityId;
        int32_t result = ConnectService();
        TELEPHONY_LOGD("Connect service: %X\n", result);
        return result;
    }

    void UnInit()
    {
        DisconnectService();
        TELEPHONY_LOGD("Disconnect service\n");
    }

    int32_t DialCall(std::u16string number, AppExecFwk::PacMap &extras) const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->DialCall(number, extras);
        }
        return TELEPHONY_FAIL;
    }

    int32_t AnswerCall(int32_t callId, int32_t videoState) const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->AnswerCall(callId, videoState);
        }
        return TELEPHONY_FAIL;
    }

    int32_t RejectCall(int32_t callId, bool isSendSms, std::u16string content) const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->RejectCall(callId, isSendSms, content);
        }
        return TELEPHONY_FAIL;
    }

    int32_t HoldCall(int32_t callId) const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->HoldCall(callId);
        }
        return TELEPHONY_FAIL;
    }

    int32_t UnHoldCall(int32_t callId) const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->UnHoldCall(callId);
        }
        return TELEPHONY_FAIL;
    }

    int32_t HangUpCall(int32_t callId) const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->HangUpCall(callId);
        }
        return TELEPHONY_FAIL;
    }

    int32_t GetCallState() const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->GetCallState();
        }
        return TELEPHONY_FAIL;
    }

    int32_t SwitchCall(int32_t callId) const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->SwitchCall(callId);
        }
        return TELEPHONY_FAIL;
    }

    bool HasCall() const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->HasCall();
        }
        return false;
    }

    bool IsNewCallAllowed() const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->IsNewCallAllowed();
        }
        return false;
    }

    bool IsRinging() const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->IsRinging();
        }
        return false;
    }

    bool IsInEmergencyCall() const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->IsInEmergencyCall();
        }
        return false;
    }

    int32_t StartDtmf(int32_t callId, char c) const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->StartDtmf(callId, c);
        }
        return TELEPHONY_FAIL;
    }

    int32_t SendDtmf(int32_t callId, char c) const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->SendDtmf(callId, c);
        }
        return TELEPHONY_FAIL;
    }

    int32_t StopDtmf(int32_t callId) const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->StopDtmf(callId);
        }
        return TELEPHONY_FAIL;
    }

    int32_t SendBurstDtmf(int32_t callId, std::u16string str, int32_t on, int32_t off) const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->SendBurstDtmf(callId, str, on, off);
        }
        return TELEPHONY_FAIL;
    }

    int32_t GetCallWaiting(int32_t slotId) const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->GetCallWaiting(slotId);
        }
        return TELEPHONY_FAIL;
    }

    int32_t SetCallWaiting(int32_t slotId, bool activate) const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->SetCallWaiting(slotId, activate);
        }
        return TELEPHONY_FAIL;
    }

    int32_t CombineConference(int mainCallId) const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->CombineConference(mainCallId);
        }
        return TELEPHONY_FAIL;
    }

    bool IsEmergencyPhoneNumber(std::u16string &number, int32_t slotId, int32_t &errorCode) const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->IsEmergencyPhoneNumber(number, slotId, errorCode);
        }
        return false;
    }

    int32_t FormatPhoneNumber(
        std::u16string &number, std::u16string &countryCode, std::u16string &formatNumber) const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->FormatPhoneNumber(number, countryCode, formatNumber);
        }
        return TELEPHONY_FAIL;
    }

    int32_t FormatPhoneNumberToE164(
        std::u16string &number, std::u16string &countryCode, std::u16string &formatNumber) const
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->FormatPhoneNumberToE164(number, countryCode, formatNumber);
        }
        return TELEPHONY_FAIL;
    }

    int32_t GetMainCallId(int32_t callId)
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->GetMainCallId(callId);
        }
        return TELEPHONY_FAIL;
    }

    std::vector<std::u16string> GetSubCallIdList(int32_t callId)
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->GetSubCallIdList(callId);
        }
        std::vector<std::u16string> vec;
        vec.clear();
        return vec;
    }

    std::vector<std::u16string> GetCallIdListForConference(int32_t callId)
    {
        if (callManagerServicePtr_ != nullptr) {
            return callManagerServicePtr_->GetCallIdListForConference(callId);
        }
        std::vector<std::u16string> vec;
        vec.clear();
        return vec;
    }

private:
    int32_t ConnectService()
    {
        Utils::UniqueWriteGuard<Utils::RWLock> guard(rwClientLock_);
        if (callManagerServicePtr_ != nullptr) {
            return TELEPHONY_SUCCESS;
        }
        sptr<ISystemAbilityManager> managerPtr =
            SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
        if (managerPtr == nullptr) {
            return TELEPHONY_FAIL;
        }
        sptr<ICallManagerService> callManagerServicePtr = nullptr;
        sptr<IRemoteObject> iRemoteObjectPtr = managerPtr->GetSystemAbility(systemAbilityId_);
        if (iRemoteObjectPtr == nullptr) {
            return TELEPHONY_CONNECT_SYSTEM_ABILITY_STUB_FAIL;
        }
        callManagerServicePtr = iface_cast<ICallManagerService>(iRemoteObjectPtr);
        if (!callManagerServicePtr) {
            return TELEPHONY_LOCAL_PTR_NULL;
        }
        callManagerServicePtr_ = callManagerServicePtr;
        int32_t ret = RegisterCallBack();
        if (ret != TELEPHONY_SUCCESS) {
            return ret;
        }
        return TELEPHONY_SUCCESS;
    }

    void DisconnectService()
    {
        Utils::UniqueWriteGuard<Utils::RWLock> guard(rwClientLock_);
        if (callManagerServicePtr_) {
            callManagerServicePtr_.clear();
            callManagerServicePtr_ = nullptr;
        }
        if (callAbilityCallbackPtr_) {
            callAbilityCallbackPtr_.clear();
            callAbilityCallbackPtr_ = nullptr;
        }
        TELEPHONY_LOGD("DisconnectService");
    }

    int32_t RegisterCallBack()
    {
        if (callManagerServicePtr_ == nullptr) {
            TELEPHONY_LOGE("callManagerServicePtr_ is null");
            return TELEPHONY_LOCAL_PTR_NULL;
        }
        callAbilityCallbackPtr_ = (std::make_unique<CallAbilityCallback>()).release();
        if (callAbilityCallbackPtr_ == nullptr) {
            DisconnectService();
            TELEPHONY_LOGE("create CallAbilityCallback object failed!");
            return TELEPHONY_FAIL;
        }

        int32_t ret = callManagerServicePtr_->RegisterCallBack(callAbilityCallbackPtr_);
        if (ret != TELEPHONY_SUCCESS) {
            DisconnectService();
            TELEPHONY_LOGE("register callback to call manager service failed,result: %{public}d", ret);
            return TELEPHONY_REGISTER_CALLBACK_FAIL;
        }
        TELEPHONY_LOGI("register call ability callback success!");
        return TELEPHONY_SUCCESS;
    }

private:
    int32_t systemAbilityId_;
    sptr<ICallManagerService> callManagerServicePtr_;
    sptr<ICallAbilityCallback> callAbilityCallbackPtr_;
    Utils::RWLock rwClientLock_;
};
} // namespace Telephony
} // namespace OHOS
#endif