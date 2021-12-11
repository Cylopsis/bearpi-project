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

#include "status_bar.h"

#include "call_manager_errors.h"

namespace OHOS {
namespace Telephony {
StatusBar::StatusBar() : isDisplayMute_(false), isDisplayIcon_(false) {}

StatusBar::~StatusBar() {}

void StatusBar::UpdateMuteIcon(bool isDisplayMute)
{
    if (isDisplayMute_ == isDisplayMute) {
        return;
    }

#ifdef ABILITY_STATUS_BAR_CONTROL_SUPPORT
    if (isDisplayMute) {
        statusBarControl.setIcon("Mute");
    } else {
        statusBarControl.removeIcon("Mute");
    }
#endif
    isDisplayMute_ = isDisplayMute;
}

void StatusBar::UpdateSpeakerphoneIcon(bool isDisplay)
{
    if (isDisplayIcon_ == isDisplay) {
        return;
    }

#ifdef ABILITY_STATUS_BAR_CONTROL_SUPPORT
    if (isDisplaySpeakerphone) {
        statusBarControl.setIcon("Speakerphone");
    } else {
        statusBarControl.removeIcon("Speakerphone");
    }
#endif
    isDisplayIcon_ = isDisplay;
}

void StatusBar::CallDestroyed(sptr<CallBase> &callObjectPtr)
{
    UpdateMuteIcon(false);
    UpdateSpeakerphoneIcon(false);
}

void StatusBar::NewCallCreated(sptr<CallBase> &callObjectPtr) {}

void StatusBar::CallStateUpdated(sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState) {}

void StatusBar::IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content) {}

void StatusBar::IncomingCallActivated(sptr<CallBase> &callObjectPtr) {}
} // namespace Telephony
} // namespace OHOS