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

#include "call_state_listener.h"

#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
CallStateListener::CallStateListener()
{
    listenerSet_.clear();
}

CallStateListener::~CallStateListener()
{
    listenerSet_.clear();
}

bool CallStateListener::AddOneObserver(const sptr<CallStateListenerBase> &observer)
{
    if (observer == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    if (listenerSet_.count(observer) > 0) {
        return true;
    }
    listenerSet_.insert(observer);
    return true;
}

bool CallStateListener::RemoveOneObserver(const sptr<CallStateListenerBase> &observer)
{
    if (observer == nullptr) {
        return false;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    listenerSet_.erase(observer);
    return true;
}

bool CallStateListener::RemoveAllObserver()
{
    std::lock_guard<std::mutex> lock(mutex_);
    listenerSet_.clear();
    return true;
}

void CallStateListener::NewCallCreated(sptr<CallBase> &callObjectPtr)
{
    if (callObjectPtr == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &observer : listenerSet_) {
        observer->NewCallCreated(callObjectPtr);
    }
}

void CallStateListener::CallDestroyed(sptr<CallBase> &callObjectPtr)
{
    if (callObjectPtr == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &observer : listenerSet_) {
        observer->CallDestroyed(callObjectPtr);
    }
}

void CallStateListener::CallStateUpdated(
    sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState)
{
    if (callObjectPtr == nullptr) {
        TELEPHONY_LOGE("callObjectPtr is nullptr");
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &observer : listenerSet_) {
        observer->CallStateUpdated(callObjectPtr, priorState, nextState);
    }
}

void CallStateListener::IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content)
{
    if (callObjectPtr == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &observer : listenerSet_) {
        observer->IncomingCallHungUp(callObjectPtr, isSendSms, content);
    }
}

void CallStateListener::IncomingCallActivated(sptr<CallBase> &callObjectPtr)
{
    if (callObjectPtr == nullptr) {
        return;
    }
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &observer : listenerSet_) {
        observer->IncomingCallActivated(callObjectPtr);
    }
}

void CallStateListener::CallEventUpdated(CallEventInfo &info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto &observer : listenerSet_) {
        observer->CallEventUpdated(info);
    }
}
} // namespace Telephony
} // namespace OHOS