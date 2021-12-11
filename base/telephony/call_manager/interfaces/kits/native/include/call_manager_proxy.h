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

#ifndef CALL_MANAGER_PROXY_H
#define CALL_MANAGER_PROXY_H

#include <mutex>

#include "if_system_ability_manager.h"
#include "refbase.h"
#include "rwlock.h"
#include "singleton.h"

#include "timer.h"
#include "i_call_manager_service.h"
#include "i_call_ability_callback.h"

namespace OHOS {
namespace Telephony {
class CallManagerProxy : public Timer, public std::enable_shared_from_this<CallManagerProxy> {
    DECLARE_DELAYED_SINGLETON(CallManagerProxy)
public:
    void Init(int32_t systemAbilityId);
    void UnInit();
    static void task();

    int32_t DialCall(std::u16string number, AppExecFwk::PacMap &extras);
    int32_t AnswerCall(int32_t callId, int32_t videoState);
    int32_t RejectCall(int32_t callId, bool isSendSms, std::u16string content);
    int32_t HangUpCall(int32_t callId);
    int32_t GetCallState();
    int32_t HoldCall(int32_t callId);
    int32_t UnHoldCall(int32_t callId);
    int32_t SwitchCall(int32_t callId);
    int32_t CombineConference(int32_t callId);
    int32_t GetMainCallId(int32_t &callId);
    std::vector<std::u16string> GetSubCallIdList(int32_t callId);
    std::vector<std::u16string> GetCallIdListForConference(int32_t callId);
    int32_t GetCallWaiting(int32_t slotId);
    int32_t SetCallWaiting(int32_t slotId, bool activate);
    int32_t StartDtmf(int32_t callId, char str);
    int32_t StopDtmf(int32_t callId);
    int32_t SendDtmf(int32_t callId, char str);
    int32_t SendBurstDtmf(int32_t callId, std::u16string str, int32_t on, int32_t off);
    bool IsRinging();
    bool HasCall();
    bool IsNewCallAllowed();
    bool IsInEmergencyCall();
    bool IsEmergencyPhoneNumber(std::u16string &number, int32_t slotId, int32_t &errorCode);
    int32_t FormatPhoneNumber(std::u16string &number, std::u16string &countryCode, std::u16string &formatNumber);
    int32_t FormatPhoneNumberToE164(
        std::u16string &number, std::u16string &countryCode, std::u16string &formatNumber);

private:
    int32_t ConnectService();
    int32_t RegisterCallBack();
    void DisconnectService();
    int32_t ReConnectService();
    void OnDeath();
    void Clean();
    void NotifyDeath();

private:
    int32_t systemAbilityId_;
    sptr<ICallManagerService> callManagerServicePtr_;
    sptr<ICallAbilityCallback> callAbilityCallbackPtr_;
    Utils::RWLock rwClientLock_;
    std::mutex mutex_;
};
} // namespace Telephony
} // namespace OHOS

#endif