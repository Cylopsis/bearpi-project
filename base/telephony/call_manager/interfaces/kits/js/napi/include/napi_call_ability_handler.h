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

#ifndef NAPI_CALL_ABILITY_HANDLER_H
#define NAPI_CALL_ABILITY_HANDLER_H

#include <mutex>

#include "pac_map.h"
#include "event_handler.h"
#include "event_runner.h"
#include "singleton.h"

#include "napi_call_manager_types.h"
#include "napi_call_ability_callback.h"

namespace OHOS {
namespace Telephony {
class NapiCallAbilityHandler : public AppExecFwk::EventHandler {
public:
    NapiCallAbilityHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);
    virtual ~NapiCallAbilityHandler();
    void Init();
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event);

private:
    std::shared_ptr<NapiCallAbilityCallback> napiCallAbilityCallbackPtr_;
};

class NapiCallAbilityHandlerService : public std::enable_shared_from_this<NapiCallAbilityHandlerService> {
    DECLARE_DELAYED_SINGLETON(NapiCallAbilityHandlerService)
public:
    void Start();
    int32_t UpdateCallStateInfoHandler(const CallAttributeInfo &info);
    int32_t UpdateCallEventHandler(const CallEventInfo &info);
    int32_t UpdateSupplementInfoHandler(CallResultReportId reportId, AppExecFwk::PacMap &resultInfo);

    enum NapiCallAbilityInterfaceType {
        NAPI_HANDLER_UPDATE_CALL_INFO = 0,
        NAPI_HANDLER_UPDATE_CALL_EVENT,
        NAPI_HANDLER_UPDATE_SUPPLEMENT,
    };

private:
    std::shared_ptr<AppExecFwk::EventRunner> eventLoop_;
    std::shared_ptr<NapiCallAbilityHandler> handler_;
    std::mutex mutex_;
};
} // namespace Telephony
} // namespace OHOS

#endif // NAPI_CALL_ABILITY_HANDLER_H