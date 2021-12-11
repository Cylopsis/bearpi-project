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

#include "napi_call_manager.h"

#include <securec.h>

#include "system_ability_definition.h"

#include "telephony_log_wrapper.h"

#include "call_manager_errors.h"
#include "napi_call_manager_types.h"
#include "napi_call_ability_callback.h"
#include "napi_call_ability_handler.h"
#include "call_manager_proxy.h"
#include "napi_call_manager.h"

namespace OHOS {
namespace Telephony {
NapiCallManager::NapiCallManager() {}

void Init()
{
    DelayedSingleton<CallManagerProxy>::GetInstance()->Init(TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID);
    DelayedSingleton<NapiCallAbilityHandlerService>::GetInstance()->Start();
}

napi_value NapiCallManager::CallManagerEnumTypeInit(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_STATIC_PROPERTY("CALL_WAITING_DISABLE", ToInt32Value(env, CALL_WAITING_DISABLE)),
        DECLARE_NAPI_STATIC_PROPERTY("CALL_WAITING_ENABLE", ToInt32Value(env, CALL_WAITING_ENABLE)),
        DECLARE_NAPI_STATIC_PROPERTY("CALL_STATUS_ACTIVE", ToInt32Value(env, CALL_STATUS_ACTIVE)),
        DECLARE_NAPI_STATIC_PROPERTY("CALL_STATUS_HOLDING", ToInt32Value(env, CALL_STATUS_HOLDING)),
        DECLARE_NAPI_STATIC_PROPERTY("CALL_STATUS_DIALING", ToInt32Value(env, CALL_STATUS_DIALING)),
        DECLARE_NAPI_STATIC_PROPERTY("CALL_STATUS_ALERTING", ToInt32Value(env, CALL_STATUS_ALERTING)),
        DECLARE_NAPI_STATIC_PROPERTY("CALL_STATUS_INCOMING", ToInt32Value(env, CALL_STATUS_INCOMING)),
        DECLARE_NAPI_STATIC_PROPERTY("CALL_STATUS_WAITING", ToInt32Value(env, CALL_STATUS_WAITING)),
        DECLARE_NAPI_STATIC_PROPERTY("CALL_STATUS_DISCONNECTED", ToInt32Value(env, CALL_STATUS_DISCONNECTED)),
        DECLARE_NAPI_STATIC_PROPERTY("CALL_STATUS_DISCONNECTING", ToInt32Value(env, CALL_STATUS_DISCONNECTING)),
        DECLARE_NAPI_STATIC_PROPERTY("CALL_STATUS_IDLE", ToInt32Value(env, CALL_STATUS_IDLE)),
        DECLARE_NAPI_STATIC_PROPERTY(
            "CALL_STATE_UNKNOWN", ToInt32Value(env, (int32_t)CallStateToApp::CALL_STATE_UNKNOWN)),
        DECLARE_NAPI_STATIC_PROPERTY("CALL_STATE_IDLE", ToInt32Value(env, (int32_t)CallStateToApp::CALL_STATE_IDLE)),
        DECLARE_NAPI_STATIC_PROPERTY(
            "CALL_STATE_RINGING", ToInt32Value(env, (int32_t)CallStateToApp::CALL_STATE_RINGING)),
        DECLARE_NAPI_STATIC_PROPERTY(
            "CALL_STATE_OFFHOOK", ToInt32Value(env, (int32_t)CallStateToApp::CALL_STATE_OFFHOOK)),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}

napi_value NapiCallManager::RegisterCallManagerFunc(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("dial", DialCall),
        DECLARE_NAPI_FUNCTION("answer", AnswerCall),
        DECLARE_NAPI_FUNCTION("reject", RejectCall),
        DECLARE_NAPI_FUNCTION("hangup", HangUpCall),
        DECLARE_NAPI_FUNCTION("holdCall", HoldCall),
        DECLARE_NAPI_FUNCTION("unHoldCall", UnHoldCall),
        DECLARE_NAPI_FUNCTION("switchCall", SwitchCall),
        DECLARE_NAPI_FUNCTION("combineConference", CombineConference),
        DECLARE_NAPI_FUNCTION("getMainCallId", GetMainCallId),
        DECLARE_NAPI_FUNCTION("getSubCallIdList", GetSubCallIdList),
        DECLARE_NAPI_FUNCTION("getCallIdListForConference", GetCallIdListForConference),
        DECLARE_NAPI_FUNCTION("getCallWaitingStatus", GetCallWaiting),
        DECLARE_NAPI_FUNCTION("setCallWaiting", SetCallWaiting),
        DECLARE_NAPI_FUNCTION("startDTMF", StartDtmf),
        DECLARE_NAPI_FUNCTION("stopDTMF", StopDtmf),
        DECLARE_NAPI_FUNCTION("getCallState", GetCallState),
        DECLARE_NAPI_FUNCTION("hasCall", HasCall),
        DECLARE_NAPI_FUNCTION("isInEmergencyCall", IsInEmergencyCall),
        DECLARE_NAPI_FUNCTION("isEmergencyPhoneNumber", IsEmergencyPhoneNumber),
        DECLARE_NAPI_FUNCTION("formatPhoneNumber", FormatPhoneNumber),
        DECLARE_NAPI_FUNCTION("formatPhoneNumberToE164", FormatPhoneNumberToE164),
        DECLARE_NAPI_FUNCTION("on", ObserverOn),
        DECLARE_NAPI_FUNCTION("off", ObserverOff),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    CallManagerEnumTypeInit(env, exports);
    Init();
    return exports;
}

napi_value NapiCallManager::DialCall(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc <= kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_string);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is srting");
    auto asyncContext = (std::make_unique<DialAsyncContext>()).release();
    napi_get_value_string_utf8(
        env, argv[kArrayIndexFirst], asyncContext->number, kPhoneNumberMaximumLimit, &(asyncContext->numberLen));
    if (argc == kTwoValueLimit) {
        if (MatchValueType(env, argv[kArrayIndexSecond], napi_function)) {
            napi_create_reference(env, argv[kArrayIndexSecond], kDataLengthOne, &(asyncContext->callbackRef));
        } else if (MatchValueType(env, argv[kArrayIndexSecond], napi_object)) {
            GetDialInfo(env, argv[kArrayIndexSecond], *asyncContext);
        }
    } else if (argc == kValueMaximumLimit && MatchValueType(env, argv[kArrayIndexSecond], napi_object)) {
        GetDialInfo(env, argv[kArrayIndexSecond], *asyncContext);
        napi_create_reference(env, argv[kArrayIndexThird], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "DialCall", NativeDialCall, NativeDialCallBack);
}

napi_value NapiCallManager::AnswerCall(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc <= kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_number);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is number");
    auto asyncContext = (std::make_unique<AnswerAsyncContext>()).release();
    napi_get_value_int32(env, argv[kArrayIndexFirst], &asyncContext->callId);
    if (argc == kTwoValueLimit) {
        if (MatchValueType(env, argv[kArrayIndexSecond], napi_function)) {
            napi_create_reference(env, argv[kArrayIndexSecond], kDataLengthOne, &(asyncContext->callbackRef));
        } else if (MatchValueType(env, argv[kArrayIndexSecond], napi_number)) {
            asyncContext->videoState = GetIntProperty(env, argv[kArrayIndexSecond], "videoState");
        }
    } else if (argc == kValueMaximumLimit) {
        asyncContext->videoState = GetIntProperty(env, argv[kArrayIndexSecond], "videoState");
        napi_create_reference(env, argv[kArrayIndexThird], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "AnswerCall", NativeAnswerCall, NativeVoidCallBack);
}

napi_value NapiCallManager::RejectCall(napi_env env, napi_callback_info info)
{
    size_t argc = kFourValueMaximumLimit;
    napi_value argv[kFourValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc < kFourValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_number);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is number");
    auto asyncContext = (std::make_unique<RejectAsyncContext>()).release();
    napi_get_value_int32(env, argv[kArrayIndexFirst], &asyncContext->callId);
    asyncContext->isSendSms = false;
    if (argc == kTwoValueLimit) {
        if (MatchValueType(env, argv[kArrayIndexSecond], napi_function)) {
            napi_create_reference(env, argv[kArrayIndexSecond], kDataLengthOne, &(asyncContext->callbackRef));
        } else if (MatchValueType(env, argv[kArrayIndexSecond], napi_object)) {
            GetSmsInfo(env, argv[kArrayIndexSecond], *asyncContext);
        }
    } else if (argc == kValueMaximumLimit) {
        if (MatchValueType(env, argv[kArrayIndexSecond], napi_object)) {
            GetSmsInfo(env, argv[kArrayIndexSecond], *asyncContext);
        }
        napi_create_reference(env, argv[kArrayIndexThird], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "RejectCall", NativeRejectCall, NativeVoidCallBack);
}

napi_value NapiCallManager::HangUpCall(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc < kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_number);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is number");
    auto asyncContext = (std::make_unique<AsyncContext>()).release();
    napi_get_value_int32(env, argv[kArrayIndexFirst], &asyncContext->callId);
    if (argc == kTwoValueLimit) {
        napi_create_reference(env, argv[kArrayIndexSecond], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "HangUpCall", NativeHangUpCall, NativeVoidCallBack);
}

napi_value NapiCallManager::HoldCall(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc < kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_number);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is number");
    auto asyncContext = (std::make_unique<AsyncContext>()).release();
    napi_get_value_int32(env, argv[kArrayIndexFirst], &asyncContext->callId);
    if (argc == kTwoValueLimit) {
        napi_create_reference(env, argv[kArrayIndexSecond], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "HoldCall", NativeHoldCall, NativeVoidCallBack);
}

napi_value NapiCallManager::UnHoldCall(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc < kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_number);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is number");
    auto asyncContext = (std::make_unique<AsyncContext>()).release();
    napi_get_value_int32(env, argv[kArrayIndexFirst], &asyncContext->callId);
    if (argc == kTwoValueLimit) {
        napi_create_reference(env, argv[kArrayIndexSecond], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "UnHoldCall", NativeUnHoldCall, NativeVoidCallBack);
}

napi_value NapiCallManager::SwitchCall(napi_env env, napi_callback_info info)
{
    size_t argc = kTwoValueLimit;
    napi_value argv[kTwoValueLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc <= kTwoValueLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_number);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is number");
    auto asyncContext = (std::make_unique<AsyncContext>()).release();
    napi_get_value_int32(env, argv[kArrayIndexFirst], &asyncContext->callId);
    if (argc == kTwoValueLimit) {
        napi_create_reference(env, argv[kArrayIndexSecond], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "SwitchCall", NativeSwitchCall, NativeVoidCallBack);
}

napi_value NapiCallManager::CombineConference(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc < kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_number);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is number");
    auto asyncContext = (std::make_unique<AsyncContext>()).release();
    napi_get_value_int32(env, argv[kArrayIndexFirst], &asyncContext->callId);
    if (argc == kTwoValueLimit) {
        napi_create_reference(env, argv[kArrayIndexSecond], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "CombineConference", NativeCombineConference, NativeVoidCallBack);
}

napi_value NapiCallManager::GetMainCallId(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc < kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_number);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is number");
    auto asyncContext = (std::make_unique<AsyncContext>()).release();
    napi_get_value_int32(env, argv[kArrayIndexFirst], &asyncContext->callId);
    if (argc == kTwoValueLimit) {
        napi_create_reference(env, argv[kArrayIndexSecond], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "GetMainCallId", NativeGetMainCallId, NativePropertyCallBack);
}

napi_value NapiCallManager::GetSubCallIdList(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc < kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_number);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is number");
    auto asyncContext = (std::make_unique<ListAsyncContext>()).release();
    napi_get_value_int32(env, argv[kArrayIndexFirst], &asyncContext->callId);
    if (argc == kTwoValueLimit) {
        napi_create_reference(env, argv[kArrayIndexSecond], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "GetSubCallIdList", NativeGetSubCallIdList, NativeListCallBack);
}

napi_value NapiCallManager::GetCallIdListForConference(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc < kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_number);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is number");
    auto asyncContext = (std::make_unique<ListAsyncContext>()).release();
    napi_get_value_int32(env, argv[kArrayIndexFirst], &asyncContext->callId);
    if (argc == kTwoValueLimit) {
        napi_create_reference(env, argv[kArrayIndexSecond], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(
        env, asyncContext, "GetCallIdListForConference", NativeGetCallIdListForConference, NativeListCallBack);
}

napi_value NapiCallManager::GetCallWaiting(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc < kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_number);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is number");
    auto asyncContext = (std::make_unique<SupplementAsyncContext>()).release();
    EventListener waitingInfoListener;
    waitingInfoListener.env = env;
    waitingInfoListener.thisVar = thisVar;
    napi_get_value_int32(env, argv[kArrayIndexFirst], &asyncContext->slotId);
    if (argc == kTwoValueLimit) {
        napi_create_reference(env, argv[kArrayIndexSecond], kDataLengthOne, &(waitingInfoListener.callbackRef));
    }
    napi_value result = nullptr;
    if (waitingInfoListener.callbackRef == nullptr) {
        napi_create_promise(env, &(waitingInfoListener.deferred), &result);
    } else {
        napi_get_undefined(env, &result);
    }
    int32_t error = TELEPHONY_SUCCESS;
    if (!IsValidSlotId(asyncContext->slotId)) {
        error = ERR_INVALID_SLOT_ID;
    } else {
        error = DelayedSingleton<NapiCallAbilityCallback>::GetInstance()->RegisterGetWaitingCallback(
            waitingInfoListener);
    }
    if (error != TELEPHONY_SUCCESS) {
        asyncContext->callbackRef = waitingInfoListener.callbackRef;
        asyncContext->deferred = waitingInfoListener.deferred;
        asyncContext->result = error;
    }

    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, "GetCallWaiting", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(env, nullptr, resourceName, NativeGetCallWaiting, NativeCallBack,
        (void *)(asyncContext), &(asyncContext->work));
    napi_queue_async_work(env, asyncContext->work);
    return result;
}

napi_value NapiCallManager::SetCallWaiting(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc <= kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_number);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is number");
    matchFlag = MatchValueType(env, argv[kArrayIndexSecond], napi_boolean);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is boolean");
    auto asyncContext = (std::make_unique<SupplementAsyncContext>()).release();
    EventListener waitingInfoListener;
    waitingInfoListener.env = env;
    waitingInfoListener.thisVar = thisVar;
    napi_get_value_int32(env, argv[kArrayIndexFirst], &asyncContext->slotId);
    napi_get_value_bool(env, argv[kArrayIndexSecond], &asyncContext->flag);
    if (argc == kValueMaximumLimit) {
        napi_create_reference(env, argv[kArrayIndexThird], kDataLengthOne, &(waitingInfoListener.callbackRef));
    }
    napi_value result = nullptr;
    if (waitingInfoListener.callbackRef == nullptr) {
        napi_create_promise(env, &(waitingInfoListener.deferred), &result);
    } else {
        napi_get_undefined(env, &result);
    }
    int32_t error = TELEPHONY_SUCCESS;
    if (!IsValidSlotId(asyncContext->slotId)) {
        error = ERR_INVALID_SLOT_ID;
    } else {
        error = DelayedSingleton<NapiCallAbilityCallback>::GetInstance()->RegisterSetWaitingCallback(
            waitingInfoListener);
    }
    if (error != TELEPHONY_SUCCESS) {
        asyncContext->callbackRef = waitingInfoListener.callbackRef;
        asyncContext->deferred = waitingInfoListener.deferred;
        asyncContext->result = error;
    }

    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, "SetCallWaiting", NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(env, nullptr, resourceName, NativeSetCallWaiting, NativeCallBack,
        (void *)(asyncContext), &(asyncContext->work));
    napi_queue_async_work(env, asyncContext->work);
    return result;
}

napi_value NapiCallManager::StartDtmf(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc <= kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_number);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is number");
    auto asyncContext = (std::make_unique<SupplementAsyncContext>()).release();
    napi_get_value_int32(env, argv[kArrayIndexFirst], &asyncContext->callId);
    napi_get_value_string_utf8(
        env, argv[kArrayIndexSecond], asyncContext->number, kPhoneNumberMaximumLimit, &(asyncContext->numberLen));
    if (argc == kValueMaximumLimit) {
        napi_create_reference(env, argv[kArrayIndexThird], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "StartDtmf", NativeStartDtmf, NativeVoidCallBack);
}

napi_value NapiCallManager::StopDtmf(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc < kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_number);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is number");
    auto asyncContext = (std::make_unique<AsyncContext>()).release();
    napi_get_value_int32(env, argv[kArrayIndexFirst], &asyncContext->callId);
    if (argc == kTwoValueLimit) {
        napi_create_reference(env, argv[kArrayIndexSecond], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "StopDtmf", NativeStopDtmf, NativeVoidCallBack);
}

napi_value NapiCallManager::SendDtmf(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc <= kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_number);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is number");
    auto asyncContext = (std::make_unique<AsyncContext>()).release();
    napi_get_value_int32(env, argv[kArrayIndexFirst], &asyncContext->callId);
    napi_get_value_string_utf8(
        env, argv[kArrayIndexSecond], asyncContext->number, kPhoneNumberMaximumLimit, &(asyncContext->numberLen));
    if (argc == kValueMaximumLimit) {
        napi_create_reference(env, argv[kArrayIndexThird], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "SendDtmf", NativeSendDtmf, NativeCallBack);
}

napi_value NapiCallManager::SendBurstDtmf(napi_env env, napi_callback_info info)
{
    size_t argc = kTwoValueLimit;
    napi_value argv[kTwoValueLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc <= kTwoValueLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_object);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is object");
    auto asyncContext = (std::make_unique<DtmfAsyncContext>()).release();
    GetDtmfBunchInfo(env, argv[kArrayIndexFirst], *asyncContext);
    if (argc == kTwoValueLimit) {
        napi_create_reference(env, argv[kArrayIndexSecond], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "SendBurstDtmf", NativeSendDtmfBunch, NativeCallBack);
}

napi_value NapiCallManager::GetCallState(napi_env env, napi_callback_info info)
{
    size_t argc = kTwoValueLimit;
    napi_value argv[kTwoValueLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc < kTwoValueLimit, "parameter error!");
    auto asyncContext = (std::make_unique<AsyncContext>()).release();
    if (argc == kOnlyOneValue) {
        napi_create_reference(env, argv[kArrayIndexFirst], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "GetCallState", NativeGetCallState, NativePropertyCallBack);
}

napi_value NapiCallManager::IsRinging(napi_env env, napi_callback_info info)
{
    size_t argc = kTwoValueLimit;
    napi_value argv[kTwoValueLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc < kTwoValueLimit, "parameter error!");
    auto asyncContext = (std::make_unique<AsyncContext>()).release();
    if (argc == kOnlyOneValue) {
        napi_create_reference(env, argv[kArrayIndexFirst], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "IsRinging", NativeIsRinging, NativeBoolCallBack);
}

napi_value NapiCallManager::HasCall(napi_env env, napi_callback_info info)
{
    size_t argc = kTwoValueLimit;
    napi_value argv[kTwoValueLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc < kTwoValueLimit, "parameter error!");
    auto asyncContext = (std::make_unique<AsyncContext>()).release();
    if (argc == kOnlyOneValue) {
        napi_create_reference(env, argv[kArrayIndexFirst], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "HasCall", NativeHasCall, NativeBoolCallBack);
}

napi_value NapiCallManager::IsNewCallAllowed(napi_env env, napi_callback_info info)
{
    size_t argc = kTwoValueLimit;
    napi_value argv[kTwoValueLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc < kTwoValueLimit, "parameter error!");
    auto asyncContext = (std::make_unique<AsyncContext>()).release();
    if (argc == kOnlyOneValue) {
        napi_create_reference(env, argv[kArrayIndexFirst], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "IsNewCallAllowed", NativeIsNewCallAllowed, NativeBoolCallBack);
}

napi_value NapiCallManager::IsInEmergencyCall(napi_env env, napi_callback_info info)
{
    size_t argc = kTwoValueLimit;
    napi_value argv[kTwoValueLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc < kTwoValueLimit, "parameter error!");
    auto asyncContext = (std::make_unique<AsyncContext>()).release();
    if (argc == kOnlyOneValue) {
        napi_create_reference(env, argv[kArrayIndexFirst], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(env, asyncContext, "IsInEmergencyCall", NativeIsInEmergencyCall, NativeBoolCallBack);
}

napi_value NapiCallManager::IsEmergencyPhoneNumber(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc <= kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_string);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is srting");
    auto asyncContext = (std::make_unique<UtilsAsyncContext>()).release();
    napi_get_value_string_utf8(
        env, argv[kArrayIndexFirst], asyncContext->number, kPhoneNumberMaximumLimit, &(asyncContext->numberLen));
    asyncContext->slotId = 0;
    if (argc == kTwoValueLimit) {
        if (MatchValueType(env, argv[kArrayIndexSecond], napi_function)) {
            napi_create_reference(env, argv[kArrayIndexSecond], kDataLengthOne, &(asyncContext->callbackRef));
        } else if (MatchValueType(env, argv[kArrayIndexSecond], napi_object)) {
            asyncContext->slotId = GetIntProperty(env, argv[kArrayIndexSecond], "slotId");
        }
    } else if (argc == kValueMaximumLimit && MatchValueType(env, argv[kArrayIndexSecond], napi_object)) {
        asyncContext->slotId = GetIntProperty(env, argv[kArrayIndexSecond], "slotId");
        napi_create_reference(env, argv[kArrayIndexThird], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(
        env, asyncContext, "IsEmergencyPhoneNumber", NativeIsEmergencyPhoneNumber, NativeBoolCallBack);
}

napi_value NapiCallManager::FormatPhoneNumber(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc <= kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_string);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is srting");
    auto asyncContext = (std::make_unique<UtilsAsyncContext>()).release();
    asyncContext->code = "cn";
    napi_get_value_string_utf8(
        env, argv[kArrayIndexFirst], asyncContext->number, kPhoneNumberMaximumLimit, &(asyncContext->numberLen));
    if (argc == kTwoValueLimit) {
        if (MatchValueType(env, argv[kArrayIndexSecond], napi_function)) {
            napi_create_reference(env, argv[kArrayIndexSecond], kDataLengthOne, &(asyncContext->callbackRef));
        } else if (MatchValueType(env, argv[kArrayIndexSecond], napi_object)) {
            asyncContext->code = GetStringProperty(env, argv[kArrayIndexSecond], "countryCode");
        }
    } else if (argc == kValueMaximumLimit && MatchValueType(env, argv[kArrayIndexSecond], napi_object)) {
        asyncContext->code = GetStringProperty(env, argv[kArrayIndexSecond], "countryCode");
        napi_create_reference(env, argv[kArrayIndexThird], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(
        env, asyncContext, "FormatPhoneNumber", NativeFormatPhoneNumber, NativeFormatNumberCallBack);
}

napi_value NapiCallManager::FormatPhoneNumberToE164(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc <= kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_string);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is srting");
    matchFlag = MatchValueType(env, argv[kArrayIndexSecond], napi_string);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is srting");
    auto asyncContext = (std::make_unique<UtilsAsyncContext>()).release();
    napi_get_value_string_utf8(
        env, argv[kArrayIndexFirst], asyncContext->number, kPhoneNumberMaximumLimit, &(asyncContext->numberLen));
    char tmpStr[kMaxNumberLen];
    size_t strLen = 0;
    napi_get_value_string_utf8(env, argv[kArrayIndexSecond], tmpStr, kPhoneNumberMaximumLimit, &strLen);
    std::string tmpCode(tmpStr, strLen);
    asyncContext->code = tmpCode;
    if (argc == kValueMaximumLimit) {
        napi_create_reference(env, argv[kArrayIndexThird], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(
        env, asyncContext, "FormatPhoneNumberToE164", NativeFormatPhoneNumberToE164, NativeFormatNumberCallBack);
}

napi_value NapiCallManager::ObserverOn(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc <= kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_string);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is srting");
    char listenerType[kPhoneNumberMaximumLimit];
    size_t strLength = 0;
    napi_get_value_string_utf8(env, argv[kArrayIndexFirst], listenerType, kPhoneNumberMaximumLimit, &strLength);
    std::string tmpStr = listenerType;
    TELEPHONY_LOGE("listenerType == %{public}s", tmpStr.c_str());
    if (tmpStr == "callDetailsChange") {
        EventListener callStateListener;
        (void)memset_s(&callStateListener, sizeof(EventListener), 0, sizeof(EventListener));
        callStateListener.env = env;
        callStateListener.thisVar = thisVar;
        napi_create_reference(env, argv[kArrayIndexSecond], kDataLengthOne, &callStateListener.callbackRef);
        DelayedSingleton<NapiCallAbilityCallback>::GetInstance()->RegisterCallStateCallback(callStateListener);
    }
    napi_value result = nullptr;
    return result;
}

napi_value NapiCallManager::ObserverOff(napi_env env, napi_callback_info info)
{
    size_t argc = kValueMaximumLimit;
    napi_value argv[kValueMaximumLimit] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, &thisVar, &data));
    NAPI_ASSERT(env, argc <= kValueMaximumLimit, "parameter error!");
    bool matchFlag = MatchValueType(env, argv[kArrayIndexFirst], napi_string);
    NAPI_ASSERT(env, matchFlag, "Type error, Should is srting");
    auto asyncContext = (std::make_unique<AsyncContext>()).release();
    char listenerType[kPhoneNumberMaximumLimit];
    size_t strLength = 0;
    napi_get_value_string_utf8(env, argv[kArrayIndexFirst], listenerType, kPhoneNumberMaximumLimit, &strLength);
    std::string tmpStr = listenerType;
    TELEPHONY_LOGE("listenerType == %{public}s", tmpStr.c_str());
    if (tmpStr == "callDetailsChange") {
        DelayedSingleton<NapiCallAbilityCallback>::GetInstance()->UnRegisterCallStateCallback();
    }
    asyncContext->result = TELEPHONY_SUCCESS;
    if (argc == kTwoValueLimit) {
        napi_create_reference(env, argv[kArrayIndexSecond], kDataLengthOne, &(asyncContext->callbackRef));
    }
    return HandleAsyncWork(
        env, asyncContext, "Off", [](napi_env env, void *data) {}, NativeVoidCallBack);
}

bool NapiCallManager::MatchValueType(napi_env env, napi_value value, napi_valuetype targetType)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    return valueType == targetType;
}

napi_value NapiCallManager::CreateUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

napi_value NapiCallManager::CreateErrorMessage(napi_env env, std::string msg)
{
    napi_value result = nullptr;
    napi_value message = nullptr;
    napi_create_string_utf8(env, (char *)msg.data(), msg.size(), &message);
    napi_create_error(env, nullptr, message, &result);
    return result;
}

void NapiCallManager::NativeCallBack(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    if (asyncContext->deferred != nullptr) {
        if (asyncContext->result == TELEPHONY_SUCCESS) {
            napi_value promiseValue = nullptr;
            napi_create_int32(env, asyncContext->result, &promiseValue);
            napi_resolve_deferred(env, asyncContext->deferred, promiseValue);
        } else {
            std::string errTip = std::to_string(asyncContext->result);
            napi_reject_deferred(env, asyncContext->deferred, NapiCallManager::CreateErrorMessage(env, errTip));
        }
    } else {
        napi_value callbackValue[kArrayIndexThird] = {0};
        if (asyncContext->result == TELEPHONY_SUCCESS) {
            callbackValue[kArrayIndexFirst] = NapiCallManager::CreateUndefined(env);
            napi_create_int32(env, asyncContext->result, &callbackValue[kArrayIndexSecond]);
        } else {
            int32_t errCode = TELEPHONY_FAIL;
            errCode = asyncContext->result;
            std::string errTip = std::to_string(errCode);
            callbackValue[kArrayIndexFirst] = NapiCallManager::CreateErrorMessage(env, errTip);
            callbackValue[kArrayIndexSecond] = NapiCallManager::CreateUndefined(env);
        }
        napi_value callback = nullptr;
        napi_value result = nullptr;
        napi_get_reference_value(env, asyncContext->callbackRef, &callback);
        napi_call_function(env, nullptr, callback, std::size(callbackValue), callbackValue, &result);
        napi_delete_reference(env, asyncContext->callbackRef);
    }
    napi_delete_async_work(env, asyncContext->work);
    delete asyncContext;
}

void NapiCallManager::NativeDialCallBack(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    if (asyncContext->deferred != nullptr) {
        if (asyncContext->result == TELEPHONY_SUCCESS) {
            napi_value promiseValue = nullptr;
            napi_get_boolean(env, true, &promiseValue);
            napi_resolve_deferred(env, asyncContext->deferred, promiseValue);
        } else {
            std::string errTip = std::to_string(asyncContext->result);
            napi_reject_deferred(env, asyncContext->deferred, NapiCallManager::CreateErrorMessage(env, errTip));
        }
    } else {
        napi_value callbackValue[kArrayIndexThird] = {0};
        if (asyncContext->result == TELEPHONY_SUCCESS) {
            callbackValue[kArrayIndexFirst] = NapiCallManager::CreateUndefined(env);
            napi_get_boolean(env, true, &callbackValue[kArrayIndexSecond]);
        } else {
            int32_t errCode = TELEPHONY_FAIL;
            errCode = asyncContext->result;
            std::string errTip = std::to_string(errCode);
            callbackValue[kArrayIndexFirst] = NapiCallManager::CreateErrorMessage(env, errTip);
            callbackValue[kArrayIndexSecond] = NapiCallManager::CreateUndefined(env);
        }
        napi_value callback = nullptr;
        napi_value result = nullptr;
        napi_get_reference_value(env, asyncContext->callbackRef, &callback);
        napi_call_function(env, nullptr, callback, std::size(callbackValue), callbackValue, &result);
        napi_delete_reference(env, asyncContext->callbackRef);
    }
    napi_delete_async_work(env, asyncContext->work);
    delete asyncContext;
}

void NapiCallManager::NativeVoidCallBack(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    if (asyncContext->deferred != nullptr) {
        if (asyncContext->result == TELEPHONY_SUCCESS) {
            napi_value promiseValue = nullptr;
            napi_get_null(env, &promiseValue);
            napi_resolve_deferred(env, asyncContext->deferred, promiseValue);
        } else {
            std::string errTip = std::to_string(asyncContext->result);
            napi_reject_deferred(env, asyncContext->deferred, NapiCallManager::CreateErrorMessage(env, errTip));
        }
    } else {
        napi_value callbackValue[kArrayIndexThird] = {0};
        if (asyncContext->result == TELEPHONY_SUCCESS) {
            callbackValue[kArrayIndexFirst] = NapiCallManager::CreateUndefined(env);
            napi_get_null(env, &callbackValue[kArrayIndexSecond]);
        } else {
            int32_t errCode = TELEPHONY_FAIL;
            errCode = asyncContext->result;
            std::string errTip = std::to_string(errCode);
            callbackValue[kArrayIndexFirst] = NapiCallManager::CreateErrorMessage(env, errTip);
            callbackValue[kArrayIndexSecond] = NapiCallManager::CreateUndefined(env);
        }
        napi_value callback = nullptr;
        napi_value result = nullptr;
        napi_get_reference_value(env, asyncContext->callbackRef, &callback);
        napi_call_function(env, nullptr, callback, std::size(callbackValue), callbackValue, &result);
        napi_delete_reference(env, asyncContext->callbackRef);
    }
    napi_delete_async_work(env, asyncContext->work);
    delete asyncContext;
}

void NapiCallManager::NativePropertyCallBack(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    if (asyncContext->deferred != nullptr) {
        napi_value promiseValue = nullptr;
        napi_create_int32(env, asyncContext->result, &promiseValue);
        napi_resolve_deferred(env, asyncContext->deferred, promiseValue);
    } else {
        napi_value callbackValue[kArrayIndexThird] = {0};
        callbackValue[kArrayIndexFirst] = NapiCallManager::CreateUndefined(env);
        napi_create_int32(env, asyncContext->result, &callbackValue[kArrayIndexSecond]);
        napi_value callback = nullptr;
        napi_value result = nullptr;
        napi_get_reference_value(env, asyncContext->callbackRef, &callback);
        napi_call_function(env, nullptr, callback, std::size(callbackValue), callbackValue, &result);
        napi_delete_reference(env, asyncContext->callbackRef);
    }
    napi_delete_async_work(env, asyncContext->work);
    delete asyncContext;
}

void NapiCallManager::NativeBoolCallBack(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    if (asyncContext->deferred != nullptr) {
        if (asyncContext->result != kBoolValueIsFalse) {
            napi_value promiseValue = nullptr;
            napi_get_boolean(env, true, &promiseValue);
            napi_resolve_deferred(env, asyncContext->deferred, promiseValue);
        } else if (asyncContext->errorCode == TELEPHONY_SUCCESS) {
            napi_value promiseValue = nullptr;
            napi_get_boolean(env, false, &promiseValue);
            napi_resolve_deferred(env, asyncContext->deferred, promiseValue);
        } else {
            std::string errTip = std::to_string(asyncContext->errorCode);
            napi_reject_deferred(env, asyncContext->deferred, NapiCallManager::CreateErrorMessage(env, errTip));
        }
    } else {
        napi_value callbackValue[kArrayIndexThird] = {0};
        if (asyncContext->result != kBoolValueIsFalse) {
            callbackValue[kArrayIndexFirst] = NapiCallManager::CreateUndefined(env);
            napi_get_boolean(env, true, &callbackValue[kArrayIndexSecond]);
        } else if (asyncContext->errorCode == TELEPHONY_SUCCESS) {
            callbackValue[kArrayIndexFirst] = NapiCallManager::CreateUndefined(env);
            napi_get_boolean(env, false, &callbackValue[kArrayIndexSecond]);
        } else {
            std::string errTip = std::to_string(asyncContext->errorCode);
            callbackValue[kArrayIndexFirst] = NapiCallManager::CreateErrorMessage(env, errTip);
            callbackValue[kArrayIndexSecond] = NapiCallManager::CreateUndefined(env);
        }
        napi_value callback = nullptr;
        napi_value result = nullptr;
        napi_get_reference_value(env, asyncContext->callbackRef, &callback);
        napi_call_function(env, nullptr, callback, std::size(callbackValue), callbackValue, &result);
        napi_delete_reference(env, asyncContext->callbackRef);
    }
    napi_delete_async_work(env, asyncContext->work);
    delete asyncContext;
}

void NapiCallManager::NativeFormatNumberCallBack(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (UtilsAsyncContext *)data;
    if (asyncContext->deferred != nullptr) {
        if (asyncContext->result == TELEPHONY_SUCCESS) {
            napi_value promiseValue = nullptr;
            napi_create_string_utf8(env, Str16ToStr8(asyncContext->formatNumber).data(),
                asyncContext->formatNumber.length(), &promiseValue);
            napi_resolve_deferred(env, asyncContext->deferred, promiseValue);
        } else {
            std::string errTip = std::to_string(asyncContext->result);
            napi_reject_deferred(env, asyncContext->deferred, NapiCallManager::CreateErrorMessage(env, errTip));
        }
    } else {
        napi_value callbackValue[kArrayIndexThird] = {0};
        if (asyncContext->result == TELEPHONY_SUCCESS) {
            callbackValue[kArrayIndexFirst] = NapiCallManager::CreateUndefined(env);
            napi_create_string_utf8(env, Str16ToStr8(asyncContext->formatNumber).data(),
                asyncContext->formatNumber.length(), &callbackValue[kArrayIndexSecond]);
        } else {
            int32_t errCode = TELEPHONY_FAIL;
            errCode = asyncContext->result;
            std::string errTip = std::to_string(errCode);
            callbackValue[kArrayIndexFirst] = NapiCallManager::CreateErrorMessage(env, errTip);
            callbackValue[kArrayIndexSecond] = NapiCallManager::CreateUndefined(env);
        }
        napi_value callback = nullptr;
        napi_value result = nullptr;
        napi_get_reference_value(env, asyncContext->callbackRef, &callback);
        napi_call_function(env, nullptr, callback, std::size(callbackValue), callbackValue, &result);
        napi_delete_reference(env, asyncContext->callbackRef);
    }
    napi_delete_async_work(env, asyncContext->work);
    delete asyncContext;
}

void NapiCallManager::NativeListCallBack(napi_env env, napi_status status, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (ListAsyncContext *)data;
    if (asyncContext->deferred != nullptr) {
        napi_value promiseValue = nullptr;
        napi_create_array(env, &promiseValue);
        std::vector<std::u16string>::iterator it = asyncContext->listResult.begin();
        int i = 0;
        for (; it != asyncContext->listResult.end(); it++) {
            napi_value info = nullptr;
            napi_create_string_utf8(env, Str16ToStr8(*it).data(), (*it).length(), &info);
            napi_set_element(env, promiseValue, i, info);
            ++i;
        }
        napi_resolve_deferred(env, asyncContext->deferred, promiseValue);
    } else {
        napi_value callbackValue[kArrayIndexThird] = {0};
        callbackValue[kArrayIndexFirst] = NapiCallManager::CreateUndefined(env);
        napi_create_array(env, &callbackValue[kArrayIndexSecond]);
        std::vector<std::u16string>::iterator it = asyncContext->listResult.begin();
        int i = 0;
        for (; it != asyncContext->listResult.end(); it++) {
            napi_value info = nullptr;
            napi_create_string_utf8(env, Str16ToStr8(*it).data(), (*it).length(), &info);
            napi_set_element(env, callbackValue[kArrayIndexSecond], i, info);
            ++i;
        }
        napi_value callback = nullptr;
        napi_value result = nullptr;
        napi_get_reference_value(env, asyncContext->callbackRef, &callback);
        napi_call_function(env, nullptr, callback, std::size(callbackValue), callbackValue, &result);
        napi_delete_reference(env, asyncContext->callbackRef);
    }
    napi_delete_async_work(env, asyncContext->work);
    delete asyncContext;
}

void NapiCallManager::GetDialInfo(napi_env env, napi_value objValue, DialAsyncContext &asyncContext)
{
    asyncContext.accountId = GetIntProperty(env, objValue, "accountId");
    asyncContext.videoState = GetIntProperty(env, objValue, "videoState");
    asyncContext.dialScene = GetIntProperty(env, objValue, "dialScene");
    asyncContext.dialType = GetIntProperty(env, objValue, "dialType");
}

void NapiCallManager::GetDtmfBunchInfo(napi_env env, napi_value objValue, DtmfAsyncContext &asyncContext)
{
    asyncContext.callId = GetIntProperty(env, objValue, "callId");
    asyncContext.code = GetStringProperty(env, objValue, "str");
    asyncContext.on = GetIntProperty(env, objValue, "on");
    asyncContext.off = GetIntProperty(env, objValue, "off");
    if (asyncContext.off == 0) {
        asyncContext.off = kDtmfDefaultOff;
    }

    TELEPHONY_LOGE("GetDtmfBunchInfo === callId = %{public}d, str = %{public}s, on = %{public}d, off = %{public}d",
        asyncContext.callId, asyncContext.code.c_str(), asyncContext.on, asyncContext.off);
}

void NapiCallManager::GetRestrictionInfo(napi_env env, napi_value objValue, SupplementAsyncContext &asyncContext)
{
    asyncContext.type = GetIntProperty(env, objValue, "type");
    asyncContext.mode = GetIntProperty(env, objValue, "mode");
    asyncContext.content = GetStringProperty(env, objValue, "password");
}

void NapiCallManager::GetTransferInfo(napi_env env, napi_value objValue, SupplementAsyncContext &asyncContext)
{
    asyncContext.type = GetIntProperty(env, objValue, "type");
    asyncContext.mode = GetIntProperty(env, objValue, "settingType");
    asyncContext.content = GetStringProperty(env, objValue, "phoneNumber");
}

void NapiCallManager::NativeDialCall(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (DialAsyncContext *)data;
    std::string phoneNumber(asyncContext->number, asyncContext->numberLen);
    OHOS::AppExecFwk::PacMap dialInfo;
    dialInfo.PutIntValue("accountId", asyncContext->accountId);
    dialInfo.PutIntValue("videoState", asyncContext->videoState);
    dialInfo.PutIntValue("dialScene", asyncContext->dialScene);
    dialInfo.PutIntValue("dialType", asyncContext->dialType);

    asyncContext->result =
        DelayedSingleton<CallManagerProxy>::GetInstance()->DialCall(Str8ToStr16(phoneNumber), dialInfo);
}

void NapiCallManager::NativeAnswerCall(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AnswerAsyncContext *)data;
    int32_t ret = CALL_MANAGER_ACCEPT_FAILED;
    ret = DelayedSingleton<CallManagerProxy>::GetInstance()->AnswerCall(
        asyncContext->callId, asyncContext->videoState);
    asyncContext->result = ret;
}

void NapiCallManager::NativeRejectCall(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (RejectAsyncContext *)data;
    int32_t ret = DelayedSingleton<CallManagerProxy>::GetInstance()->RejectCall(
        asyncContext->callId, asyncContext->isSendSms, Str8ToStr16(asyncContext->messageContent));
    asyncContext->result = ret;
}

void NapiCallManager::NativeHangUpCall(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->HangUpCall(asyncContext->callId);
}

void NapiCallManager::NativeHoldCall(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->HoldCall(asyncContext->callId);
}

void NapiCallManager::NativeUnHoldCall(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->UnHoldCall(asyncContext->callId);
}

void NapiCallManager::NativeSwitchCall(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->SwitchCall(asyncContext->callId);
}

void NapiCallManager::NativeCombineConference(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    asyncContext->result =
        DelayedSingleton<CallManagerProxy>::GetInstance()->CombineConference(asyncContext->callId);
}

void NapiCallManager::NativeGetMainCallId(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->GetMainCallId(asyncContext->callId);
}

void NapiCallManager::NativeGetSubCallIdList(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (ListAsyncContext *)data;
    asyncContext->listResult =
        DelayedSingleton<CallManagerProxy>::GetInstance()->GetSubCallIdList(asyncContext->callId);
}

void NapiCallManager::NativeGetCallIdListForConference(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (ListAsyncContext *)data;
    asyncContext->listResult =
        DelayedSingleton<CallManagerProxy>::GetInstance()->GetCallIdListForConference(asyncContext->callId);
}

void NapiCallManager::NativeGetCallWaiting(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (SupplementAsyncContext *)data;
    if (asyncContext->result != TELEPHONY_SUCCESS) {
        return;
    }
    asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->GetCallWaiting(asyncContext->slotId);
}

void NapiCallManager::NativeSetCallWaiting(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (SupplementAsyncContext *)data;
    if (asyncContext->result != TELEPHONY_SUCCESS) {
        return;
    }
    asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->SetCallWaiting(
        asyncContext->slotId, asyncContext->flag);
}

void NapiCallManager::NativeStartDtmf(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    if (asyncContext->numberLen < kTwoValueLimit) {
        asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->StartDtmf(
            asyncContext->callId, asyncContext->number[kArrayIndexFirst]);
    } else {
        asyncContext->result = TELEPHONY_FAIL;
    }
}

void NapiCallManager::NativeStopDtmf(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->StopDtmf(asyncContext->callId);
}

void NapiCallManager::NativeSendDtmf(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    if (asyncContext->numberLen < kTwoValueLimit) {
        asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->SendDtmf(
            asyncContext->callId, asyncContext->number[kArrayIndexFirst]);
    } else {
        asyncContext->result = TELEPHONY_FAIL;
    }
}

void NapiCallManager::NativeSendDtmfBunch(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (DtmfAsyncContext *)data;
    asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->SendBurstDtmf(
        asyncContext->callId, Str8ToStr16(asyncContext->code), asyncContext->on, asyncContext->off);
}

void NapiCallManager::NativeGetCallState(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->GetCallState();
}

void NapiCallManager::NativeIsRinging(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->IsRinging();
}

void NapiCallManager::NativeHasCall(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->HasCall();
}

void NapiCallManager::NativeIsNewCallAllowed(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->IsNewCallAllowed();
}

void NapiCallManager::NativeIsInEmergencyCall(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (AsyncContext *)data;
    asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->IsInEmergencyCall();
}

void NapiCallManager::NativeIsEmergencyPhoneNumber(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (UtilsAsyncContext *)data;
    std::string tmpPhoneNumber(asyncContext->number, asyncContext->numberLen);
    std::u16string phoneNumber = Str8ToStr16(tmpPhoneNumber);
    asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->IsEmergencyPhoneNumber(
        phoneNumber, asyncContext->slotId, asyncContext->errorCode);
}

void NapiCallManager::NativeFormatPhoneNumber(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (UtilsAsyncContext *)data;
    std::string tmpPhoneNumber(asyncContext->number, asyncContext->numberLen);
    std::u16string phoneNumber = Str8ToStr16(tmpPhoneNumber);
    std::u16string countryCode = Str8ToStr16(asyncContext->code);
    asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->FormatPhoneNumber(
        phoneNumber, countryCode, asyncContext->formatNumber);
}

void NapiCallManager::NativeFormatPhoneNumberToE164(napi_env env, void *data)
{
    if (data == nullptr) {
        TELEPHONY_LOGE("data is nullptr");
        return;
    }
    auto asyncContext = (UtilsAsyncContext *)data;
    std::string tmpPhoneNumber(asyncContext->number, asyncContext->numberLen);
    std::u16string phoneNumber = Str8ToStr16(tmpPhoneNumber);
    std::u16string countryCode = Str8ToStr16(asyncContext->code);
    asyncContext->result = DelayedSingleton<CallManagerProxy>::GetInstance()->FormatPhoneNumberToE164(
        phoneNumber, countryCode, asyncContext->formatNumber);
}

napi_value NapiCallManager::ToInt32Value(napi_env env, int32_t value)
{
    napi_value staticValue = nullptr;
    napi_create_int32(env, value, &staticValue);
    return staticValue;
}

napi_value NapiCallManager::GetNamedProperty(napi_env env, napi_value object, const std::string &propertyName)
{
    napi_value value = nullptr;
    napi_get_named_property(env, object, propertyName.c_str(), &value);
    return value;
}

std::string NapiCallManager::GetStringProperty(napi_env env, napi_value object, const std::string &propertyName)
{
    napi_value value = nullptr;
    napi_status getNameStatus = napi_get_named_property(env, object, propertyName.c_str(), &value);
    if (getNameStatus == napi_ok) {
        char chars[kPhoneNumberMaximumLimit] = {0};
        size_t charLength = 0;
        napi_status getStringStatus =
            napi_get_value_string_utf8(env, value, chars, kPhoneNumberMaximumLimit, &charLength);
        if (getStringStatus == napi_ok && charLength > 0) {
            return std::string(chars, charLength);
        }
    }
    return "";
}

int32_t NapiCallManager::GetIntProperty(napi_env env, napi_value object, const std::string &propertyName)
{
    int32_t intValue = 0;
    napi_value value = nullptr;
    napi_status getNameStatus = napi_get_named_property(env, object, propertyName.c_str(), &value);
    if (getNameStatus == napi_ok) {
        napi_status getIntStatus = napi_get_value_int32(env, value, &intValue);
        if (getIntStatus == napi_ok) {
            return intValue;
        }
    }
    return intValue;
}

void NapiCallManager::GetSmsInfo(napi_env env, napi_value objValue, RejectAsyncContext &asyncContext)
{
    asyncContext.isSendSms = true;
    asyncContext.messageContent = GetStringProperty(env, objValue, "messageContent");
}

napi_value NapiCallManager::HandleAsyncWork(napi_env env, AsyncContext *context, std::string workName,
    napi_async_execute_callback execute, napi_async_complete_callback complete)
{
    napi_value result = nullptr;
    if (context->callbackRef == nullptr) {
        napi_create_promise(env, &context->deferred, &result);
    } else {
        napi_get_undefined(env, &result);
    }
    napi_value resource = CreateUndefined(env);
    napi_value resourceName = nullptr;
    napi_create_string_utf8(env, workName.data(), NAPI_AUTO_LENGTH, &resourceName);
    napi_create_async_work(env, resource, resourceName, execute, complete, (void *)context, &context->work);
    napi_queue_async_work(env, context->work);
    return result;
}

bool NapiCallManager::IsValidSlotId(int32_t slotId)
{
    return slotId == kDefaultSlotId;
}

static napi_module g_nativeCallManagerModule = {
    .nm_version = kNativeVersion,
    .nm_flags = kNativeFlags,
    .nm_filename = nullptr,
    .nm_register_func = NapiCallManager::RegisterCallManagerFunc,
    .nm_modname = "telephony.call",
    .nm_priv = ((void *)0),
    .reserved = {0},
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_nativeCallManagerModule);
}
} // namespace Telephony
} // namespace OHOS