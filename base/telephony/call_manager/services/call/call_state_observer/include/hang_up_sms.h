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

#ifndef HANG_UP_SMS_H
#define HANG_UP_SMS_H

#include <mutex>

#include "call_state_listener_base.h"

#include "i_sms_service_interface.h"
#include "i_delivery_short_message_callback.h"
#include "i_send_short_message_callback.h"
#include "short_message_manager.h"

namespace OHOS {
namespace Telephony {
class HangUpSms : public CallStateListenerBase {
public:
    HangUpSms();
    virtual ~HangUpSms();
    void NewCallCreated(sptr<CallBase> &callObjectPtr) override;
    void CallDestroyed(sptr<CallBase> &callObjectPtr) override;
    void IncomingCallActivated(sptr<CallBase> &callObjectPtr) override;
    void IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content) override;
    void CallStateUpdated(sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState) override;

private:
    std::mutex mutex_;
    const int32_t DEFAULT_SLOT_ID = 0;
    sptr<ISmsServiceInterface> smsService_;
    std::unique_ptr<ShortMessageManager> msgManager_;
    bool IsSmsAbilityExist();
    int32_t ConnectSmsService();
    void SendMessage(int32_t slotId, const std::string &desAddr, const std::string &text);
    void SendMessage(int32_t slotId, const std::u16string &desAddr, const std::u16string &text);
    int32_t GetSlotId() const;
    std::u16string ConvertToUtf16(const std::string &str);
};
} // namespace Telephony
} // namespace OHOS
#endif // HANG_UP_SMS_H