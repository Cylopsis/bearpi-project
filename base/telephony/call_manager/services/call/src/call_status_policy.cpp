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

#include "call_status_policy.h"

#include <securec.h>

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

#include "call_control_manager.h"

namespace OHOS {
namespace Telephony {
CallStatusPolicy::CallStatusPolicy() {}

CallStatusPolicy::~CallStatusPolicy() {}

int32_t CallStatusPolicy::IncomingHandlePolicy(ContactInfo info)
{
    int32_t ret = TELEPHONY_FAIL;
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("VoicemailInterceptor failed!");
    } else {
        TELEPHONY_LOGI("VoicemailInterceptor success!");
    }
    return ret;
}

int32_t CallStatusPolicy::DialingHandlePolicy()
{
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusPolicy::ActiveHandlePolicy()
{
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusPolicy::HoldingHandlePolicy()
{
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusPolicy::WaitingHandlePolicy()
{
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusPolicy::AlertHandlePolicy()
{
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusPolicy::DisconnectingHandlePolicy()
{
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusPolicy::DisconnectedHandlePolicy()
{
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusPolicy::IdleHandlePolicy()
{
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusPolicy::FilterResultsDispose(sptr<CallBase> call)
{
    int32_t ret = TELEPHONY_FAIL;
    if (call == nullptr) {
        return CALL_MANAGER_CALL_NULL;
    }
    if (HasRingingMaximum() || HasDialingMaximum()) {
        TELEPHONY_LOGI("the number of Ringing or Dialing call is bigger than Maximum, start to hung up");
        ret = call->HangUpCall();
        if (ret != TELEPHONY_SUCCESS) {
            TELEPHONY_LOGE("HangUpCall failed!");
            return ret;
        }
        // Print the log and record the information to the database
        return TELEPHONY_SUCCESS;
    }
    ret = call->IncomingCallBase();
    DelayedSingleton<CallControlManager>::GetInstance()->NotifyNewCallCreated(call);
    // Print the log and record the information to the database
    // Call notification module
    return TELEPHONY_SUCCESS;
}
} // namespace Telephony
} // namespace OHOS