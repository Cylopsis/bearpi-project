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

#include "napi_state_registry.h"

#include "string_ex.h"
#include "telephony_state_manager.h"
#include "event_listener.h"
#include "napi_telephony_observer.h"
#include "telephony_log_wrapper.h"
#include "core_manager.h"

namespace OHOS {
namespace Telephony {
constexpr int BUF_SIZE = 31;
constexpr int EVENT_TYPE_LENGTH = 32;
static std::unique_ptr<TelephonyStateManager> g_telephonyStateManager;
static std::list<EventListener> g_eventListenerList;
static uint32_t g_eventMasks = 0;
static bool InitTelephonyStateManager()
{
    if (g_telephonyStateManager == nullptr) {
        g_telephonyStateManager = std::make_unique<TelephonyStateManager>();
    }
    return g_telephonyStateManager != nullptr;
}

static int32_t GetDefaultSlotId()
{
    return CoreManager::DEFAULT_SLOT_ID;
}

static void AddStateObserver(uint32_t eventType, int32_t slotId, const std::u16string &package)
{
    if ((eventType & g_eventMasks) == 0) {
        g_eventMasks += eventType;
        sptr<TelephonyObserverBroker> observer = new NapiTelephonyObserver(eventType, g_eventListenerList);
        if ((observer != nullptr) && InitTelephonyStateManager()) {
            g_telephonyStateManager->AddStateObserver(observer, slotId, eventType, package, false);
            TELEPHONY_LOGD("Exec ObserverOnce AddStateObserver");
        }
    }
}

static void HasSlotIdProperty(
    bool hasSlotIdProperty, napi_env env, int32_t slotId, const std::string &slotIdStr, napi_value value)
{
    if (hasSlotIdProperty) {
        napi_value propertyName = nullptr;
        napi_value slotIdValue = nullptr;
        char *nameChars = (char *)slotIdStr.data();
        napi_create_string_utf8(env, nameChars, std::strlen(nameChars), &propertyName);
        napi_get_property(env, value, propertyName, &slotIdValue);
        napi_get_value_int32(env, slotIdValue, &(slotId));
    }
}

static napi_value ObserverOn(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 3;
    napi_value parameters[3] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    if (parameterCount == 2) {
        NAPI_ASSERT(env, NapiTelephonyObserver::MatchParameters(env, parameters, {napi_string, napi_function}),
            "type mismatch");
    } else if (parameterCount == 3) {
        NAPI_ASSERT(env,
            NapiTelephonyObserver::MatchParameters(env, parameters, {napi_string, napi_object, napi_function}),
            "type mismatch");
    } else {
        NAPI_ASSERT(env, false, "type mismatch");
    }
    napi_ref callbackRef = nullptr;
    char eventTypeChars[EVENT_TYPE_LENGTH];
    size_t eventTypeCharsSize;
    int32_t slotId = GetDefaultSlotId();
    napi_get_value_string_utf8(env, parameters[0], eventTypeChars, BUF_SIZE, &eventTypeCharsSize);
    std::string eventTypeStr(eventTypeChars, eventTypeCharsSize);
    if (parameterCount == 2) {
        napi_create_reference(env, parameters[1], 1, &callbackRef);
    } else if (parameterCount == 3) {
        bool hasSlotIdProperty = false;
        std::string slotIdStr = "slotId";
        napi_has_named_property(env, parameters[1], slotIdStr.data(), &hasSlotIdProperty);
        HasSlotIdProperty(hasSlotIdProperty, env, slotId, slotIdStr, parameters[1]);
        napi_create_reference(env, parameters[2], 1, &callbackRef);
    }
    napi_value result = nullptr;
    uint32_t eventType = NapiTelephonyObserver::GetEventType(eventTypeStr);
    if (eventType != NONE_EVENT_TYPE) {
        EventListener listener = {env, eventType, false, thisVar, callbackRef, slotId};
        g_eventListenerList.push_back(listener);
    }
    std::string package("telephony.state_registry.on");
    std::u16string packageForU16 = Str8ToStr16(package);
    AddStateObserver(eventType, slotId, packageForU16);
    return result;
}

static napi_value ObserverOnce(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 3;
    napi_value parameters[3] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    if (parameterCount == 2) {
        NAPI_ASSERT(env, NapiTelephonyObserver::MatchParameters(env, parameters, {napi_string, napi_function}),
            "type mismatch");
    } else if (parameterCount == 3) {
        NAPI_ASSERT(env,
            NapiTelephonyObserver::MatchParameters(env, parameters, {napi_string, napi_object, napi_function}),
            "type mismatch");
    } else {
        NAPI_ASSERT(env, false, "type mismatch");
    }
    napi_ref callbackRef = nullptr;
    char eventTypeChars[EVENT_TYPE_LENGTH];
    size_t eventTypeCharsSize;
    int32_t slotId = GetDefaultSlotId();
    napi_get_value_string_utf8(env, parameters[0], eventTypeChars, BUF_SIZE, &eventTypeCharsSize);
    if (parameterCount == 2) {
        napi_create_reference(env, parameters[1], 1, &callbackRef);
    } else if (parameterCount == 3) {
        bool hasSlotIdProperty = false;
        std::string slotIdStr = "slotId";
        napi_has_named_property(env, parameters[1], slotIdStr.data(), &hasSlotIdProperty);
        HasSlotIdProperty(hasSlotIdProperty, env, slotId, slotIdStr, parameters[1]);
        napi_create_reference(env, parameters[2], 1, &callbackRef);
    }
    napi_value result = nullptr;
    uint32_t eventType = NapiTelephonyObserver::GetEventType(eventTypeChars);
    if (eventType != NONE_EVENT_TYPE) {
        EventListener listener = {env, eventType, true, thisVar, callbackRef, slotId};
        g_eventListenerList.push_back(listener);
        TELEPHONY_LOGD("Exec ObserverOnce after push_back size = %{public}zu", g_eventListenerList.size());
    }
    std::string package("telephony.test.once");
    std::u16string packageForU16 = Str8ToStr16(package);
    AddStateObserver(eventType, slotId, packageForU16);
    return result;
}

static napi_value ObserverOff(napi_env env, napi_callback_info info)
{
    size_t parameterCount = 3;
    napi_value parameters[3] = {0};
    napi_value thisVar = nullptr;
    void *data = nullptr;
    napi_get_cb_info(env, info, &parameterCount, parameters, &thisVar, &data);
    if (parameterCount == 1) {
        NAPI_ASSERT(env, NapiTelephonyObserver::MatchParameters(env, parameters, {napi_string}), "type mismatch");
    } else if (parameterCount == 2) {
        NAPI_ASSERT(env, NapiTelephonyObserver::MatchParameters(env, parameters, {napi_string, napi_function}),
            "type mismatch");
    } else {
        NAPI_ASSERT(env, false, "type mismatch");
    }
    napi_value result = nullptr;
    char eventTypeChars[EVENT_TYPE_LENGTH];
    size_t eventTypeCharsSize;
    napi_get_value_string_utf8(env, parameters[0], eventTypeChars, BUF_SIZE, &eventTypeCharsSize);
    int32_t eventType = NapiTelephonyObserver::GetEventType(eventTypeChars);

    if (eventType == NONE_EVENT_TYPE) {
        return result;
    }

    if (InitTelephonyStateManager()) {
        for (std::list<EventListener>::iterator it = g_eventListenerList.begin(); it != g_eventListenerList.end();
             it++) {
            if (it->eventType == eventType) {
                g_telephonyStateManager->RemoveStateObserver(it->slotId, it->eventType);
                TELEPHONY_LOGD("Exec ObserverOff after RemoveStateObserver eventType = %{public}d", it->eventType);
            }
        }
    }
    g_eventListenerList.remove_if(
        [eventType](EventListener listener) -> bool { return listener.eventType == eventType; });

    return result;
}

EXTERN_C_START
napi_value InitNapiStateRegistry(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("on", ObserverOn),
        DECLARE_NAPI_FUNCTION("once", ObserverOnce),
        DECLARE_NAPI_FUNCTION("off", ObserverOff),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
    return exports;
}
EXTERN_C_END

static napi_module _stateRegistryModule = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = nullptr,
    .nm_register_func = InitNapiStateRegistry,
    .nm_modname = "telephony.observer",
    .nm_priv = ((void *)0),
    .reserved = {(void *)0},
};

extern "C" __attribute__((constructor)) void RegisterTelephonyObserverModule(void)
{
    napi_module_register(&_stateRegistryModule);
}
} // namespace Telephony
} // namespace OHOS
