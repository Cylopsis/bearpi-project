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

#include "incoming_call_notification.h"

#include "if_system_ability_manager.h"
#include "iservice_registry.h"
#include "system_ability_definition.h"

#include "telephony_log_wrapper.h"

#ifdef ABILITY_DISPLAY_SUPPORT
#include "display_manager.h"
#include "display_mgr_service.h"
#endif

namespace OHOS {
namespace Telephony {
IncomingCallNotification::IncomingCallNotification()
{
    Init();
}

IncomingCallNotification::~IncomingCallNotification()
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

void IncomingCallNotification::Init()
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

void IncomingCallNotification::NewCallCreated(sptr<CallBase> &callObjectPtr)
{
    if (callObjectPtr != nullptr && callObjectPtr->GetTelCallState() == TelCallState::CALL_STATUS_INCOMING) {
        PublishNotification(title_, callObjectPtr->GetAccountNumber());
    }
}

void IncomingCallNotification::CallDestroyed(sptr<CallBase> &callObjectPtr)
{
    if (callObjectPtr != nullptr && callObjectPtr->GetTelCallState() == TelCallState::CALL_STATUS_INCOMING) {
        CancelNotification();
    }
}

void IncomingCallNotification::CallStateUpdated(
    sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState)
{
    if (callObjectPtr == nullptr) {
        return;
    }
    if (priorState == TelCallState::CALL_STATUS_INCOMING) {
        CancelNotification(); // incoming call state left
    } else if (nextState == TelCallState::CALL_STATUS_INCOMING) {
        PublishNotification(title_, callObjectPtr->GetAccountNumber()); // other call state --> incoming
    }
}

void IncomingCallNotification::IncomingCallActivated(sptr<CallBase> &callObjectPtr)
{
    if (callObjectPtr == nullptr) {
        return;
    }
    CancelNotification();
}

void IncomingCallNotification::IncomingCallHungUp(
    sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content)
{
    if (callObjectPtr == nullptr) {
        return;
    }
    CancelNotification();
}

void IncomingCallNotification::PublishNotification(const std::string &title, const std::string &text)
{
    // show the incoming call notification while full screen
    if (!IsFullScreen()) {
        return;
    }
#ifdef ABILITY_NOTIFICATION_SUPPORT
    if (ansManagerProxy_ == nullptr) {
        TELEPHONY_LOGE("ans manager proxy nullptr");
        return;
    }
    std::unique_ptr<NotificationMediaContent> notificationMediaContent =
        std::make_unique<NotificationMediaContent>();
    sptr<NotificationMediaContent> mediaContent = notificationMediaContent.release();
    if (mediaContent == nullptr) {
        TELEPHONY_LOGE("ans notification nullptr");
        return;
    }
    mediaContent->SetTitle(title);
    mediaContent->SetText(text);
    mediaContent.SetShownActions(ACTION_ANSWER, ACTION_REJECT);
    std::unique_ptr<NotificationContent> notificationContent = std::make_unique<NotificationContent>(mediaContent);
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
    notification->SetNotificationId(notificationId_);
    notification->SetContent(content);
    ansManagerProxy_->Publish(notification, nullptr, true);
#endif
}

void IncomingCallNotification::CancelNotification()
{
#ifdef ABILITY_NOTIFICATION_SUPPORT
    if (ansManagerProxy_ == nullptr) {
        TELEPHONY_LOGE("ans manager proxy nullptr");
        return;
    }
    ansManagerProxy_->CancelNotification(notificationId_);
#endif
}

bool IncomingCallNotification::IsAnsAbilityExist()
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

bool IncomingCallNotification::IsFullScreen()
{
#ifdef ABILITY_DISPLAY_SUPPORT
    std::lock_guard<std::mutex> lock(mutex_);
    sptr<ISystemAbilityManager> sysAbilityMgr = SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (!sysAbilityMgr) {
        TELEPHONY_LOGE("system ability manager nullptr");
        return false;
    }
    sptr<IRemoteObject> remote = sysAbilityMgr->CheckSystemAbility(DISPLAY_MANAGER_SERVICE_ID);
    if (!remote) {
        TELEPHONY_LOGE("no display ability");
        return false;
    }
    return DisplayManager::IsFullScreen();
#endif
    return false;
}
} // namespace Telephony
} // namespace OHOS