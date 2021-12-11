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

#include "napi_call_ability_callback.h"

#include <securec.h>
#include <memory>

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
NapiCallAbilityCallback::NapiCallAbilityCallback()
{
    (void)memset_s(&stateCallback_, sizeof(EventListener), 0, sizeof(EventListener));
    (void)memset_s(&eventCallback_, sizeof(EventListener), 0, sizeof(EventListener));
    (void)memset_s(&getWaitingCallback_, sizeof(EventListener), 0, sizeof(EventListener));
    (void)memset_s(&setWaitingCallback_, sizeof(EventListener), 0, sizeof(EventListener));
    (void)memset_s(&getRestrictionCallback_, sizeof(EventListener), 0, sizeof(EventListener));
    (void)memset_s(&setRestrictionCallback_, sizeof(EventListener), 0, sizeof(EventListener));
    (void)memset_s(&getTransferCallback_, sizeof(EventListener), 0, sizeof(EventListener));
    (void)memset_s(&setTransferCallback_, sizeof(EventListener), 0, sizeof(EventListener));
}

NapiCallAbilityCallback::~NapiCallAbilityCallback() {}

void NapiCallAbilityCallback::RegisterCallStateCallback(EventListener stateCallback)
{
    stateCallback_ = stateCallback;
}

void NapiCallAbilityCallback::UnRegisterCallStateCallback()
{
    if (stateCallback_.callbackRef) {
        (void)memset_s(&stateCallback_, sizeof(EventListener), 0, sizeof(EventListener));
    }
}

void NapiCallAbilityCallback::RegisterCallEventCallback(EventListener eventCallback)
{
    eventCallback_ = eventCallback;
}

void NapiCallAbilityCallback::UnRegisterCallEventCallback()
{
    if (eventCallback_.callbackRef) {
        (void)memset_s(&eventCallback_, sizeof(EventListener), 0, sizeof(EventListener));
    }
}

int32_t NapiCallAbilityCallback::RegisterGetWaitingCallback(EventListener callback)
{
    if (getWaitingCallback_.thisVar) {
        return CALL_MANAGER_HAS_CALLBACK;
    }
    getWaitingCallback_ = callback;
    return TELEPHONY_SUCCESS;
}

int32_t NapiCallAbilityCallback::RegisterSetWaitingCallback(EventListener callback)
{
    if (setWaitingCallback_.thisVar) {
        return CALL_MANAGER_HAS_CALLBACK;
    }
    setWaitingCallback_ = callback;
    return TELEPHONY_SUCCESS;
}

int32_t NapiCallAbilityCallback::RegisterGetRestrictionCallback(EventListener callback)
{
    if (getRestrictionCallback_.thisVar) {
        return CALL_MANAGER_HAS_CALLBACK;
    }
    getRestrictionCallback_ = callback;
    return TELEPHONY_SUCCESS;
}

int32_t NapiCallAbilityCallback::RegisterSetRestrictionCallback(EventListener callback)
{
    if (setRestrictionCallback_.thisVar) {
        return CALL_MANAGER_HAS_CALLBACK;
    }
    setRestrictionCallback_ = callback;
    return TELEPHONY_SUCCESS;
}

int32_t NapiCallAbilityCallback::RegisterGetTransferCallback(EventListener callback)
{
    if (getTransferCallback_.thisVar) {
        return CALL_MANAGER_HAS_CALLBACK;
    }
    getTransferCallback_ = callback;
    return TELEPHONY_SUCCESS;
}

int32_t NapiCallAbilityCallback::RegisterSetTransferCallback(EventListener callback)
{
    if (setTransferCallback_.thisVar) {
        return CALL_MANAGER_HAS_CALLBACK;
    }
    setTransferCallback_ = callback;
    return TELEPHONY_SUCCESS;
}

