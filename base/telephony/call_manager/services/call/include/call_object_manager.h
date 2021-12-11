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

#ifndef CALL_OBJECT_MANAGER_H
#define CALL_OBJECT_MANAGER_H

#include <cstdio>
#include <cstdlib>
#include <list>
#include <memory>
#include <mutex>

#include "refbase.h"

#include "call_base.h"
#include "common_type.h"

namespace OHOS {
namespace Telephony {
class CallObjectManager {
public:
    CallObjectManager();
    virtual ~CallObjectManager();

    static int32_t AddOneCallObject(sptr<CallBase> &call);
    static int32_t DeleteOneCallObject(int32_t callId);
    static void DeleteOneCallObject(sptr<CallBase> &call);
    static sptr<CallBase> GetOneCallObject(int32_t callId);
    static sptr<CallBase> GetOneCallObject(std::string &phoneNumber);
    int32_t HasNewCall();
    bool IsNewCallAllowedCreate();
    static int32_t GetActiveCallList(std::list<sptr<CallBase>> &list);
    static int32_t GetHoldCallList(std::list<sptr<CallBase>> &list);
    int32_t GetCarrierCallList(std::list<sptr<CallBase>> &list);
    bool HasRingingMaximum();
    bool HasDialingMaximum();
    bool HasEmergencyCall();
    static int32_t GetNewCallId();
    bool IsCallExist(int32_t callId);
    bool IsCallExist(std::string &phoneNumber);
    static bool HasCallExist();
    bool HasRingingCall();
    TelCallState GetCallState(int32_t callId);
    std::list<sptr<CallBase>> GetCallList();
    static bool IsCallExist(CallType type, TelCallState callState);

private:
    static std::list<sptr<CallBase>> callObjectPtrList_;
    static std::mutex listMutex_;
    static int32_t callId_;
    static uint32_t maxCallCount_;
};
} // namespace Telephony
} // namespace OHOS

#endif // CALL_OBJECT_MANAGER_H