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

#ifndef RING_H
#define RING_H

#include <memory>

#ifdef ABILITY_AUDIO_SUPPORT
#include "audio_service_client.h"
#endif
#include "audio_proxy.h"

namespace OHOS {
namespace Telephony {
struct RingtoneStream {
    std::string ringtonePath;
    uint32_t playedPosition;
};
enum RingState { RINGING = 0, PAUSED, STOPPED };

/**
 * @class Ring
 * plays the default or specific ringtone.
 */
class Ring {
public:
    Ring();
    explicit Ring(const std::string &path);
    virtual ~Ring();
    void Init(const std::string &ringtonePath);
    int32_t Start();
    int32_t Stop();
    int32_t Resume();
    int32_t Pause();
    int32_t Release();
    int32_t StartVibrate();
    int32_t CancelVibrate();
    void SetVolume(float volume);
    float GetVolume();
    void SetLoop(uint32_t number);
    uint32_t GetLoop();
    void SetSpeed(double speed);
    double GetSpeed();

private:
    const uint32_t DEFAULT_RING_LOOP_NUMBER = 10; // default loop time
    const float DEFAULT_RING_SPEED = 1.0; // default play speed
    const static size_t MIN_BYTES = 4;
    const static size_t ELEMENT_SIZE = 1;
    bool isVibrating_;
    bool shouldRing_;
    bool shouldVibrate_;
    float volume_ = 0;
    int32_t loopNumber_;
    float speed_;
    bool ShouldVibrate();
    static bool keepRinging_;
#ifdef ABILITY_AUDIO_SUPPORT
    static std::unique_ptr<AudioServiceClient> ringtonePlayer_;
#endif
    static std::unique_ptr<RingtoneStream> ringtoneStream_;
    static int32_t PlayRingtoneStream(const std::string &path, int32_t offset);
};
} // namespace Telephony
} // namespace OHOS
#endif // RNG_H