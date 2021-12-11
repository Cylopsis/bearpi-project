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

#ifndef REPORT_CALL_STATE_CLIENT_H
#define REPORT_CALL_STATE_CLIENT_H

#include "if_system_ability_manager.h"
#include "rwlock.h"
#include "singleton.h"

#include "i_telephony_state_notify.h"

#include "timer.h"

namespace OHOS {
namespace Telephony {
class CallStateReportProxy : public Timer, public std::enable_shared_from_this<CallStateReportProxy> {
    DECLARE_DELAYED_SINGLETON(CallStateReportProxy)
public:
    int32_t Init(int32_t systemAbilityId);
    void UnInit();
    static void task();
    int32_t ReportCallState(int32_t callState, std::u16string phoneNumber);
    int32_t ReportCallStateForCallId(
        int32_t subId, int32_t callId, int32_t callState, std::u16string incomingNumber);

private:
    int32_t ConnectService();
    void DisconnectService();
    void ReConnectService();
    void OnDeath();
    void Clean();
    void NotifyDeath();

private:
    int32_t systemAbilityId_;
    sptr<ITelephonyStateNotify> telephonyStateNotifyPtr_;
    sptr<IRemoteObject::DeathRecipient> stateRegistryRecipient_;
    bool connectState_;
    Utils::RWLock rwClientLock_;
    std::mutex mutex_;
};
} // namespace Telephony
} // namespace OHOS

#endif // REPORT_CALL_STATE_CLIENT_H
