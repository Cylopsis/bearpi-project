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

#include "hang_up_sms.h"

#include "iservice_registry.h"
#include "system_ability.h"
#include "system_ability_definition.h"

#include "telephony_log_wrapper.h"

#include "call_manager_errors.h"

namespace OHOS {
namespace Telephony {
HangUpSms::HangUpSms() : smsService_(nullptr), msgManager_(std::make_unique<ShortMessageManager>()) {}

HangUpSms::~HangUpSms()
{
    if (smsService_ != nullptr) {
        smsService_ = nullptr;
    }
}

void HangUpSms::IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content)
{
    TELEPHONY_LOGD("start to send reject message...");
    if (callObjectPtr == nullptr || !isSendSms || content.empty() || callObjectPtr->GetAccountNumber().empty()) {
        return;
    }
    SendMessage(DEFAULT_SLOT_ID, callObjectPtr->GetAccountNumber(), content);
}

void HangUpSms::SendMessage(int32_t slotId, const std::string &desAddr, const std::string &content)
{
    TELEPHONY_LOGD("SendMessage...");
    SendMessage(slotId, ConvertToUtf16(desAddr), ConvertToUtf16(content));
}

void HangUpSms::SendMessage(int32_t slotId, const std::u16string &desAddr, const std::u16string &text)
{
    if (msgManager_ == nullptr) {
        TELEPHONY_LOGE("short message manager nullptr");
        return;
    }
    msgManager_->SendMessage(slotId, desAddr, ConvertToUtf16(""), text, nullptr, nullptr);
    TELEPHONY_LOGD("send reject message success");
}

std::u16string HangUpSms::ConvertToUtf16(const std::string &str)
{
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> converter;
    return converter.from_bytes(str);
}

bool HangUpSms::IsSmsAbilityExist()
{
    std::lock_guard<std::mutex> lock(mutex_);
    sptr<ISystemAbilityManager> sysAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!sysAbilityMgr) {
        TELEPHONY_LOGE("system ability manager nullptr");
        return false;
    }
    sptr<IRemoteObject> remote = sysAbilityMgr->CheckSystemAbility(TELEPHONY_SMS_MMS_SYS_ABILITY_ID);
    if (!remote) {
        TELEPHONY_LOGE("no sms ability");
        return false;
    }
    return true;
}

int32_t HangUpSms::ConnectSmsService()
{
    std::lock_guard<std::mutex> lock(mutex_);
    sptr<OHOS::ISystemAbilityManager> smsManager =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (smsManager == nullptr) {
        return TELEPHONY_FAIL;
    }
    sptr<IRemoteObject> object = smsManager->GetSystemAbility(TELEPHONY_SMS_MMS_SYS_ABILITY_ID);
    if (object != nullptr) {
        smsService_ = iface_cast<ISmsServiceInterface>(object);
    } else {
        return TELEPHONY_FAIL;
    }
    if (smsService_ == nullptr) {
        TELEPHONY_LOGE("sms service interface nullptr");
        return TELEPHONY_FAIL;
    }
    return TELEPHONY_SUCCESS;
}

void HangUpSms::NewCallCreated(sptr<CallBase> &callObjectPtr) {}

void HangUpSms::CallDestroyed(sptr<CallBase> &callObjectPtr) {}

void HangUpSms::IncomingCallActivated(sptr<CallBase> &callObjectPtr) {}

void HangUpSms::CallStateUpdated(sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState) {}
} // namespace Telephony
} // namespace OHOS