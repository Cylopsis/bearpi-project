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

#ifndef EVENT_LISTENER_H
#define EVENT_LISTENER_H

#include <cstdint>

#include "napi/native_api.h"

namespace OHOS {
namespace Telephony {
struct EventListener {
    napi_env env;
    int32_t eventType;
    bool isOnce;
    napi_value thisVar;
    napi_ref callbackRef;
    int32_t slotId;
};
} // namespace Telephony
} // namespace OHOS
#endif // EVENT_LISTENER_H
