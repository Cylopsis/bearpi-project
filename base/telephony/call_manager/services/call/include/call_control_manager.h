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

#ifndef CALL_CONTROL_MANAGER_H
#define CALL_CONTROL_MANAGER_H

#include <cstring>
#include <list>
#include <memory>
#include <mutex>

#include "pac_map.h"
#include "singleton.h"
#include "cellular_call_types.h"

#include "call_policy.h"
#include "call_state_listener.h"
#include "report_call_state_handler.h"
#include "call_request_handler.h"
#include "hang_up_sms.h"

namespace OHOS {
namespace Telephony {
struct DialSourceInfo {
    int32_t callId;
    std::string number;
    AppExecFwk::PacMap extras;
    DialType dialType;
    bool isDialing;
};

class CallControlManager : public CallPolicy {
    DECLARE_DELAYED_SINGLETON(CallControlManager)
public:
    bool Init();
    int32_t DialCall(std::u16string &number, AppExecFwk::PacMap &extras);
    int32_t AnswerCall(int32_t callId, int32_t videoState);
    int32_t RejectCall(int32_t callId, bool rejectWithMessage, std::u16string textMessage);
    int32_t HangUpCall(int32_t callId);
    int32_t GetCallState();
    int32_t HoldCall(int32_t callId);
    int32_t UnHoldCall(int32_t callId);
    int32_t SwitchCall();
    bool HasCall();
    bool IsNewCallAllowed();
    bool IsRinging();
    bool HasEmergency();
    bool NotifyNewCallCreated(sptr<CallBase> &callObjectPtr);
    bool NotifyCallDestroyed(sptr<CallBase> &callObjectPtr);
    bool NotifyCallStateUpdated(sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState);
    bool NotifyIncomingCallAnswered(sptr<CallBase> &callObjectPtr);
    bool NotifyIncomingCallRejected(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content);
    bool NotifyCallEventUpdated(CallEventInfo &info);
    int32_t StartDtmf(int32_t callId, char str);
    int32_t SendDtmf(int32_t callId, char str);
    int32_t SendBurstDtmf(int32_t callId, std::u16string str, int32_t on, int32_t off);
    int32_t StopDtmf(int32_t callId);
    int32_t CombineConference(int32_t mainCallId);
    int32_t SeparateConference(int32_t callId);
    int32_t GetMainCallId(int32_t callId);
    std::vector<std::u16string> GetSubCallIdList(int32_t callId);
    std::vector<std::u16string> GetCallIdListForConference(int32_t callId);
    int32_t DialProcess();
    int32_t CreateNewCall(const CallReportInfo &info);

private:
    int32_t NumberLegalityCheck(std::string &number);
    int32_t CarrierDialProcess();
    int32_t VoiceMailDialProcess();
    int32_t OttDialProcess();
    void PackCellularCallInfo(CellularCallInfo &callInfo);

private:
    std::unique_ptr<CallStateListener> callStateListenerPtr_;
    std::unique_ptr<CallRequestHandlerService> callRequestHandlerServicePtr_;
    sptr<ReportCallStateHandlerService> reportCallStateHandlerPtr_;
    sptr<HangUpSms> hungUpSms_;
    DialSourceInfo dialSrcInfo_;
    std::mutex mutex_;
};
} // namespace Telephony
} // namespace OHOS

#endif // CALL_CONTROL_MANAGER_H