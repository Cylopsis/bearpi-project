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

#include "missed_call_notification.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "telephony_log_wrapper.h"

#ifdef ABILITY_NOTIFICATION_SUPPORT
#include "ans_notification.h"
#endif

namespace OHOS {
namespace Telephony {
MissedCallNotification::MissedCallNotification()
{
    Init();
}

MissedCallNotification::~MissedCallNotification()
{
#ifdef ABILITY_NOTIFICATION_SUPPORT
    if (ansAbility_ != nullptr) {
        ansAbility_ = nullptr;
    }
    if (ansManagerProxy_ != nullptr) {
        ansManagerProxy_ = nullptr;
    }
#endif
}

void MissedCallNotification::Init()
{
#ifdef ABILITY_NOTIFICATION_SUPPORT
    std::lock_guard<std::mutex> lock(mutex_);
    sptr<ISystemAbilityManager> sam = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (sam == nullptr) {
        TELEPHONY_LOGE("system ability manager nullptr");
        return;
    }
    remoteObject_ = sam->GetSystemAbility(ADVANCED_NOTIFICATION_SERVICE_ABILITY_ID); // ans ability
    if (remoteObject_ == nullptr) {
        TELEPHONY_LOGE("ans ability nullptr");
        return;
    }
    ansManagerProxy_ = std::make_unique<AnsManagerProxy>(remoteObject_);
    if (audioManagerProxy_ == nullptr) {
        TELEPHONY_LOGE("ans manager nullptr");
    }
#endif
}

void MissedCallNotification::PublishNotification(const std::string &title, const std::string &text)
{
#ifdef ABILITY_NOTIFICATION_SUPPORT
    if (ansManagerProxy_ == nullptr) {
        TELEPHONY_LOGE("ans manager proxy nullptr");
        return;
    }
    std::unique_ptr<NotificationNormalContent> notificationNormalContent =
        std::make_unique<NotificationNormalContent>();
    sptr<NotificationNormalContent> normalContent = notificationNormalContent.release();
    if (normalContent == nullptr) {
        TELEPHONY_LOGE("ans notification nullptr");
        return;
    }
    normalContent->SetTitle(title);
    normalContent->SetText(text);
    std::unique_ptr<NotificationContent> notificationContent = std::make_unique<NotificationContent>(normalContent);
    sptr<NotificationContent> content = notificationContent.release();
    if (content == nullptr) {
        TELEPHONY_LOGE("ans notification nullptr");
        return;
    }
    std::unique_ptr<AnsNotification> ansNotification = std::make_unique<AnsNotification>();
    sptr<AnsNotification> notification = ansNotification.release();
    if (notification == nullptr) {
        TELEPHONY_LOGE("ans notification nullptr");
        return;
    }
    notification->SetNotificationId(notificationId_++);
    notification->SetContent(content);
    ansManagerProxy_->Publish(notification, nullptr, true);
#endif
}

void MissedCallNotification::CancelNotification(int32_t id)
{
#ifdef ABILITY_NOTIFICATION_SUPPORT
    if (ansManagerProxy_ == nullptr) {
        TELEPHONY_LOGE("ans manager proxy nullptr");
        return;
    }
    ansManagerProxy_->CancelNotification(id);
#endif
}

bool MissedCallNotification::IsAnsAbilityExist()
{
    std::lock_guard<std::mutex> lock(mutex_);
    sptr<ISystemAbilityManager> sysAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!sysAbilityMgr) {
        TELEPHONY_LOGE("system ability manager nullptr");
        return false;
    }
    sptr<IRemoteObject> remote = sysAbilityMgr->CheckSystemAbility(ADVANCED_NOTIFICATION_SERVICE_ABILITY_ID);
    if (!remote) {
        TELEPHONY_LOGE("no ans service ability");
        return false;
    }
    return true;
}
} // namespace Telephony
} // namespace OHOS