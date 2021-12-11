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

#include "call_policy.h"

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

#include "call_number_utils.h"

namespace OHOS {
namespace Telephony {
CallPolicy::CallPolicy() {}

CallPolicy::~CallPolicy() {}

int32_t CallPolicy::DialPolicy(int32_t slotId)
{
    if (!DelayedSingleton<CallNumberUtils>::GetInstance()->IsValidSlotId(slotId)) {
        return ERR_INVALID_SLOT_ID;
    }
    return HasNewCall();
}

int32_t CallPolicy::AnswerCallPolicy(int32_t callId)
{
    if (!IsCallExist(callId)) {
        TELEPHONY_LOGE("callId is invalid, callId:%{public}d", callId);
        return CALL_MANAGER_CALLID_INVALID;
    }
    TelCallState state = GetCallState(callId);
    if (state != CALL_STATUS_INCOMING && state != CALL_STATUS_WAITING) {
        TELEPHONY_LOGE("current call state is:%{public}d, accept call not allowed", state);
        return CALL_MANAGER_ILLEGAL_CALL_OPERATION;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallPolicy::RejectCallPolicy(int32_t callId)
{
    if (!IsCallExist(callId)) {
        TELEPHONY_LOGE("callId is invalid, callId:%{public}d", callId);
        return CALL_MANAGER_CALLID_INVALID;
    }
    TelCallState state = GetCallState(callId);
    if (state != CALL_STATUS_INCOMING && state != CALL_STATUS_WAITING) {
        TELEPHONY_LOGE("current call state is:%{public}d, reject call not allowed", state);
        return CALL_MANAGER_ILLEGAL_CALL_OPERATION;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallPolicy::HoldCallPolicy(int32_t callId)
{
    sptr<CallBase> call = GetOneCallObject(callId);
    if (call == nullptr) {
        TELEPHONY_LOGE("GetOneCallObject failed, this callId is invalid! callId:%{public}d", callId);
        return CALL_MANAGER_CALLID_INVALID;
    }
    if (call->GetCallRunningState() != CallRunningState::CALL_RUNNING_STATE_ACTIVE) {
        TELEPHONY_LOGE("this call is not activated! callId:%{public}d", callId);
        return CALL_MANAGER_CALL_IS_NOT_ACTIVATED;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallPolicy::UnHoldCallPolicy(int32_t callId)
{
    sptr<CallBase> call = GetOneCallObject(callId);
    if (call == nullptr) {
        TELEPHONY_LOGE("GetOneCallObject failed, this callId is invalid! callId:%{public}d", callId);
        return CALL_MANAGER_CALLID_INVALID;
    }
    if (call->GetCallRunningState() != CallRunningState::CALL_RUNNING_STATE_HOLD) {
        TELEPHONY_LOGE("this call is not on holding state! callId:%{public}d", callId);
        return CALL_MANAGER_CALL_IS_NOT_ON_HOLDING;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallPolicy::HangUpPolicy(int32_t callId)
{
    if (!IsCallExist(callId)) {
        TELEPHONY_LOGE("callId is invalid, callId:%{public}d", callId);
        return CALL_MANAGER_CALLID_INVALID;
    }
    TelCallState state = GetCallState(callId);
    if (state == CALL_STATUS_IDLE || state == CALL_STATUS_DISCONNECTING || state == CALL_STATUS_DISCONNECTED) {
        TELEPHONY_LOGE("current call state is:%{public}d, hang up call not allowed", state);
        return CALL_MANAGER_ILLEGAL_CALL_OPERATION;
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallPolicy::SwitchCallPolicy(int32_t &callId)
{
    std::list<sptr<CallBase>> callList;
    GetCarrierCallList(callList);
    if (callList.size() < onlyTwoCall_) {
        callList.clear();
        return CALL_MANAGER_ILLEGAL_CALL_OPERATION;
    }
    callId = callList.front()->GetCallID();
    callList.clear();
    return TELEPHONY_SUCCESS;
}

int32_t CallPolicy::JoinCallPolicy()
{
    return TELEPHONY_SUCCESS;
}

int32_t CallPolicy::GetTransferNumberPolicy()
{
    return TELEPHONY_SUCCESS;
}

int32_t CallPolicy::SetTransferNumberPolicy()
{
    return TELEPHONY_SUCCESS;
}

int32_t CallPolicy::StartConferencePolicy()
{
    return TELEPHONY_SUCCESS;
}

int32_t CallPolicy::InviteToConferencePolicy()
{
    return TELEPHONY_SUCCESS;
}

int32_t CallPolicy::KickOutConferencePolicy()
{
    return TELEPHONY_SUCCESS;
}

int32_t CallPolicy::LeaveConferencePolicy()
{
    return TELEPHONY_SUCCESS;
}
} // namespace Telephony
} // namespace OHOS
