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

#ifndef CALL_STATUS_MANAGER_H
#define CALL_STATUS_MANAGER_H

#include "refbase.h"

#include "common_type.h"
#include "call_status_policy.h"

namespace OHOS {
namespace Telephony {
class CallStatusManager : public CallStatusPolicy {
public:
    CallStatusManager();
    ~CallStatusManager();
    int32_t Init();
    int32_t UnInit();
    int32_t HandleCallReportInfo(const CallReportInfo &info);
    int32_t HandleCallsReportInfo(const CallsReportInfo &info);
    int32_t HandleEventResultReportInfo(const CellularCallEventInfo &info);

private:
    int32_t IncomingHandle(const CallReportInfo &info);
    int32_t DialingHandle(const CallReportInfo &info);
    int32_t ActiveHandle(const CallReportInfo &info);
    int32_t HoldingHandle(const CallReportInfo &info);
    int32_t WaitingHandle(const CallReportInfo &info);
    int32_t AlertHandle(const CallReportInfo &info);
    int32_t DisconnectingHandle(const CallReportInfo &info);
    int32_t DisconnectedHandle(const CallReportInfo &info);

    int32_t ConferenceHandle();
    int32_t CreateNewCall(const CallReportInfo &info, int32_t callId);
    int32_t UpdateCallState(sptr<CallBase> &call, TelCallState nextState);
    int32_t ToSpeakerPhone(sptr<CallBase> &call);
    int32_t TurnOffMute(sptr<CallBase> &call);

private:
    CallReportInfo callReportInfo_;
    CallsReportInfo callsReportInfo_;
};
} // namespace Telephony
} // namespace OHOS

#endif // CALL_STATUS_MANAGER_H