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

#include "wired_headset.h"

#include "call_manager_errors.h"

namespace OHOS {
namespace Telephony {
WiredHeadsetHandler::WiredHeadsetHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner)
    : AppExecFwk::EventHandler(runner)
{}

WiredHeadsetHandler::~WiredHeadsetHandler() {}

void WiredHeadsetHandler::Init() {}

void WiredHeadsetHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr) {
        return;
    }
    switch (event->GetInnerEventId()) {
        case WiredHeadset::MEDIA_INITIALIZE_MSG: {
            // set media initialization
            break;
        }
        case WiredHeadset::MEDIA_SET_ACTIVE_MSG: {
            // set media is active
            break;
        }
        case WiredHeadset::MEDIA_SET_LEAVE_MSG: {
            // set media is inactive
            break;
        }
        default: {
            break;
        }
    }
}

WiredHeadset::WiredHeadset() : eventLoop_(nullptr), handler_(nullptr) {}

WiredHeadset::~WiredHeadset() {}

void WiredHeadset::Init() {}

#ifdef ABILITY_MEDIA_SUPPORT
boolean WiredHeadset::ButtonEventhandling(ButtonEvent event)
{
    if (event.isShortPress()) {
        return callsManager.onButtonDealing(ButtonEvent::SHORT_PRESS);
    }
    return callsManager.onButtonDealing(ButtonEvent::LONG_PRESS);
}
#endif

void WiredHeadset::NewCallCreated(sptr<CallBase> &callObjectPtr)
{
    if (handler_ != nullptr) {
        handler_->SendEvent(WiredHeadset::MEDIA_SET_ACTIVE_MSG);
    }
}

void WiredHeadset::CallDestroyed(sptr<CallBase> &callObjectPtr)
{
    if (handler_ != nullptr) {
        handler_->SendEvent(WiredHeadset::MEDIA_SET_LEAVE_MSG);
    }
}

void WiredHeadset::CallStateUpdated(sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState)
{}

void WiredHeadset::IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content) {}

void WiredHeadset::IncomingCallActivated(sptr<CallBase> &callObjectPtr) {}
} // namespace Telephony
} // namespace OHOS