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

#include "call_object_manager.h"

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
std::list<sptr<CallBase>> CallObjectManager::callObjectPtrList_;
std::mutex CallObjectManager::listMutex_;
int32_t CallObjectManager::callId_ = kCallStartId;
uint32_t CallObjectManager::maxCallCount_ = MAX_CALL_COUNT;

CallObjectManager::CallObjectManager()
{
    callObjectPtrList_.clear();
}

CallObjectManager::~CallObjectManager()
{
    std::list<sptr<CallBase>>::iterator it = callObjectPtrList_.begin();
    while (it != callObjectPtrList_.end()) {
        (*it) = nullptr;
        callObjectPtrList_.erase(it++);
    }
}

int32_t CallObjectManager::AddOneCallObject(sptr<CallBase> &call)
{
    if (call == nullptr) {
        return CALL_MANAGER_CALL_NULL;
    }
    std::lock_guard<std::mutex> lock(listMutex_);
    std::list<sptr<CallBase>>::iterator it = callObjectPtrList_.begin();
    for (; it != callObjectPtrList_.end(); it++) {
        if ((*it)->GetCallID() == call->GetCallID()) {
            TELEPHONY_LOGI("this call has existed yet!");
            return TELEPHONY_FAIL;
        }
    }
    callObjectPtrList_.emplace_back(call);
    TELEPHONY_LOGI("AddOneCallObject success! callId:%{public}d,call list size:%{public}zu", call->GetCallID(),
        callObjectPtrList_.size());
    return TELEPHONY_SUCCESS;
}

int32_t CallObjectManager::DeleteOneCallObject(int32_t callId)
{
    std::lock_guard<std::mutex> lock(listMutex_);
    std::list<sptr<CallBase>>::iterator it;
    for (it = callObjectPtrList_.begin(); it != callObjectPtrList_.end(); it++) {
        if ((*it)->GetCallID() == callId) {
            callObjectPtrList_.erase(it);
            TELEPHONY_LOGI("DeleteOneCallObject success! call list size:%{public}zu", callObjectPtrList_.size());
            break;
        }
    }
    return TELEPHONY_SUCCESS;
}

void CallObjectManager::DeleteOneCallObject(sptr<CallBase> &call)
{
    if (call == nullptr) {
        TELEPHONY_LOGE("call is null!");
        return;
    }
    std::lock_guard<std::mutex> lock(listMutex_);
    callObjectPtrList_.remove(call);
    TELEPHONY_LOGE("DeleteOneCallObject success! callList size:%{public}zu", callObjectPtrList_.size());
}

sptr<CallBase> CallObjectManager::GetOneCallObject(int32_t callId)
{
    sptr<CallBase> retPtr = nullptr;
    std::lock_guard<std::mutex> lock(listMutex_);
    std::list<sptr<CallBase>>::iterator it = CallObjectManager::callObjectPtrList_.begin();
    for (; it != callObjectPtrList_.end(); it++) {
        if ((*it)->GetCallID() == callId) {
            retPtr = *it;
            break;
        }
    }
    return retPtr;
}

sptr<CallBase> CallObjectManager::GetOneCallObject(std::string &phoneNumber)
{
    if (phoneNumber.empty()) {
        TELEPHONY_LOGE("call is null!");
        return nullptr;
    }
    sptr<CallBase> retPtr = nullptr;
    std::lock_guard<std::mutex> lock(listMutex_);
    std::list<sptr<CallBase>>::iterator it = callObjectPtrList_.begin();
    for (; it != callObjectPtrList_.end(); it++) {
        if ((*it)->GetAccountNumber() == phoneNumber) {
            TELEPHONY_LOGE("GetOneCallObject success!");
            retPtr = *it;
            break;
        }
    }
    return retPtr;
}

