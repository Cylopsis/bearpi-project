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

#ifndef CS_CONFERENCE_BASE_H
#define CS_CONFERENCE_BASE_H

#include <cstdio>
#include <cstdlib>
#include <vector>
#include <memory>
#include <mutex>

namespace OHOS {
namespace Telephony {
enum ConferenceState {
    CONFERENCE_STATE_IDLE = 0,
    CONFERENCE_STATE_CREATING,
    CONFERENCE_STATE_HOLDING,
    CONFERENCE_STATE_ACTIVE,
    CONFERENCE_STATE_LEAVING,
};

class CsConferenceBase {
public:
    CsConferenceBase();
    ~CsConferenceBase();
    static int32_t SetMainCall(int32_t callId);
    static int32_t AddOneConferenceSubCallId(int32_t callId);
    static int32_t GetMainCsCallId(int32_t callId);
    static std::vector<std::u16string> GetSubCsCallIdList(int32_t callId);
    static std::vector<std::u16string> GetCsCallIdListForConference(int32_t callId);
    static ConferenceState GetCsConferenceState();
    static void SetCsConferenceState(ConferenceState state);
    static int32_t SeparateConference(int32_t callId);
    static int32_t CanCombineCsConference();
    static int32_t SubCallCombineToCsConference(int32_t callId);
    static int32_t SubCallSeparateFromCsConference(int32_t callId);
    static int32_t CanSeparateCsConference();

private:
    static int32_t mainCallId_;
    static ConferenceState state_;
    static std::vector<int32_t> subCallIdVec_;
    static std::mutex vecMutex_;
};
} // namespace Telephony
} // namespace OHOS

#endif // CS_CONFERENCE_BASE_H