int32_t NapiCallAbilityCallback::UpdateCallStateInfo(const CallAttributeInfo &info)
{
    if (stateCallback_.thisVar == nullptr) {
        TELEPHONY_LOGE("stateCallback is null!");
        return TELEPHONY_FAIL;
    }
    uv_loop_s *loop = nullptr;
#if NAPI_VERSION >= 2
    napi_get_uv_event_loop(stateCallback_.env, &loop);
#endif
    ReceiveDataWorker *dataWorker = (std::make_unique<ReceiveDataWorker>()).release();
    if (!dataWorker) {
        return TELEPHONY_FAIL;
    }
    dataWorker->env = stateCallback_.env;
    dataWorker->ref = stateCallback_.callbackRef;
    dataWorker->info = info;
    dataWorker->callback = stateCallback_;
    uv_work_t *work = (std::make_unique<uv_work_t>()).release();
    if (!work) {
        return TELEPHONY_FAIL;
    }
    work->data = (void *)dataWorker;
    uv_queue_work(
        loop, work, [](uv_work_t *work) {}, ReportCallStateWork);
    return TELEPHONY_SUCCESS;
}

void NapiCallAbilityCallback::ReportCallStateWork(uv_work_t *work, int status)
{
    if (work == nullptr) {
        return;
    }
    ReceiveDataWorker *dataWorkerData = (ReceiveDataWorker *)work->data;
    if (dataWorkerData == nullptr) {
        return;
    }
    int32_t ret = ReportCallState(dataWorkerData->info, dataWorkerData->callback);
    TELEPHONY_LOGE("%{public}d", ret);
    delete dataWorkerData;
    dataWorkerData = nullptr;
    delete work;
    work = nullptr;
}

int32_t NapiCallAbilityCallback::ReportCallState(CallAttributeInfo &info, EventListener stateCallback)
{
    napi_value callbackFunc = nullptr;
    napi_env env = stateCallback.env;
    napi_value callbackValues[kArrayIndexThird] = {0};
    callbackValues[kArrayIndexFirst] = CreateUndefined(env);
    napi_create_object(env, &callbackValues[kArrayIndexSecond]);
    SetPropertyStringUtf8(env, callbackValues[kArrayIndexSecond], "accountNumber", info.accountNumber);
    SetPropertyInt32(env, callbackValues[kArrayIndexSecond], "accountId", info.accountId);
    SetPropertyInt32(env, callbackValues[kArrayIndexSecond], "videoState", info.videoState);
    SetPropertyInt32(env, callbackValues[kArrayIndexSecond], "callType", info.callType);
    SetPropertyInt32(env, callbackValues[kArrayIndexSecond], "callId", info.callId);
    SetPropertyInt32(env, callbackValues[kArrayIndexSecond], "callState", info.callState);
    SetPropertyInt32(env, callbackValues[kArrayIndexSecond], "conferenceState", info.conferenceState);
    napi_get_reference_value(env, stateCallback.callbackRef, &callbackFunc);
    napi_value callbackResult = nullptr;
    if (callbackFunc == nullptr) {
        TELEPHONY_LOGE("callbackFunc is null!");
        return TELEPHONY_FAIL;
    }
    napi_call_function(env, stateCallback.thisVar, callbackFunc, kDataLengthTwo, callbackValues, &callbackResult);
    return TELEPHONY_SUCCESS;
}

int32_t NapiCallAbilityCallback::UpdateCallEvent(const CallEventInfo &info)
{
    if (eventCallback_.thisVar == nullptr) {
        TELEPHONY_LOGE("eventCallback is null!");
        return TELEPHONY_FAIL;
    }
    napi_handle_scope scope = nullptr;
    napi_value callbackFunc = nullptr;
    napi_env env = eventCallback_.env;
    napi_open_handle_scope(env, &scope);
    napi_value callbackValues[kArrayIndexThird] = {0};
    {
        std::unique_lock<std::mutex> lock(mutex_);
        callbackValues[kArrayIndexFirst] = CreateUndefined(env);
        napi_create_object(env, &callbackValues[kArrayIndexSecond]);
        SetPropertyInt32(env, callbackValues[kArrayIndexSecond], "eventId", info.eventId);
        napi_get_reference_value(env, eventCallback_.callbackRef, &callbackFunc);
    }
    napi_value callbackResult = nullptr;
    if (callbackFunc == nullptr) {
        TELEPHONY_LOGE("callbackFunc is null!");
        return TELEPHONY_FAIL;
    }
    napi_call_function(env, eventCallback_.thisVar, callbackFunc, kDataLengthTwo, callbackValues, &callbackResult);
    napi_close_handle_scope(env, scope);
    return TELEPHONY_SUCCESS;
}

