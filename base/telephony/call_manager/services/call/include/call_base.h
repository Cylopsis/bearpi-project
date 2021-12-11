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

#ifndef CALL_BASE_H
#define CALL_BASE_H

#include <unistd.h>
#include <cstring>
#include <memory>
#include <mutex>

#include "refbase.h"
#include "pac_map.h"

#include "common_type.h"

namespace OHOS {
namespace Telephony {
class CallBase : public virtual RefBase {
public:
    CallBase();
    virtual ~CallBase();

    void InitOutCallBase(const CallReportInfo &info, AppExecFwk::PacMap &extras, int32_t callId);
    void InitInCallBase(const CallReportInfo &info, int32_t callId);
    virtual int32_t DialingProcess() = 0;
    virtual int32_t AnswerCall(int32_t videoState) = 0;
    virtual int32_t RejectCall(bool isSendSms, std::string &content) = 0;
    virtual int32_t HangUpCall() = 0;
    virtual int32_t HoldCall() = 0;
    virtual int32_t UnHoldCall() = 0;
    virtual int32_t SwitchCall() = 0;
    virtual void GetCallAttributeInfo(CallAttributeInfo &info) = 0;
    virtual bool GetEmergencyState() = 0;
    virtual int32_t StartDtmf(std::string &phoneNum, char str) = 0;
    virtual int32_t StopDtmf(std::string &phoneNum) = 0;
    virtual int32_t SendDtmf(std::string &phoneNum, char str) = 0;
    virtual int32_t SendBurstDtmf(std::string &phoneNum, std::string str, int32_t on, int32_t off) = 0;
    virtual int32_t GetSlotId() = 0;
    virtual int32_t CombineConference() = 0;
    virtual int32_t CanCombineConference() = 0;
    virtual int32_t SubCallCombineToConference() = 0;
    virtual int32_t SubCallSeparateFromConference() = 0;
    virtual int32_t CanSeparateConference() = 0;
    virtual int32_t GetMainCallId() = 0;
    virtual std::vector<std::u16string> GetSubCallIdList() = 0;
    virtual std::vector<std::u16string> GetCallIdListForConference() = 0;
    int32_t DialCallBase();
    int32_t IncomingCallBase();
    int32_t AcceptCallBase();
    int32_t RejectCallBase();
    int32_t HoldCallBase();
    int32_t UnHoldCallBase();
    int32_t TurnOffVoice(bool silence);
    void GetCallAttributeBaseInfo(CallAttributeInfo &info);
    int32_t GetCallID();
    CallType GetCallType();
    CallRunningState GetCallRunningState();
    int32_t SetTelCallState(TelCallState nextState);
    TelCallState GetTelCallState();
    int32_t SetTelConferenceState(TelConferenceState state);
    TelConferenceState GetTelConferenceState();
    VideoStateType GetVideoStateType();
    void SetPolicyFlag(int64_t policyFlag);
    int64_t GetPolicyFlag();
    bool GetCallerInfo(ContactInfo &info);
    void SetCallerInfo(const ContactInfo &contactInfo);
    CallEndedType GetCallEndedType();
    int32_t SetCallEndedType(CallEndedType callEndedType);
    bool IsSpeakerphoneEnabled();
    bool IsCurrentRinging();
    std::string GetAccountNumber();
    int32_t SetSpeakerphoneOn(bool speakerphoneOn);
    bool IsSpeakerphoneOn();
    void SetAudio();
    bool CheckVoicemailNumber(std::string phoneNumber);
    bool IsAliveState();

protected:
    int32_t callId_;
    CallType callType_;
    VideoStateType videoState_;
    std::string accountNumber_;

private:
    CallRunningState callRunningState_;
    TelConferenceState conferenceState_;
    int32_t startTime_; // Call start time
    CallDirection direction_;
    int64_t policyFlag_;
    TelCallState callState_;
    bool isSpeakerphoneOn_;
    CallEndedType callEndedType_;
    ContactInfo contactInfo_;
    std::mutex mutex_;
};
} // namespace Telephony
} // namespace OHOS

#endif // CALL_BASE_H