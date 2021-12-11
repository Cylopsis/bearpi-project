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

#include "call_ability_callback.h"

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

#include "napi_call_ability_callback.h"
#include "napi_call_ability_handler.h"

namespace OHOS {
namespace Telephony {
CallAbilityCallback::CallAbilityCallback() {}

CallAbilityCallback::~CallAbilityCallback() {}

int32_t CallAbilityCallback::OnCallDetailsChange(const CallAttributeInfo &info)
{
    TELEPHONY_LOGE("start");
    int32_t ret = DelayedSingleton<NapiCallAbilityHandlerService>::GetInstance()->UpdateCallStateInfoHandler(info);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("UpdateCallStateInfoHandler failed! errCode:%{public}d", ret);
    } else {
        TELEPHONY_LOGI("UpdateCallStateInfoHandler success! state:%d", info.callState);
    }
    TELEPHONY_LOGE("end");
    return ret;
}

int32_t CallAbilityCallback::OnCallEventChange(const CallEventInfo &info)
{
    int32_t ret = DelayedSingleton<NapiCallAbilityHandlerService>::GetInstance()->UpdateCallEventHandler(info);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("UpdateCallEventHandler failed! errCode:%{public}d", ret);
    } else {
        TELEPHONY_LOGI("UpdateCallEventHandler success!");
    }
    return ret;
}

int32_t CallAbilityCallback::OnSupplementResult(CallResultReportId reportId, AppExecFwk::PacMap &resultInfo)
{
    int32_t ret =
        DelayedSingleton<NapiCallAbilityCallback>::GetInstance()->UpdateSupplementInfo(reportId, resultInfo);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("OnSupplementResult failed! errCode:%{public}d", ret);
    } else {
        TELEPHONY_LOGI("OnSupplementResult success!");
    }
    return TELEPHONY_SUCCESS;
}
} // namespace Telephony
} // namespace OHOS