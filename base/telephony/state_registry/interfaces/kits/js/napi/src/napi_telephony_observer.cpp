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

#include "napi_telephony_observer.h"
#include "i_sim_state_manager.h"
#include "napi_sim_type.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
napi_value CreateErrorMessage(napi_env env, std::string msg)
{
    napi_value result = nullptr;
    napi_value message = nullptr;
    napi_create_string_utf8(env, (char *)msg.data(), msg.size(), &message);
    napi_create_error(env, nullptr, message, &result);
    return result;
}

napi_value CreateUndefined(napi_env env)
{
    napi_value result = nullptr;
    napi_get_undefined(env, &result);
    return result;
}

NapiTelephonyObserver::NapiTelephonyObserver(int32_t eventType, std::list<EventListener> &listenerList)
{
    eventType_ = eventType;
    listenerList_ = &listenerList;
}

void NapiTelephonyObserver::SetPropertyBoolean(napi_env env, napi_value object, std::string name, bool value)
{
    napi_value propertyValue = nullptr;
    napi_get_boolean(env, value, &propertyValue);
    char *nameChars = (char *)name.data();
    napi_set_named_property(env, object, nameChars, propertyValue);
}

int32_t NapiTelephonyObserver::WrapSimState(int32_t simState)
{
    switch (simState) {
        case ExternalState::EX_READY:
            return static_cast<int32_t>(SimState::SIM_STATE_READY);
        case ExternalState::EX_PIN_LOCKED:
        case ExternalState::EX_PUK_LOCKED:
        case ExternalState::EX_SIMLOCK:
        case ExternalState::EX_BLOCKED_PERM:
            return static_cast<int32_t>(SimState::SIM_STATE_LOCKED);
        case ExternalState::EX_ABSENT:
            return static_cast<int32_t>(SimState::SIM_STATE_NOT_PRESENT);
        case ExternalState::EX_UNREADY:
        case ExternalState::EX_ICC_ERROR:
        case ExternalState::EX_ICC_RESTRICTED:
            return static_cast<int32_t>(SimState::SIM_STATE_NOT_READY);
        default:
            return static_cast<int32_t>(SimState::SIM_STATE_UNKNOWN);
    }
}

void NapiTelephonyObserver::OnSimStateUpdated(int32_t state, const std::u16string &reason)
{
    for (auto iter = listenerList_->begin(); iter != listenerList_->end(); iter++) {
        if (iter->eventType == LISTEN_SIM_STATE) {
            napi_env env = iter->env;
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(env, &scope);
            napi_value thisVar = iter->thisVar;
            napi_ref callbackRef = iter->callbackRef;
            napi_value callbackFunc = nullptr;
            napi_get_reference_value(env, callbackRef, &callbackFunc);
            napi_value callbackResult = nullptr;
            napi_value callbackValues[2] = {0};
            int32_t wrappedSimState = WrapSimState(state);
            TELEPHONY_LOGD("OnSimStateUpdated eventType %{public}d", wrappedSimState);
            if (wrappedSimState >= static_cast<int32_t>(SimState::SIM_STATE_UNKNOWN)) {
                callbackValues[0] = CreateUndefined(env);
                napi_create_object(env, &callbackValues[1]);
                SetPropertyInt32(env, callbackValues[1], "state", wrappedSimState);
                SetPropertyStringUtf8(env, callbackValues[1], "reason", ToUtf8(reason));
            } else {
                callbackValues[0] = CreateErrorMessage(env, "get data error");
                callbackValues[1] = CreateUndefined(env);
            }
            napi_call_function(env, thisVar, callbackFunc, 2, callbackValues, &callbackResult);
            napi_close_handle_scope(env, scope);
        }
    }
    listenerList_->remove_if(
        [](EventListener listener) -> bool { return listener.isOnce && listener.eventType == LISTEN_SIM_STATE; });
}

void NapiTelephonyObserver::OnCallStateUpdated(int32_t callState, const std::u16string &phoneNumber)
{
    for (std::list<EventListener>::iterator listenerIterator = listenerList_->begin();
         listenerIterator != listenerList_->end(); ++listenerIterator) {
        if (listenerIterator->eventType == LISTEN_CALL_STATE) {
            napi_env env = listenerIterator->env;
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(env, &scope);
            napi_value thisVar = listenerIterator->thisVar;
            napi_ref callbackRef = listenerIterator->callbackRef;
            napi_value callbackFunc = nullptr;
            napi_get_reference_value(env, callbackRef, &callbackFunc);
            napi_value callbackResult = nullptr;
            std::string phoneNumberStr = ToUtf8(phoneNumber);
            napi_value callbackValues[2] = {0};
            int32_t wrappedCallState = WrapCallState(callState);
            if (wrappedCallState >= static_cast<int32_t>(CallState::CALL_STATE_UNKNOWN)) {
                callbackValues[0] = CreateUndefined(env);
                napi_create_object(env, &callbackValues[1]);
                SetPropertyInt32(env, callbackValues[1], "state", wrappedCallState);
                SetPropertyStringUtf8(env, callbackValues[1], "number", phoneNumberStr);
            } else {
                callbackValues[0] = CreateErrorMessage(env, "get data error");
                callbackValues[1] = CreateUndefined(env);
            }
            napi_call_function(env, thisVar, callbackFunc, 2, callbackValues, &callbackResult);
            napi_close_handle_scope(env, scope);
        }
    }
    listenerList_->remove_if(
        [](EventListener listener) -> bool { return listener.isOnce && listener.eventType == LISTEN_CALL_STATE; });
}