int32_t CallObjectManager::HasNewCall()
{
    std::lock_guard<std::mutex> lock(listMutex_);
    std::list<sptr<CallBase>>::iterator it;
    for (it = callObjectPtrList_.begin(); it != callObjectPtrList_.end(); it++) {
        if ((*it)->GetCallRunningState() == CallRunningState::CALL_RUNNING_STATE_CREATE ||
            (*it)->GetCallRunningState() == CallRunningState::CALL_RUNNING_STATE_CONNECTING ||
            (*it)->GetCallRunningState() == CallRunningState::CALL_RUNNING_STATE_DIALING) {
            TELEPHONY_LOGE("there is already a new call[callId:%d,state:%d], please redial later",
                (*it)->GetCallID(), (*it)->GetCallRunningState());
            return CALL_MANAGER_HAS_NEW_CALL;
        }
    }
    return TELEPHONY_SUCCESS;
}

bool CallObjectManager::IsNewCallAllowedCreate()
{
    bool ret = true;
    std::lock_guard<std::mutex> lock(listMutex_);
    if (callObjectPtrList_.size() > maxCallCount_) {
        TELEPHONY_LOGE("The number of calls exceeds the limit");
        return false;
    }
    std::list<sptr<CallBase>>::iterator it;
    for (it = callObjectPtrList_.begin(); it != callObjectPtrList_.end(); it++) {
        if ((*it)->GetCallRunningState() == CallRunningState::CALL_RUNNING_STATE_CREATE ||
            (*it)->GetCallRunningState() == CallRunningState::CALL_RUNNING_STATE_CONNECTING ||
            (*it)->GetCallRunningState() == CallRunningState::CALL_RUNNING_STATE_DIALING) {
            TELEPHONY_LOGE("there is already a new call, please redial later");
            ret = false;
            break;
        }
    }
    return ret;
}

