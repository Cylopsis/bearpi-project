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

#include "audio_event_handler.h"

#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
AudioEventHandler::AudioEventHandler(
    const std::shared_ptr<AppExecFwk::EventRunner> &runner, const std::shared_ptr<AudioEvent> &manager)
    : AppExecFwk::EventHandler(runner), audioEvent_(manager)
{}

void AudioEventHandler::SendEmptyEvent(uint32_t event)
{
    SendEvent(event, 0, 0);
}

AudioEventHandler::~AudioEventHandler() {}

void AudioEventHandler::ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event)
{
    if (event == nullptr || audioEvent_ == nullptr) {
        return;
    }
    audioEvent_->HandleEvent(event->GetInnerEventId());
}
} // namespace Telephony
} // namespace OHOS