int32_t NapiCallAbilityCallback::UpdateSupplementInfo(
    const CallResultReportId reportId, AppExecFwk::PacMap &resultInfo)
{
    switch (reportId) {
        case CallResultReportId::GET_CALL_WAITING_REPORT_ID:
            ReportGetWaitingInfo(resultInfo);
            break;
        case CallResultReportId::SET_CALL_WAITING_REPORT_ID:
            ReportSetWaitingInfo(resultInfo);
            break;
        case CallResultReportId::GET_CALL_RESTRICTION_REPORT_ID:
            ReportGetRestrictionInfo(resultInfo);
            break;
        case CallResultReportId::SET_CALL_RESTRICTION_REPORT_ID:
            ReportSetRestrictionInfo(resultInfo);
            break;
        case CallResultReportId::GET_CALL_TRANSFER_REPORT_ID:
            ReportGetTransferInfo(resultInfo);
            break;
        case CallResultReportId::SET_CALL_TRANSFER_REPORT_ID:
            ReportSetTransferInfo(resultInfo);
            break;
        default:
            break;
    }
    return TELEPHONY_SUCCESS;
}

int32_t NapiCallAbilityCallback::ReportGetWaitingInfo(AppExecFwk::PacMap &resultInfo)
{
    if (getWaitingCallback_.thisVar == nullptr) {
        TELEPHONY_LOGE("getWaitingCallback is null!");
        return TELEPHONY_FAIL;
    }
    napi_handle_scope scope = nullptr;
    napi_env env = getWaitingCallback_.env;
    napi_open_handle_scope(env, &scope);
    ReportWaitAndLimitInfo(env, resultInfo, getWaitingCallback_);
    if (getWaitingCallback_.thisVar) {
        (void)memset_s(&getWaitingCallback_, sizeof(EventListener), 0, sizeof(EventListener));
    }
    napi_close_handle_scope(env, scope);
    return TELEPHONY_SUCCESS;
}

int32_t NapiCallAbilityCallback::ReportSetWaitingInfo(AppExecFwk::PacMap &resultInfo)
{
    if (setWaitingCallback_.thisVar == nullptr) {
        TELEPHONY_LOGE("setWaitingCallback is null!");
        return TELEPHONY_FAIL;
    }
    return ReportSettingInfo(setWaitingCallback_, resultInfo);
}

int32_t NapiCallAbilityCallback::ReportGetRestrictionInfo(AppExecFwk::PacMap &resultInfo)
{
    if (getRestrictionCallback_.thisVar == nullptr) {
        TELEPHONY_LOGE("getRestrictionCallback is null!");
        return TELEPHONY_FAIL;
    }
    napi_handle_scope scope = nullptr;
    napi_env env = getRestrictionCallback_.env;
    napi_open_handle_scope(env, &scope);
    ReportWaitAndLimitInfo(env, resultInfo, getRestrictionCallback_);
    if (getRestrictionCallback_.thisVar) {
        (void)memset_s(&getRestrictionCallback_, sizeof(EventListener), 0, sizeof(EventListener));
    }
    napi_close_handle_scope(env, scope);
    return TELEPHONY_SUCCESS;
}

int32_t NapiCallAbilityCallback::ReportSetRestrictionInfo(AppExecFwk::PacMap &resultInfo)
{
    if (setRestrictionCallback_.thisVar == nullptr) {
        TELEPHONY_LOGE("setRestrictionCallback is null!");
        return TELEPHONY_FAIL;
    }
    return ReportSettingInfo(setRestrictionCallback_, resultInfo);
}

