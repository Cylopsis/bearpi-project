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

#ifndef CS_CALL_H
#define CS_CALL_H

#include "cs_conference_base.h"
#include "carrier_call.h"

namespace OHOS {
namespace Telephony {
class CSCall : public CarrierCall, public CsConferenceBase {
public:
    CSCall();
    ~CSCall();
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

#endif // CS_CALL_H