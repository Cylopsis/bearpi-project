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

#ifndef CARRIER_CALL_H
#define CARRIER_CALL_H

#include "cellular_call_types.h"

#include "call_base.h"

namespace OHOS {
namespace Telephony {
class CarrierCall : public CallBase {
public:
    CarrierCall();
    ~CarrierCall();
    void InitCarrierOutCallInfo(const CallReportInfo &info, AppExecFwk::PacMap &extras, int32_t callId);
    void InitCarrierInCallInfo(const CallReportInfo &info, int32_t callId);
    int32_t CarrierDialingProcess();
    int32_t CarrierAcceptCall(int32_t videoState);
    int32_t CarrierRejectCall(bool isSendSms, std::string &content);
    int32_t CarrierHangUpCall();
    int32_t CarrierHoldCall();
    int32_t CarrierUnHoldCall();
    void GetCallAttributeCarrierInfo(CallAttributeInfo &info);
    bool GetEmergencyState() override;
    int32_t CarrierSwitchCall();
    int32_t GetCallTransferInfo();
    int32_t SetCallTransferInfo();
    int32_t StartDtmf(std::string &phoneNum, char str) override;
    int32_t StopDtmf(std::string &phoneNum) override;
    int32_t SendDtmf(std::string &phoneNum, char str) override;
    int32_t SendBurstDtmf(std::string &phoneNum, std::string str, int32_t on, int32_t off) override;
    int32_t GetSlotId() override;
    int32_t CarrierCombineConference();

private:
    void PackCellularCallInfo(CellularCallInfo &callInfo);

private:
    DialScene dialScene_;
    int32_t slotId_;
    int32_t index_;
    bool isEcc_;
    bool isDefault_; // Whether to dial by default
    bool isVoicemailNumber_;
    std::mutex mutex_;
};
} // namespace Telephony
} // namespace OHOS

#endif // CARRIER_CALL_H