int32_t NapiCallAbilityCallback::ReportGetTransferInfo(AppExecFwk::PacMap &resultInfo)
{
    if (getTransferCallback_.thisVar == nullptr) {
        TELEPHONY_LOGE("getTransferCallback is null!");
        return TELEPHONY_FAIL;
    }
    napi_handle_scope scope = nullptr;
    napi_env env = getTransferCallback_.env;
    napi_open_handle_scope(env, &scope);

    napi_value callbackValue = nullptr;
    napi_create_object(env, &callbackValue);
    SetPropertyInt32(env, callbackValue, "status", resultInfo.GetIntValue("status"));
    SetPropertyStringUtf8(env, callbackValue, "number", resultInfo.GetStringValue("number"));
    SetPropertyInt32(env, callbackValue, "type", resultInfo.GetIntValue("type"));
    int32_t result = resultInfo.GetIntValue("result");

    ReportSupplementInfo(env, result, getRestrictionCallback_, callbackValue);
    if (getTransferCallback_.thisVar) {
        (void)memset_s(&getTransferCallback_, sizeof(EventListener), 0, sizeof(EventListener));
    }
    napi_close_handle_scope(env, scope);
    return TELEPHONY_SUCCESS;
}

int32_t NapiCallAbilityCallback::ReportSetTransferInfo(AppExecFwk::PacMap &resultInfo)
{
    if (setTransferCallback_.thisVar == nullptr) {
        TELEPHONY_LOGE("setTransferCallback is null!");
        return TELEPHONY_FAIL;
    }
    return ReportSettingInfo(setTransferCallback_, resultInfo);
}

void NapiCallAbilityCallback::ReportWaitAndLimitInfo(
    napi_env env, AppExecFwk::PacMap &resultInfo, EventListener supplementInfo)
{
    int32_t result = resultInfo.GetIntValue("result");
    int32_t status = resultInfo.GetIntValue("status");

    if (supplementInfo.callbackRef != nullptr) {
        napi_value callbackValues[kArrayIndexThird] = {0};
        if (result == TELEPHONY_SUCCESS) {
            callbackValues[kArrayIndexFirst] = CreateUndefined(env);
            napi_create_int32(env, status, &callbackValues[kArrayIndexSecond]);
        } else {
            std::string errTip = std::to_string(result);
            callbackValues[kArrayIndexFirst] = CreateErrorMessage(env, errTip);
            callbackValues[kArrayIndexSecond] = CreateUndefined(env);
        }
        napi_value callbackFunc = nullptr;
        napi_get_reference_value(env, supplementInfo.callbackRef, &callbackFunc);
        napi_value callbackResult = nullptr;
        napi_call_function(
            env, supplementInfo.thisVar, callbackFunc, kDataLengthTwo, callbackValues, &callbackResult);
        napi_delete_reference(env, supplementInfo.callbackRef);
    } else if (supplementInfo.deferred != nullptr) {
        if (result == TELEPHONY_SUCCESS) {
            napi_value promiseValue = nullptr;
            napi_create_int32(env, status, &promiseValue);
            napi_resolve_deferred(env, supplementInfo.deferred, promiseValue);
        } else {
            std::string errTip = std::to_string(result);
            napi_reject_deferred(env, supplementInfo.deferred, CreateErrorMessage(env, errTip));
        }
    }
}

