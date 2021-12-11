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

#ifndef CALL_MANAGER_BROADCAST_SUBSCRIBER_H
#define CALL_MANAGER_BROADCAST_SUBSCRIBER_H

#include <string>
#include <vector>
#include <map>

#include "common_event.h"
#include "common_event_manager.h"

namespace OHOS {
namespace Telephony {
using namespace OHOS::EventFwk;
const std::string SIM_STATE_UPDATE_ACTION = "com.hos.action.SIM_STATE_CHANGED";
class CallManagerBroadcastSubscriber : public CommonEventSubscriber {
public:
    explicit CallManagerBroadcastSubscriber(const CommonEventSubscribeInfo &subscriberInfo);
    ~CallManagerBroadcastSubscriber() = default;
    virtual void OnReceiveEvent(const CommonEventData &data);

private:
    enum CallManagerBroadcastEventType {
        UNKNOWN_BROADCAST_EVENT = 0,
        SIM_STATE_BROADCAST_EVENT,
    };
    using broadcastSubscriberFunc = void (CallManagerBroadcastSubscriber::*)(const CommonEventData &data);

    void UnknownBroadcast(const CommonEventData &data);
    void SimStateBroadcast(const CommonEventData &data);
    std::map<uint32_t, broadcastSubscriberFunc> memberFuncMap_;
};
} // namespace Telephony
} // namespace OHOS

#endif // CALL_MANAGER_BROADCAST_SUBSCRIBER_H
