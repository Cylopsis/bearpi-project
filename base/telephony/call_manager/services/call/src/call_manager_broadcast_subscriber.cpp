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

#include "call_manager_broadcast_subscriber.h"

#include <string_ex.h>

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
using namespace OHOS::EventFwk;
CallManagerBroadcastSubscriber::CallManagerBroadcastSubscriber(
    const OHOS::EventFwk::CommonEventSubscribeInfo &subscriberInfo)
    : CommonEventSubscriber(subscriberInfo)
{
    memberFuncMap_[UNKNOWN_BROADCAST_EVENT] = &CallManagerBroadcastSubscriber::UnknownBroadcast;
    memberFuncMap_[SIM_STATE_BROADCAST_EVENT] = &CallManagerBroadcastSubscriber::SimStateBroadcast;
}

void CallManagerBroadcastSubscriber::OnReceiveEvent(const CommonEventData &data)
{
    uint32_t code = UNKNOWN_BROADCAST_EVENT;
    OHOS::EventFwk::Want want = data.GetWant();
    std::string action = data.GetWant().GetAction();
    TELEPHONY_LOGD("receive one broadcast:%s", action.c_str());
    if (action == SIM_STATE_UPDATE_ACTION) {
        code = SIM_STATE_BROADCAST_EVENT;
    } else {
        code = UNKNOWN_BROADCAST_EVENT;
    }

    auto itFunc = memberFuncMap_.find(code);
    if (itFunc != memberFuncMap_.end()) {
        auto memberFunc = itFunc->second;
        if (memberFunc != nullptr) {
            return (this->*memberFunc)(data);
        }
    }
}

void CallManagerBroadcastSubscriber::UnknownBroadcast(const CommonEventData &data)
{
    TELEPHONY_LOGD("you receive one unknown broadcast!");
}

void CallManagerBroadcastSubscriber::SimStateBroadcast(const CommonEventData &data)
{
    TELEPHONY_LOGD("sim state broadcast code:%d", data.GetCode());
}
} // namespace Telephony
} // namespace OHOS