void NapiTelephonyObserver::OnSignalInfoUpdated(const std::vector<sptr<SignalInformation>> &signalInfoList)
{
    for (std::list<EventListener>::iterator listenerIterator = listenerList_->begin();
         listenerIterator != listenerList_->end(); ++listenerIterator) {
        if (listenerIterator->eventType != LISTEN_SIGNAL_STRENGTHS) {
            continue;
        }

        napi_env env = listenerIterator->env;
        napi_handle_scope scope = nullptr;
        napi_open_handle_scope(env, &scope);
        napi_value thisVar = listenerIterator->thisVar;
        napi_ref callbackRef = listenerIterator->callbackRef;
        napi_value callbackFunc = nullptr;
        napi_get_reference_value(env, callbackRef, &callbackFunc);
        int listSize = signalInfoList.size();
        TELEPHONY_LOGD("OnSignalInfoUpdated eventType == LISTEN_SIGNAL_STRENGTHS listSize = %d", listSize);
        napi_value callbackValue[2] = {0};
        if (listSize > 0) {
            callbackValue[0] = CreateUndefined(env);
            napi_create_array(env, &callbackValue[1]);
            for (int i = 0; i < listSize; ++i) {
                sptr<SignalInformation> infoItem = signalInfoList[i];
                napi_value info = nullptr;
                napi_create_object(env, &info);
                SetPropertyInt32(env, info, "signalType", WrapNetworkType(infoItem->GetNetworkType()));
                SetPropertyInt32(env, info, "signalLevel", infoItem->GetSignalLevel());
                napi_set_element(env, callbackValue[1], i, info);
            }
        } else {
            callbackValue[0] = CreateErrorMessage(env, "get signal info list failed");
            callbackValue[1] = CreateUndefined(env);
        }
        napi_value callbackResult = nullptr;
        napi_call_function(env, thisVar, callbackFunc, 2, callbackValue, &callbackResult);
        napi_close_handle_scope(env, scope);
    }
    listenerList_->remove_if([](EventListener listener) -> bool {
        return listener.isOnce && listener.eventType == LISTEN_SIGNAL_STRENGTHS;
    });
}

void NapiTelephonyObserver::OnNetworkStateUpdated(const sptr<NetworkState> &networkState)
{
    for (std::list<EventListener>::iterator listenerIterator = listenerList_->begin();
         listenerIterator != listenerList_->end(); ++listenerIterator) {
        if (listenerIterator->eventType == LISTEN_NET_WORK_STATE) {
            napi_env env = listenerIterator->env;
            napi_handle_scope scope = nullptr;
            napi_open_handle_scope(env, &scope);
            napi_value thisVar = listenerIterator->thisVar;
            napi_ref callbackRef = listenerIterator->callbackRef;
            napi_value callbackFunc = nullptr;
            napi_get_reference_value(env, callbackRef, &callbackFunc);
            napi_value callbackValues[2] = {0};
            if (networkState != nullptr) {
                callbackValues[0] = CreateUndefined(env);
                napi_value object = nullptr;
                napi_create_object(env, &object);
                SetPropertyStringUtf8(env, object, "longOperatorName", networkState->GetLongOperatorName());
                SetPropertyStringUtf8(env, object, "shortOperatorName", networkState->GetShortOperatorName());
                SetPropertyStringUtf8(env, object, "plmnNumeric", networkState->GetPlmnNumeric());
                SetPropertyInt32(env, object, "isRoaming", networkState->IsRoaming() ? 1 : 0);
                SetPropertyInt32(env, object, "regStatus", networkState->GetRegStatus());
                SetPropertyInt32(env, object, "isEmergency", networkState->IsEmergency());
                callbackValues[1] = object;
            } else {
                callbackValues[0] = CreateErrorMessage(env, "networkState data error");
                callbackValues[1] = CreateUndefined(env);
            }
            napi_value callbackResult = nullptr;
            napi_call_function(env, thisVar, callbackFunc, 2, callbackValues, &callbackResult);
            napi_close_handle_scope(env, scope);
        }
    }
    listenerList_->remove_if([](EventListener listener) -> bool {
        return listener.isOnce && listener.eventType == LISTEN_NET_WORK_STATE;
    });
}

