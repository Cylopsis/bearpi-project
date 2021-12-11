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

#ifndef TELEPHONY_STATE_MANAGER_H
#define TELEPHONY_STATE_MANAGER_H

#include "i_telephony_state_notify.h"
#include "telephony_observer.h"

namespace OHOS {
namespace Telephony {
class TelephonyStateManager {
public:
    TelephonyStateManager();
    virtual ~TelephonyStateManager();
    int32_t AddStateObserver(const sptr<TelephonyObserverBroker> &telephonyObserver, int32_t subId, uint32_t mask,
        const std::u16string &callingPackage, bool notifyNow);
    int32_t RemoveStateObserver(int32_t subId, uint32_t mask);

private:
    sptr<ITelephonyStateNotify> telephonyStateNotify_;

    int ConnectService();
};
} // namespace Telephony
} // namespace OHOS
#endif // TELEPHONY_STATE_MANAGER_H