int32_t CallObjectManager::GetActiveCallList(std::list<sptr<CallBase>> &list)
{
    list.clear();
    std::lock_guard<std::mutex> lock(listMutex_);
    std::list<sptr<CallBase>>::iterator it;
    for (it = callObjectPtrList_.begin(); it != callObjectPtrList_.end(); it++) {
        if ((*it)->GetCallRunningState() == CallRunningState::CALL_RUNNING_STATE_ACTIVE) {
            list.emplace_back(*it);
        }
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallObjectManager::GetHoldCallList(std::list<sptr<CallBase>> &list)
{
    list.clear();
    std::lock_guard<std::mutex> lock(listMutex_);
    std::list<sptr<CallBase>>::iterator it;
    for (it = callObjectPtrList_.begin(); it != callObjectPtrList_.end(); it++) {
        if ((*it)->GetCallRunningState() == CallRunningState::CALL_RUNNING_STATE_HOLD) {
            list.emplace_back(*it);
        }
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallObjectManager::GetCarrierCallList(std::list<sptr<CallBase>> &list)
{
    list.clear();
    std::lock_guard<std::mutex> lock(listMutex_);
    std::list<sptr<CallBase>>::iterator it;
    for (it = callObjectPtrList_.begin(); it != callObjectPtrList_.end(); it++) {
        if ((*it)->GetCallType() == CallType::TYPE_CS || (*it)->GetCallType() == CallType::TYPE_IMS) {
            list.emplace_back(*it);
        }
    }
    return TELEPHONY_SUCCESS;
}

bool CallObjectManager::HasRingingMaximum()
{
    int32_t ringingCount = 0;
    std::lock_guard<std::mutex> lock(listMutex_);
    std::list<sptr<CallBase>>::iterator it;
    for (it = callObjectPtrList_.begin(); it != callObjectPtrList_.end(); it++) {
        // Count the number of calls in the ringing state
        if ((*it)->GetCallRunningState() == CallRunningState::CALL_RUNNING_STATE_RINGING) {
            ringingCount++;
        }
    }
    if (ringingCount >= kMaxRingingCallNumberLen) {
        return true;
    }
    return false;
}

bool CallObjectManager::HasDialingMaximum()
{
    int32_t dialingCount = 0;
    std::lock_guard<std::mutex> lock(listMutex_);
    std::list<sptr<CallBase>>::iterator it;
    for (it = callObjectPtrList_.begin(); it != callObjectPtrList_.end(); it++) {
        // Count the number of calls in the active state
        if ((*it)->GetCallRunningState() == CallRunningState::CALL_RUNNING_STATE_ACTIVE) {
            dialingCount++;
        }
    }
    if (dialingCount >= kMaxDialingCallNumberLen) {
        return true;
    }
    return false;
}

bool CallObjectManager::HasEmergencyCall()
{
    std::lock_guard<std::mutex> lock(listMutex_);
    std::list<sptr<CallBase>>::iterator it;
    for (it = callObjectPtrList_.begin(); it != callObjectPtrList_.end(); it++) {
        if ((*it)->GetEmergencyState()) {
            return true;
        }
    }
    return false;
}

int32_t CallObjectManager::GetNewCallId()
{
    int32_t ret = 0;
    std::lock_guard<std::mutex> lock(listMutex_);
    ret = ++callId_;
    return ret;
}

bool CallObjectManager::IsCallExist(int32_t callId)
{
    std::lock_guard<std::mutex> lock(listMutex_);
    std::list<sptr<CallBase>>::iterator it = callObjectPtrList_.begin();
    for (; it != callObjectPtrList_.end(); it++) {
        if ((*it)->GetCallID() == callId) {
            return true;
        }
    }
    return false;
}

bool CallObjectManager::IsCallExist(std::string &phoneNumber)
{
    if (phoneNumber.empty()) {
        return CALL_MANAGER_PHONE_NUMBER_NULL;
    }
    std::lock_guard<std::mutex> lock(listMutex_);
    std::list<sptr<CallBase>>::iterator it = callObjectPtrList_.begin();
    for (; it != callObjectPtrList_.end(); it++) {
        if ((*it)->GetAccountNumber() == phoneNumber) {
            return true;
        }
    }
    return false;
}

bool CallObjectManager::HasCallExist()
{
    std::lock_guard<std::mutex> lock(listMutex_);
    if (callObjectPtrList_.empty()) {
        TELEPHONY_LOGI("call list size:%{public}zu", callObjectPtrList_.size());
        return false;
    }
    return true;
}

bool CallObjectManager::HasRingingCall()
{
    bool ret = false;
    std::lock_guard<std::mutex> lock(listMutex_);
    std::list<sptr<CallBase>>::iterator it;
    for (it = callObjectPtrList_.begin(); it != callObjectPtrList_.end(); it++) {
        // Count the number of calls in the ringing state
        if ((*it)->GetCallRunningState() == CallRunningState::CALL_RUNNING_STATE_RINGING) {
            ret = true;
            break;
        }
    }
    return ret;
}

TelCallState CallObjectManager::GetCallState(int32_t callId)
{
    TelCallState retState = CALL_STATUS_IDLE;
    std::lock_guard<std::mutex> lock(listMutex_);
    std::list<sptr<CallBase>>::iterator it = CallObjectManager::callObjectPtrList_.begin();
    for (; it != callObjectPtrList_.end(); it++) {
        if ((*it)->GetCallID() == callId) {
            retState = (*it)->GetTelCallState();
            break;
        }
    }
    return retState;
}

std::list<sptr<CallBase>> CallObjectManager::GetCallList()
{
    return callObjectPtrList_;
}

bool CallObjectManager::IsCallExist(CallType callType, TelCallState callState)
{
    std::lock_guard<std::mutex> lock(listMutex_);
    std::list<sptr<CallBase>>::iterator it;
    for (it = callObjectPtrList_.begin(); it != callObjectPtrList_.end(); it++) {
        if ((*it)->GetCallType() == callType && (*it)->GetTelCallState() == callState) {
            TELEPHONY_LOGD("the call is exist.");
            return true;
        }
    }
    TELEPHONY_LOGD("the call is does not exist.");
    return false;
}
} // namespace Telephony
} // namespace OHOS