std::string NapiTelephonyObserver::ToUtf8(std::u16string str16)
{
    return std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> {}.to_bytes(str16);
}

int32_t NapiTelephonyObserver::WrapRadioTech(RadioTech radioTechType)
{
    switch (radioTechType) {
        case RADIO_TECHNOLOGY_GSM: {
            return RADIO_TECHNOLOGY_GSM;
        }
        case RADIO_TECHNOLOGY_LTE: {
            return RADIO_TECHNOLOGY_LTE;
        }
        default: {
            return RADIO_TECHNOLOGY_UNKNOWN;
        }
    }
}

int32_t NapiTelephonyObserver::WrapNetworkType(SignalInformation::NetworkType networkType)
{
    int type = static_cast<int>(networkType);
    NetworkType wrapNetworkType = NetworkType::NETWORK_TYPE_UNKNOWN;
    switch (type) {
        case GSM: {
            wrapNetworkType = NetworkType::NETWORK_TYPE_GSM;
            break;
        }
        case LTE: {
            wrapNetworkType = NetworkType::NETWORK_TYPE_LTE;
            break;
        }
        default: {
            break;
        }
    }
    return static_cast<int32_t>(wrapNetworkType);
}

int32_t NapiTelephonyObserver::WrapCallState(int32_t callState)
{
    switch (callState) {
        case Telephony::CALL_STATUS_ACTIVE:
        case Telephony::CALL_STATUS_HOLDING:
        case Telephony::CALL_STATUS_DIALING:
        case Telephony::CALL_STATUS_ALERTING:
        case Telephony::CALL_STATUS_DISCONNECTING:
            return static_cast<int32_t>(CallState::CALL_STATE_OFFHOOK);
        case Telephony::CALL_STATUS_WAITING:
        case Telephony::CALL_STATUS_INCOMING:
            return static_cast<int32_t>(CallState::CALL_STATE_RINGING);
        case Telephony::CALL_STATUS_DISCONNECTED:
        case Telephony::CALL_STATUS_IDLE:
            return static_cast<int32_t>(CallState::CALL_STATE_IDLE);
        default:
            return static_cast<int32_t>(CallState::CALL_STATE_UNKNOWN);
    }
}

bool NapiTelephonyObserver::MatchValueType(napi_env env, napi_value value, napi_valuetype targetType)
{
    napi_valuetype valueType = napi_undefined;
    napi_typeof(env, value, &valueType);
    return valueType == targetType;
}

bool NapiTelephonyObserver::MatchParameters(
    napi_env env, const napi_value parameters[], std::initializer_list<napi_valuetype> valueTypes)
{
    int i = 0;
    for (auto beg = valueTypes.begin(); beg != valueTypes.end(); ++beg) {
        if (!MatchValueType(env, parameters[i], *beg)) {
            return false;
        }
        ++i;
    }
    return true;
}

bool NapiTelephonyObserver::MatchEventType(const std::string &type, const std::string &goalTypeStr)
{
    return goalTypeStr.compare(type) == 0;
}

int32_t NapiTelephonyObserver::GetEventType(const std::string &type)
{
    if (MatchEventType(type, NET_WORK_STATE_CHANGE)) {
        return LISTEN_NET_WORK_STATE;
    }
    if (MatchEventType(type, CALL_STATE_CHANGE)) {
        return LISTEN_CALL_STATE;
    }
    if (MatchEventType(type, SIGNAL_STRENGTHS_CHANGE)) {
        return LISTEN_SIGNAL_STRENGTHS;
    }
    if (MatchEventType(type, SIM_STATE_CHANGE)) {
        return LISTEN_SIM_STATE;
    }
    return NONE_EVENT_TYPE;
}

bool NapiTelephonyObserver::HasEventMask(uint32_t eventType, uint32_t masks)
{
    return (eventType & masks) > 0;
}

void NapiTelephonyObserver::SetPropertyInt32(napi_env env, napi_value object, std::string name, int32_t value)
{
    napi_value propertyValue = nullptr;
    napi_create_int32(env, value, &propertyValue);
    napi_set_named_property(env, object, name.c_str(), propertyValue);
}

void NapiTelephonyObserver::SetPropertyStringUtf8(
    napi_env env, napi_value object, std::string name, std::string value)
{
    napi_value propertyValue = nullptr;
    char *valueChars = (char *)value.c_str();
    napi_create_string_utf8(env, valueChars, std::strlen(valueChars), &propertyValue);
    napi_set_named_property(env, object, name.c_str(), propertyValue);
}
} // namespace Telephony
} // namespace OHOS