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

#ifndef CALL_STATUS_POLICY_H
#define CALL_STATUS_POLICY_H

#include "call_manager_errors.h"

#include "call_object_manager.h"

namespace OHOS {
namespace Telephony {
class CallStatusPolicy : public CallObjectManager {
public:
    CallStatusPolicy();
    ~CallStatusPolicy();
    int32_t IncomingHandlePolicy(ContactInfo info);
    int32_t DialingHandlePolicy();
    int32_t ActiveHandlePolicy();
    int32_t HoldingHandlePolicy();
    int32_t WaitingHandlePolicy();
    int32_t AlertHandlePolicy();
    int32_t DisconnectingHandlePolicy();
    int32_t DisconnectedHandlePolicy();
    int32_t IdleHandlePolicy();
    int32_t FilterResultsDispose(sptr<CallBase> call);
};
} // namespace Telephony
} // namespace OHOS

#endif // CALL_STATUS_POLICY_H
