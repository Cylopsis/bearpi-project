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

#include "call_ability_report_ipc_proxy.h"

#include <string_ex.h>

#include "iservice_registry.h"
#include "system_ability.h"
#include "system_ability_definition.h"

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
CallAbilityReportIpcProxy::CallAbilityReportIpcProxy() : callAbilityCallbackPtr_(nullptr) {}

CallAbilityReportIpcProxy::~CallAbilityReportIpcProxy()
{
    if (callAbilityCallbackPtr_) {
        callAbilityCallbackPtr_.clear();
        callAbilityCallbackPtr_ = nullptr;
    }
}

int32_t CallAbilityReportIpcProxy::RegisterCallBack(sptr<ICallAbilityCallback> callAbilityCallbackPtr)
{
    if (callAbilityCallbackPtr == nullptr) {
        TELEPHONY_LOGE("callAbilityCallbackPtr is null");
        return TELEPHONY_LOCAL_PTR_NULL;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (callAbilityCallbackPtr_) {
        callAbilityCallbackPtr_.clear();
        callAbilityCallbackPtr_ = nullptr;
    }
    callAbilityCallbackPtr_ = callAbilityCallbackPtr;
    return TELEPHONY_SUCCESS;
}

int32_t CallAbilityReportIpcProxy::ReportCallStateInfo(const CallAttributeInfo &info)
{
    int ret = TELEPHONY_FAIL;
    std::lock_guard<std::mutex> lock(mutex_);
    if (callAbilityCallbackPtr_ == nullptr) {
        TELEPHONY_LOGE("call ability service haven't connected");
        return TELEPHONY_LOCAL_PTR_NULL;
    }

    ret = callAbilityCallbackPtr_->OnCallDetailsChange(info);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("OnCallDetailsChange failed, errcode:%{public}d", ret);
        return ret;
    }
    TELEPHONY_LOGD("report call state:%d", info.callState);
    return ret;
}

int32_t CallAbilityReportIpcProxy::ReportCallEvent(const CallEventInfo &info)
{
    int ret = TELEPHONY_FAIL;
    std::lock_guard<std::mutex> lock(mutex_);
    if (callAbilityCallbackPtr_ == nullptr) {
        TELEPHONY_LOGE("call ability service haven't connected");
        return TELEPHONY_LOCAL_PTR_NULL;
    }

    ret = callAbilityCallbackPtr_->OnCallEventChange(info);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("OnCallEventChange failed, errcode:%{public}d", ret);
        return ret;
    }
    TELEPHONY_LOGD("report call event, eventId:%{public}d", info.eventId);
    return ret;
}

int32_t CallAbilityReportIpcProxy::ReportSupplementResult(
    const CallResultReportId reportId, AppExecFwk::PacMap &resultInfo)
{
    int ret = TELEPHONY_FAIL;
    std::lock_guard<std::mutex> lock(mutex_);
    if (callAbilityCallbackPtr_ == nullptr) {
        TELEPHONY_LOGE("call ability service haven't connected");
        return TELEPHONY_LOCAL_PTR_NULL;
    }

    ret = callAbilityCallbackPtr_->OnSupplementResult(reportId, resultInfo);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("OnSupplementResult failed, errcode:%{public}d", ret);
        return ret;
    }
    return ret;
}
} // namespace Telephony
} // namespace OHOS