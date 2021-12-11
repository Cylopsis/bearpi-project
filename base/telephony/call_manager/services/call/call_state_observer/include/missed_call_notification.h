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

#ifndef MISSED_CALL_NOTIFICATION_H
#define MISSED_CALL_NOTIFICATION_H

#include <cstdint>
#include <string>
#include <mutex>

#ifdef ABILITY_NOTIFICATION_SUPPORT
#include "ans_manager.h"
#include "advanced_notification_service.h"
#include "ans_manager_interface.h"
#include "ans_manager_proxy.h"
#endif

namespace OHOS {
namespace Telephony {
class MissedCallNotification {
public:
    MissedCallNotification();
    virtual ~MissedCallNotification();
    void Init();
    void PublishNotification(const std::string &title, const std::string &text);
    void CancelNotification(int32_t id);

private:
    std::mutex mutex_;
#ifdef ABILITY_NOTIFICATION_SUPPORT
    int32_t notificationId_ = 0;
    sptr<AdvancedNotificationService> ansAbility_;
    sptr<IRemoteObject> remoteObject_;
    std::unique_ptr<AnsManagerProxy> ansManagerProxy_;
#endif
    bool IsAnsAbilityExist();
};
} // namespace Telephony
} // namespace OHOS
#endif // MISSED_CALL_NOTIFICATION_H