void NapiCallAbilityCallback::ReportSupplementInfo(
    napi_env env, int32_t result, EventListener supplementInfo, napi_value &callbackValue)
{
    if (supplementInfo.callbackRef != nullptr) {
        TELEPHONY_LOGE("supplementInfo.callbackRef  on ");
        napi_value callbackValues[kArrayIndexThird] = {0};
        if (result == TELEPHONY_SUCCESS) {
            callbackValues[kArrayIndexFirst] = CreateUndefined(env);
            callbackValues[kArrayIndexSecond] = callbackValue;
        } else {
            std::string errTip = std::to_string(result);
            callbackValues[kArrayIndexFirst] = CreateErrorMessage(env, errTip);
            callbackValues[kArrayIndexSecond] = CreateUndefined(env);
        }
        napi_value callbackFunc = nullptr;
        napi_get_reference_value(env, supplementInfo.callbackRef, &callbackFunc);
        napi_value callbackResult = nullptr;
        napi_call_function(
            env, supplementInfo.thisVar, callbackFunc, kDataLengthTwo, callbackValues, &callbackResult);
        napi_delete_reference(env, supplementInfo.callbackRef);
    } else if (supplementInfo.deferred != nullptr) {
        TELEPHONY_LOGE("supplementInfo.deferred  on ");
        if (result == TELEPHONY_SUCCESS) {
            napi_resolve_deferred(env, supplementInfo.deferred, callbackValue);
        } else {
            std::string errTip = std::to_string(result);
            napi_reject_deferred(env, supplementInfo.deferred, CreateErrorMessage(env, errTip));
        }
    }
}

int32_t NapiCallAbilityCallback::ReportSettingInfo(EventListener &settingInfo, AppExecFwk::PacMap &resultInfo)
{
    napi_handle_scope scope = nullptr;
    napi_env env = settingInfo.env;
    napi_open_handle_scope(env, &scope);

    napi_value callbackValue = nullptr;
    napi_create_object(env, &callbackValue);
    int32_t result = resultInfo.GetIntValue("result");

    if (settingInfo.callbackRef != nullptr) {
        napi_value callbackValues[kArrayIndexThird] = {0};
        if (result == TELEPHONY_SUCCESS) {
            callbackValues[kArrayIndexFirst] = CreateUndefined(env);
            napi_get_null(env, &callbackValues[kArrayIndexSecond]);
        } else {
            std::string errTip = std::to_string(result);
            callbackValues[kArrayIndexFirst] = CreateErrorMessage(env, errTip);
            callbackValues[kArrayIndexSecond] = CreateUndefined(env);
        }
        napi_value callbackFunc = nullptr;
        napi_get_reference_value(env, settingInfo.callbackRef, &callbackFunc);
        napi_value callbackResult = nullptr;
        napi_call_function(env, settingInfo.thisVar, callbackFunc, kDataLengthTwo, callbackValues, &callbackResult);
        napi_delete_reference(env, settingInfo.callbackRef);
    } else if (settingInfo.deferred != nullptr) {
        if (result == TELEPHONY_SUCCESS) {
            napi_value promiseValue = nullptr;
            napi_get_null(env, &promiseValue);
            napi_resolve_deferred(env, settingInfo.deferred, promiseValue);
        } else {
            std::string errTip = std::to_string(result);
            napi_reject_deferred(env, settingInfo.deferred, CreateErrorMessage(env, errTip));
        }
    }
    if (settingInfo.thisVar) {
        (void)memset_s(&settingInfo, sizeof(EventListener), 0, sizeof(EventListener));
    }
    napi_close_handle_scope(env, scope);
    return TELEPHONY_SUCCESS;
}

napi_value NapiCallAbilityCallback::CreateUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiCallAbilityCallback::CreateErrorMessage(napi_env env, std::string msg)
{
    napi_value result = nullptr;
    napi_value message = nullptr;
    napi_create_string_utf8(env, (char *)msg.data(), msg.size(), &message);
    napi_create_error(env, nullptr, message, &result);
    return result;
}

void NapiCallAbilityCallback::SetPropertyInt32(napi_env env, napi_value object, std::string name, int32_t value)
{
    napi_value peopertyValue = nullptr;
    napi_create_int32(env, value, &peopertyValue);
    napi_set_named_property(env, object, name.c_str(), peopertyValue);
}

void NapiCallAbilityCallback::SetPropertyStringUtf8(
    napi_env env, napi_value object, std::string name, std::string value)
{
    napi_value peopertyValue = nullptr;
    char *valueChars = (char *)value.c_str();
    napi_create_string_utf8(env, valueChars, std::strlen(valueChars), &peopertyValue);
    napi_set_named_property(env, object, name.c_str(), peopertyValue);
}
} // namespace Telephony
} // namespace OHOS