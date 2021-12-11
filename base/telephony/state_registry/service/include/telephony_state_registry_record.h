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

#ifndef STATE_REGISTRY_TELEPHONY_STATE_REGISTRY_RECORD_H
#define STATE_REGISTRY_TELEPHONY_STATE_REGISTRY_RECORD_H

#include <string>

#include "telephony_observer_broker.h"

namespace OHOS {
namespace Telephony {
class TelephonyStateRegistryRecord {
public:
    bool IsCanReadCallHistory();
    /**
     * IsExistStateListener
     *
     * @param mask Listening type bitmask
     * @return bool mask exist on true, others on false.
     */
    bool IsExistStateListener(uint32_t mask) const;

public:
    std::u16string package_;
    pid_t pid_ = 0;
    unsigned int mask_ = 0;
    int simId_ = 0;
    sptr<TelephonyObserverBroker> telephonyObserver_ = nullptr;
};
} // namespace Telephony
} // namespace OHOS
#endif // STATE_REGISTRY_TELEPHONY_STATE_REGISTRY_RECORD_H
