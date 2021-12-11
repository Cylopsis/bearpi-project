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

#ifndef NAPI_TELEPHONY_OBSERVER_H
#define NAPI_TELEPHONY_OBSERVER_H

#include <codecvt>
#include <cstring>
#include <list>
#include <locale>
#include <memory>
#include <string>

#include "call_manager_inner_type.h"
#include "cellular_data_types.h"
#include "event_listener.h"
#include "napi_state_registry.h"
#include "signal_information.h"
#include "telephony_observer.h"

namespace OHOS {
namespace Telephony {
class NapiTelephonyObserver : public TelephonyObserver {
public:
    NapiTelephonyObserver(int32_t eventType, std::list<EventListener> &eventListener);
    ~NapiTelephonyObserver() = default;
    void OnCallStateUpdated(int32_t callState, const std::u16string &phoneNumber) override;
    void OnSignalInfoUpdated(const std::vector<sptr<SignalInformation>> &vec) override;
    void OnNetworkStateUpdated(const sptr<NetworkState> &networkState) override;
    void OnSimStateUpdated(int32_t state, const std::u16string &reason) override;
    bool HasEventMask(uint32_t eventType, uint32_t masks);
    static bool MatchEventType(const std::string &type, const std::string &goalTypeStr);
    static void SetPropertyInt32(napi_env env, napi_value object, std::string name, int32_t value);
    static void SetPropertyStringUtf8(napi_env env, napi_value object, std::string name, std::string value);
    static bool MatchValueType(napi_env env, napi_value value, napi_valuetype targetType);
    static bool MatchParameters(
        napi_env env, const napi_value parameters[], std::initializer_list<napi_valuetype> valueTypes);
    static int32_t GetEventType(const std::string &type);

private:
    int32_t eventType_;
    std::list<EventListener> *listenerList_;
    std::string ToUtf8(std::u16string str16);
    int32_t WrapRadioTech(RadioTech radioTechType);
    int32_t WrapNetworkType(SignalInformation::NetworkType tech);
    int32_t WrapCallState(int32_t callState);
    int32_t WrapSimState(int32_t simState);
    void SetPropertyBoolean(napi_env env, napi_value object, std::string name, bool value);
};
} // namespace Telephony
} // namespace OHOS
#endif // NAPI_TELEPHONY_OBSERVER_H