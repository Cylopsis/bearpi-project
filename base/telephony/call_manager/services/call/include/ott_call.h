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

#ifndef OTT_CALL_H
#define OTT_CALL_H

#include "call_base.h"
#include "net_call_base.h"

namespace OHOS {
namespace Telephony {
class OTTCall : public CallBase, public NetCallBase {
public:
    OTTCall();
    ~OTTCall();

    void OutCallInit(const CallReportInfo &info, AppExecFwk::PacMap &extras, int32_t callId);
    void InCallInit(const CallReportInfo &info, int32_t callId);
    int32_t DialingProcess() override;
    int32_t AnswerCall(int32_t videoState) override;
    int32_t RejectCall(bool isSendSms, std::string &content) override;
    int32_t HangUpCall() override;
    int32_t HoldCall() override;
    int32_t UnHoldCall() override;
    int32_t SwitchCall() override;
    void GetCallAttributeInfo(CallAttributeInfo &info) override;
    bool GetEmergencyState() override;
    int32_t StartDtmf(std::string &phoneNum, char str) override;
    int32_t StopDtmf(std::string &phoneNum) override;
    int32_t SendDtmf(std::string &phoneNum, char str) override;
    int32_t SendBurstDtmf(std::string &phoneNum, std::string str, int32_t on, int32_t off) override;
    int32_t GetSlotId() override;
    int32_t CombineConference() override;
    int32_t CanCombineConference() override;
    int32_t SubCallCombineToConference() override;
    int32_t SubCallSeparateFromConference() override;
    int32_t CanSeparateConference() override;
    int32_t GetMainCallId() override;
    std::vector<std::u16string> GetSubCallIdList() override;
    std::vector<std::u16string> GetCallIdListForConference() override;
};
} // namespace Telephony
} // namespace OHOS

#endif // OTT_CALL_H