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

#ifndef CALL_SETTING_MANAGER_H
#define CALL_SETTING_MANAGER_H

#include <cstring>
#include <memory>

#include "pac_map.h"
#include "singleton.h"

#include "cellular_call_types.h"

namespace OHOS {
namespace Telephony {
class CallSettingManager {
    DECLARE_DELAYED_SINGLETON(CallSettingManager)
public:
    bool Init();

    int32_t GetCallWaiting(int32_t slotId);
    int32_t SetCallWaiting(int32_t slotId, bool activate);
    int32_t GetCallRestriction(int32_t slotId, CallRestrictionType type);
    int32_t SetCallRestriction(int32_t slotId, CallRestrictionInfo &info);
    int32_t GetCallTransferInfo(int32_t slotId, CallTransferType type);
    int32_t SetCallTransferInfo(int32_t slotId, CallTransferInfo &info);
};
} // namespace Telephony
} // namespace OHOS
#endif // CALL_SETTING_MANAGER_H
