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

#ifndef NAPI_CALL_MANAGER_H
#define NAPI_CALL_MANAGER_H

#include "pac_map.h"
#include "string_ex.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "napi_call_manager_types.h"

namespace OHOS {
namespace Telephony {
class NapiCallManager {
public:
    NapiCallManager();
    ~NapiCallManager() = default;

    static napi_value CallManagerEnumTypeInit(napi_env env, napi_value exports);
    static napi_value RegisterCallManagerFunc(napi_env env, napi_value exports);
    static napi_value DialCall(napi_env env, napi_callback_info info);
    static napi_value AnswerCall(napi_env env, napi_callback_info info);
    static napi_value RejectCall(napi_env env, napi_callback_info info);
    static napi_value HangUpCall(napi_env env, napi_callback_info info);
    static napi_value HoldCall(napi_env env, napi_callback_info info);
    static napi_value UnHoldCall(napi_env env, napi_callback_info info);
    static napi_value SwitchCall(napi_env env, napi_callback_info info);
    static napi_value CombineConference(napi_env env, napi_callback_info info);
    static napi_value GetMainCallId(napi_env env, napi_callback_info info);
    static napi_value GetSubCallIdList(napi_env env, napi_callback_info info);
    static napi_value GetCallIdListForConference(napi_env env, napi_callback_info info);
    static napi_value GetCallWaiting(napi_env env, napi_callback_info info);
    static napi_value SetCallWaiting(napi_env env, napi_callback_info info);
    static napi_value StartDtmf(napi_env env, napi_callback_info info);
    static napi_value StopDtmf(napi_env env, napi_callback_info info);
    static napi_value SendDtmf(napi_env env, napi_callback_info info);
    static napi_value SendBurstDtmf(napi_env env, napi_callback_info info);
    static napi_value GetCallState(napi_env env, napi_callback_info info);
    static napi_value IsRinging(napi_env env, napi_callback_info info);
    static napi_value HasCall(napi_env env, napi_callback_info info);
    static napi_value IsNewCallAllowed(napi_env env, napi_callback_info info);
    static napi_value IsInEmergencyCall(napi_env env, napi_callback_info info);
    static napi_value IsEmergencyPhoneNumber(napi_env env, napi_callback_info info);
    static napi_value FormatPhoneNumber(napi_env env, napi_callback_info info);
    static napi_value FormatPhoneNumberToE164(napi_env env, napi_callback_info info);
    static napi_value ObserverOn(napi_env env, napi_callback_info info);
    static napi_value ObserverOff(napi_env env, napi_callback_info info);

private:
    static bool MatchValueType(napi_env env, napi_value value, napi_valuetype targetType);
    static napi_value CreateUndefined(napi_env env);
    static napi_value CreateErrorMessage(napi_env env, std::string msg);
    static void NativeCallBack(napi_env env, napi_status status, void *data);
    static void NativeDialCallBack(napi_env env, napi_status status, void *data);
    static void NativeVoidCallBack(napi_env env, napi_status status, void *data);
    static void NativePropertyCallBack(napi_env env, napi_status status, void *data);
    static void NativeBoolCallBack(napi_env env, napi_status status, void *data);
    static void NativeFormatNumberCallBack(napi_env env, napi_status status, void *data);
    static void NativeListCallBack(napi_env env, napi_status status, void *data);
    static void GetDialInfo(napi_env env, napi_value objValue, DialAsyncContext &asyncContext);
    static void GetDtmfBunchInfo(napi_env env, napi_value objValue, DtmfAsyncContext &asyncContext);
    static void GetRestrictionInfo(napi_env env, napi_value objValue, SupplementAsyncContext &asyncContext);
    static void GetTransferInfo(napi_env env, napi_value objValue, SupplementAsyncContext &asyncContext);
    static void NativeDialCall(napi_env env, void *data);
    static void NativeAnswerCall(napi_env env, void *data);
    static void NativeRejectCall(napi_env env, void *data);
    static void NativeHangUpCall(napi_env env, void *data);
    static void NativeHoldCall(napi_env env, void *data);
    static void NativeUnHoldCall(napi_env env, void *data);
    static void NativeSwitchCall(napi_env env, void *data);
    static void NativeCombineConference(napi_env env, void *data);
    static void NativeGetMainCallId(napi_env env, void *data);
    static void NativeGetSubCallIdList(napi_env env, void *data);
    static void NativeGetCallIdListForConference(napi_env env, void *data);
    static void NativeGetCallWaiting(napi_env env, void *data);
    static void NativeSetCallWaiting(napi_env env, void *data);
    static void NativeStartDtmf(napi_env env, void *data);
    static void NativeStopDtmf(napi_env env, void *data);
    static void NativeSendDtmf(napi_env env, void *data);
    static void NativeSendDtmfBunch(napi_env env, void *data);
    static void NativeGetCallState(napi_env env, void *data);
    static void NativeIsRinging(napi_env env, void *data);
    static void NativeHasCall(napi_env env, void *data);
    static void NativeIsNewCallAllowed(napi_env env, void *data);
    static void NativeIsInEmergencyCall(napi_env env, void *data);
    static void NativeIsEmergencyPhoneNumber(napi_env env, void *data);
    static void NativeFormatPhoneNumber(napi_env env, void *data);
    static void NativeFormatPhoneNumberToE164(napi_env env, void *data);
    static napi_value ToInt32Value(napi_env env, int32_t value);
    static napi_value GetNamedProperty(napi_env env, napi_value object, const std::string &propertyName);
    static std::string GetStringProperty(napi_env env, napi_value object, const std::string &propertyName);
    static int32_t GetIntProperty(napi_env env, napi_value object, const std::string &propertyName);
    static void GetSmsInfo(napi_env env, napi_value objValue, RejectAsyncContext &asyncContext);
    static napi_value HandleAsyncWork(napi_env env, AsyncContext *context, std::string workName,
        napi_async_execute_callback execute, napi_async_complete_callback complete);
    static bool IsValidSlotId(int32_t slotId);
};
} // namespace Telephony
} // namespace OHOS

#endif // NAPI_CALL_MANAGER_H