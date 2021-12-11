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

#ifndef NAPI_CALL_ABILITY_CALLBACK_H
#define NAPI_CALL_ABILITY_CALLBACK_H

#include <uv.h>

#include "pac_map.h"
#include "singleton.h"

#include "napi_call_manager_types.h"

namespace OHOS {
namespace Telephony {
class NapiCallAbilityCallback {
    DECLARE_DELAYED_SINGLETON(NapiCallAbilityCallback)
public:
    void RegisterCallStateCallback(EventListener stateCallback);
    void UnRegisterCallStateCallback();
    void RegisterCallEventCallback(EventListener eventCallback);
    void UnRegisterCallEventCallback();
    int32_t RegisterGetWaitingCallback(EventListener callback);
    void UnRegisterGetWaitingCallback();
    int32_t RegisterSetWaitingCallback(EventListener callback);
    void UnRegisterSetWaitingCallback();
    int32_t RegisterGetRestrictionCallback(EventListener callback);
    void UnRegisterGetRestrictionCallback();
    int32_t RegisterSetRestrictionCallback(EventListener callback);
    void UnRegisterSetRestrictionCallback();
    int32_t RegisterGetTransferCallback(EventListener callback);
    void UnRegisterGetTransferCallback();
    int32_t RegisterSetTransferCallback(EventListener callback);
    void UnRegisterSetTransferCallback();
    int32_t UpdateCallStateInfo(const CallAttributeInfo &info);
    int32_t UpdateCallEvent(const CallEventInfo &info);
    int32_t UpdateSupplementInfo(const CallResultReportId reportId, AppExecFwk::PacMap &resultInfo);

private:
    static void ReportCallStateWork(uv_work_t *work, int status);
    static int32_t ReportCallState(CallAttributeInfo &info, EventListener stateCallback);
    int32_t ReportGetWaitingInfo(AppExecFwk::PacMap &resultInfo);
    int32_t ReportSetWaitingInfo(AppExecFwk::PacMap &resultInfo);
    int32_t ReportGetRestrictionInfo(AppExecFwk::PacMap &resultInfo);
    int32_t ReportSetRestrictionInfo(AppExecFwk::PacMap &resultInfo);
    int32_t ReportGetTransferInfo(AppExecFwk::PacMap &resultInfo);
    int32_t ReportSetTransferInfo(AppExecFwk::PacMap &resultInfo);
    void ReportWaitAndLimitInfo(napi_env env, AppExecFwk::PacMap &resultInfo, EventListener supplementInfo);
    void ReportSupplementInfo(
        napi_env env, int32_t result, EventListener supplementInfo, napi_value &callbackValue);
    int32_t ReportSettingInfo(EventListener &settingInfo, AppExecFwk::PacMap &resultInfo);
    static napi_value CreateUndefined(napi_env env);
    napi_value CreateErrorMessage(napi_env env, std::string msg);
    static void SetPropertyInt32(napi_env env, napi_value object, std::string name, int32_t value);
    static void SetPropertyStringUtf8(napi_env env, napi_value object, std::string name, std::string value);

private:
    EventListener stateCallback_;
    EventListener eventCallback_;
    EventListener getWaitingCallback_;
    EventListener setWaitingCallback_;
    EventListener getRestrictionCallback_;
    EventListener setRestrictionCallback_;
    EventListener getTransferCallback_;
    EventListener setTransferCallback_;
    std::mutex mutex_;
};
} // namespace Telephony
} // namespace OHOS

#endif // NAPI_CALL_ABILITY_CALLBACK_H