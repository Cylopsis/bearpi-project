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

#include "cs_conference_base.h"

#include <string>
#include <string_ex.h>

#include "call_manager_errors.h"
#include "call_manager_inner_type.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
constexpr uint32_t CS_CONFERENCE_MIN_CALLS_CNT = 2;
constexpr uint32_t CS_CONFERENCE_MAX_CALLS_CNT = 5;
int32_t CsConferenceBase::mainCallId_ = ERR_ID;
ConferenceState CsConferenceBase::state_ = CONFERENCE_STATE_IDLE;
std::vector<int32_t> CsConferenceBase::subCallIdVec_;
std::mutex CsConferenceBase::vecMutex_;

CsConferenceBase::CsConferenceBase()
{
    subCallIdVec_.clear();
}

CsConferenceBase::~CsConferenceBase() {}

int32_t CsConferenceBase::SetMainCall(int32_t callId)
{
    if (callId <= ERR_ID) {
        TELEPHONY_LOGE("callId is invalid:%{public}d", callId);
        return CALL_MANAGER_CALLID_INVALID;
    }
    std::lock_guard<std::mutex> lock(vecMutex_);
    if (subCallIdVec_.size() >= CS_CONFERENCE_MAX_CALLS_CNT) {
        TELEPHONY_LOGE("there is %{public}zu calls in the conference yet!", subCallIdVec_.size());
        return CALL_MANAGER_CONFERENCE_CALL_EXCEED_LIMIT;
    }
    if (mainCallId_ == ERR_ID) {
        mainCallId_ = callId;
        state_ = CONFERENCE_STATE_CREATING;
    } else {
        if (mainCallId_ != callId) {
            TELEPHONY_LOGE(
                "there is already an mainCallId:%{public}d, your input callId:%{public}d", mainCallId_, callId);
            return CALL_MANAGER_CALLID_INVALID;
        }
    }
    return TELEPHONY_SUCCESS;
}

int32_t CsConferenceBase::AddOneConferenceSubCallId(int32_t callId)
{
    std::lock_guard<std::mutex> lock(vecMutex_);
    if (subCallIdVec_.size() > CS_CONFERENCE_MAX_CALLS_CNT) {
        TELEPHONY_LOGE("the number of calls in the conference exceeds the limit");
        return TELEPHONY_ERROR;
    }
    if (mainCallId_ == callId) {
        TELEPHONY_LOGE("this is mainCallId:%{public}d", callId);
        return TELEPHONY_SUCCESS;
    }
    std::vector<int32_t>::iterator it = subCallIdVec_.begin();
    for (; it != subCallIdVec_.end(); it++) {
        if (*it == callId) {
            TELEPHONY_LOGE("this callId:%{public}d is exist yet!", callId);
            return TELEPHONY_ERROR;
        }
    }
    subCallIdVec_.push_back(callId);
    state_ = CONFERENCE_STATE_ACTIVE;
    return TELEPHONY_SUCCESS;
}

int32_t CsConferenceBase::GetMainCsCallId(int32_t callId)
{
    std::lock_guard<std::mutex> lock(vecMutex_);
    return mainCallId_;
}

std::vector<std::u16string> CsConferenceBase::GetSubCsCallIdList(int32_t callId)
{
    std::string tmpStr = "";
    std::vector<std::u16string> vec;
    vec.clear();
    std::lock_guard<std::mutex> lock(vecMutex_);
    if (mainCallId_ != callId) {
        TELEPHONY_LOGE("err callId, mainCallId:%{public}d, your input callId:%{public}d", mainCallId_, callId);
        return vec;
    }
    std::vector<int32_t>::iterator it = subCallIdVec_.begin();
    for (; it != subCallIdVec_.end(); it++) {
        tmpStr = std::to_string(*it);
        vec.push_back(Str8ToStr16(tmpStr));
    }
    return vec;
}

std::vector<std::u16string> CsConferenceBase::GetCsCallIdListForConference(int32_t callId)
{
    std::string tmpStr = "";
    std::vector<std::u16string> vec;
    vec.clear();
    std::lock_guard<std::mutex> lock(vecMutex_);
    if (mainCallId_ != callId) {
        TELEPHONY_LOGE("err callId, mainCallId:%{public}d, your input callId:%{public}d", mainCallId_, callId);
        return vec;
    }
    std::vector<int32_t>::iterator it = subCallIdVec_.begin();
    for (; it != subCallIdVec_.end(); it++) {
        tmpStr = std::to_string(*it);
        vec.push_back(Str8ToStr16(tmpStr));
    }
    tmpStr = std::to_string(mainCallId_);
    vec.push_back(Str8ToStr16(tmpStr));
    return vec;
}

ConferenceState CsConferenceBase::GetCsConferenceState()
{
    std::lock_guard<std::mutex> lock(vecMutex_);
    return state_;
}

void CsConferenceBase::SetCsConferenceState(ConferenceState state)
{
    std::lock_guard<std::mutex> lock(vecMutex_);
    state_ = state;
}

int32_t CsConferenceBase::SeparateConference(int32_t callId)
{
    std::lock_guard<std::mutex> lock(vecMutex_);
    std::vector<int32_t>::iterator it = subCallIdVec_.begin();
    for (; it != subCallIdVec_.end(); it++) {
        if (*it == callId) {
            subCallIdVec_.erase(it);
            break;
        }
    }
    if (subCallIdVec_.size() < CS_CONFERENCE_MIN_CALLS_CNT) {
        state_ = CONFERENCE_STATE_IDLE;
        mainCallId_ = ERR_ID;
        subCallIdVec_.clear();
    }
    return TELEPHONY_SUCCESS;
}

int32_t CsConferenceBase::CanCombineCsConference()
{
    std::lock_guard<std::mutex> lock(vecMutex_);
    if (subCallIdVec_.size() >= CS_CONFERENCE_MAX_CALLS_CNT) {
        TELEPHONY_LOGE("there is %{public}zu calls in the conference yet!", subCallIdVec_.size());
        return CALL_MANAGER_CONFERENCE_CALL_EXCEED_LIMIT;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CsConferenceBase::SubCallCombineToCsConference(int32_t callId)
{
    bool combineStatus = false;
    {
        std::lock_guard<std::mutex> lock(vecMutex_);
        if (state_ == CONFERENCE_STATE_CREATING || state_ == CONFERENCE_STATE_ACTIVE ||
            state_ == CONFERENCE_STATE_LEAVING) {
            combineStatus = true;
        }
    }
    if (combineStatus) {
        return AddOneConferenceSubCallId(callId);
    }
    return TELEPHONY_ERROR;
}

int32_t CsConferenceBase::SubCallSeparateFromCsConference(int32_t callId)
{
    bool separateStatus = false;
    {
        std::lock_guard<std::mutex> lock(vecMutex_);
        if (state_ == CONFERENCE_STATE_ACTIVE || state_ == CONFERENCE_STATE_LEAVING) {
            separateStatus = true;
        }
    }
    if (separateStatus) {
        return SeparateConference(callId);
    }
    return TELEPHONY_ERROR;
}

int32_t CsConferenceBase::CanSeparateCsConference()
{
    std::lock_guard<std::mutex> lock(vecMutex_);
    if (subCallIdVec_.empty()) {
        TELEPHONY_LOGE("no call is currently in the conference!");
        return CALL_MANAGER_CONFERENCE_NOT_EXISTS;
    }
    return TELEPHONY_SUCCESS;
}
} // namespace Telephony
} // namespace OHOS
