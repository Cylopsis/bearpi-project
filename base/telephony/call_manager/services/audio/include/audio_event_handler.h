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

#ifndef AUDIO_EVENT_HANDLER_H
#define AUDIO_EVENT_HANDLER_H

#include <memory>

#include "event_handler.h"
#include "event_runner.h"

#include "audio_event.h"

namespace OHOS {
namespace Telephony {
/**
 * @class AudioEventHandler
 * handle call state event and audio device event.
 */
class AudioEventHandler : public AppExecFwk::EventHandler {
public:
    AudioEventHandler(
        const std::shared_ptr<AppExecFwk::EventRunner> &runner, const std::shared_ptr<AudioEvent> &manager);
    ~AudioEventHandler();
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event) override;
    void SendEmptyEvent(uint32_t event);

private:
    std::shared_ptr<AudioEvent> audioEvent_;
};
} // namespace Telephony
} // namespace OHOS
#endif // AUDIO_EVENT_HANDLER_H