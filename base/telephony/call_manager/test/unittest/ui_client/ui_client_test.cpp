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

#include <securec.h>
#include <memory>
#include <iostream>
#include <string_ex.h>

#include "system_ability_definition.h"

#include "call_manager_errors.h"
#include "call_manager_inner_type.h"
#include "call_manager_ipc_client.h"

namespace OHOS {
namespace Telephony {
std::unique_ptr<CallManagerIpcClient> g_clientPtr = nullptr;
using CallManagerServiceFunc = void (*)();
std::map<uint32_t, CallManagerServiceFunc> g_memberFuncMap;

void DialCall()
{
    int32_t accountId = 0;
    int32_t videoState = 0;
    int32_t dialScene = 0;
    int32_t dialType = 0;
    int32_t ret = 0;
    std::u16string phoneNumber;
    std::string tmpStr;
    AppExecFwk::PacMap dialInfo;
    std::cout << "------Dial------" << std::endl;
    std::cout << "please input phone number:" << std::endl;
    phoneNumber.clear();
    tmpStr.clear();
    std::cin >> tmpStr;
    phoneNumber = Str8ToStr16(tmpStr);
    std::cout << "you want to call " << tmpStr << std::endl;
    std::cout << "please input accountId:" << std::endl;
    std::cin >> accountId;
    std::cout << "please input videoState[0:audio,1:video]:" << std::endl;
    std::cin >> videoState;
    std::cout << "please input dialScene[0:normal,1:privileged,2:emergency]:" << std::endl;
    std::cin >> dialScene;
    std::cout << "please input dialType[0:carrier,1:voice mail,2:ott]:" << std::endl;
    std::cin >> dialType;
    dialInfo.PutIntValue("accountId", accountId);
    dialInfo.PutIntValue("videoState", videoState);
    dialInfo.PutIntValue("dialScene", dialScene);
    dialInfo.PutIntValue("dialType", dialType);
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    ret = g_clientPtr->DialCall(phoneNumber, dialInfo);
    std::cout << "return value:" << ret << std::endl;
}

void AnswerCall()
{
    int callId = 0;
    int videoState = 0;
    std::cout << "------Answer------" << std::endl;
    std::cout << "please input callId:" << std::endl;
    std::cin >> callId;
    std::cout << "please input videoState[0:audio,1:video]:" << std::endl;
    std::cin >> videoState;
    int ret = 0;
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    ret = g_clientPtr->AnswerCall(callId, videoState);
    std::cout << "return value:" << ret << std::endl;
}

void RejectCall()
{
    int callId = 0;
    int boolValue = 0;
    bool flag = false;
    std::u16string content;
    std::string tmpStr;
    content.clear();
    std::cout << "------Reject------" << std::endl;
    std::cout << "please input callId:" << std::endl;
    std::cin >> callId;
    std::cout << "Whether to enter the reason for rejection?[0:no,1:yes]:" << std::endl;
    std::cin >> boolValue;
    if (boolValue != 0) {
        flag = true;
        std::cout << "please input reject message:" << std::endl;
        tmpStr.clear();
        std::cin >> tmpStr;
        content = Str8ToStr16(tmpStr);
    }
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = g_clientPtr->RejectCall(callId, flag, content);
    std::cout << "return value:" << ret << std::endl;
}

void HoldCall()
{
    int callId = 0;
    std::cout << "------HoldCall------" << std::endl;
    std::cout << "please input callId:" << std::endl;
    std::cin >> callId;
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = g_clientPtr->HoldCall(callId);
    std::cout << "return value:" << ret << std::endl;
}

void UnHoldCall()
{
    int callId = 0;
    std::cout << "------UnHoldCall------" << std::endl;
    std::cout << "please input callId:" << std::endl;
    std::cin >> callId;
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = g_clientPtr->UnHoldCall(callId);
    std::cout << "return value:" << ret << std::endl;
}

void HangUpCall()
{
    int callId = 0;
    std::cout << "------HangUpCall------" << std::endl;
    std::cout << "please input callId:" << std::endl;
    std::cin >> callId;
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = g_clientPtr->HangUpCall(callId);
    std::cout << "return value:" << ret << std::endl;
}

void CombineConference()
{
    int mainCallId = 0;
    std::cout << "------CombineConference------" << std::endl;
    std::cout << "please input mainCallId:" << std::endl;
    std::cin >> mainCallId;
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = g_clientPtr->CombineConference(mainCallId);
    std::cout << "return value:" << ret << std::endl;
}

void SeparateConference()
{
    int callId = 0;
    std::cout << "------SeparateConference------" << std::endl;
    std::cout << "please input callId:" << std::endl;
    std::cin >> callId;
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = 0;
    std::cout << "return value:" << ret << std::endl;
}

void GetCallState()
{
    std::cout << "------GetCallState------" << std::endl;
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = g_clientPtr->GetCallState();
    std::cout << "return value:" << ret << std::endl;
}

void SwitchCall()
{
    int callId = 0;
    std::cout << "------SwitchCall------" << std::endl;
    std::cout << "please input callId:" << std::endl;
    std::cin >> callId;
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = g_clientPtr->SwitchCall(callId);
    std::cout << "return value:" << ret << std::endl;
}

void HasCall()
{
    std::cout << "------HasCall------" << std::endl;
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = g_clientPtr->HasCall();
    std::cout << "return value:" << ret << std::endl;
}

void IsNewCallAllowed()
{
    std::cout << "------IsNewCallAllowed------" << std::endl;
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = g_clientPtr->IsNewCallAllowed();
    std::cout << "return value:" << ret << std::endl;
}

void IsRinging()
{
    std::cout << "------IsRinging------" << std::endl;
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = g_clientPtr->IsRinging();
    std::cout << "return value:" << ret << std::endl;
}

void IsInEmergencyCall()
{
    std::cout << "------IsInEmergencyCall------" << std::endl;
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = g_clientPtr->IsInEmergencyCall();
    std::cout << "return value:" << ret << std::endl;
}

void StartDtmf()
{
    char c = 0;
    int callId = 0;
    std::cout << "please input StartDtmf callId:" << std::endl;
    std::cin >> callId;
    std::cout << "Please enter to send dtmf characters:" << std::endl;
    std::cin >> c;
    int32_t ret = g_clientPtr->StartDtmf(callId, c);
    std::cout << "return value:" << ret << std::endl;
}

void StopDtmf()
{
    int callId = 0;
    std::cout << "please input StopDtmf callId:" << std::endl;
    std::cin >> callId;
    int32_t ret = g_clientPtr->StopDtmf(callId);
    std::cout << "return value:" << ret << std::endl;
}

void SendDtmf()
{
    char c = 0;
    int callId = 0;
    std::cout << "please input SendDtmf callId:" << std::endl;
    std::cin >> callId;
    std::cout << "Please enter to send dtmf characters:" << std::endl;
    std::cin >> c;
    int32_t ret = g_clientPtr->SendDtmf(callId, c);
    std::cout << "return value:" << ret << std::endl;
}

void SendBurstDtmf()
{
    std::u16string strCode;
    std::string tmpStr;
    int callId = 0;
    int on = 0;
    int off = 0;
    std::cout << "please input SendBurstDtmf callId:" << std::endl;
    std::cin >> callId;
    std::cout << "please input Send dtmf string:" << std::endl;
    strCode.clear();
    tmpStr.clear();
    std::cin >> tmpStr;
    strCode = Str8ToStr16(tmpStr);
    std::cout << "please input on:" << std::endl;
    std::cin >> on;
    std::cout << "please input off:" << std::endl;
    std::cin >> off;
    int32_t ret = g_clientPtr->SendBurstDtmf(callId, strCode, on, off);
    std::cout << "return value:" << ret << std::endl;
}

void GetCallWaiting()
{
    int32_t slotId = 0;
    std::cout << "------GetCallWaiting------" << std::endl;
    std::cout << "please input slotId:" << std::endl;
    std::cin >> slotId;
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = g_clientPtr->GetCallWaiting(slotId);
    std::cout << "return value:" << ret << std::endl;
}

void SetCallWaiting()
{
    int32_t slotId = 0;
    int32_t flag = 0;
    std::cout << "------SetCallWaiting------" << std::endl;
    std::cout << "please input slotId:" << std::endl;
    std::cin >> slotId;
    std::cout << "whether open(0:no 1:yes):" << std::endl;
    std::cin >> flag;
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = g_clientPtr->SetCallWaiting(slotId, (flag == 1) ? true : false);
    std::cout << "return value:" << ret << std::endl;
}

void GetCallRestriction()
{
    int32_t slotId = 0;
    int32_t tmpType = 0;
    CallRestrictionType type;
    std::cout << "------GetCallRestriction------" << std::endl;
    std::cout << "please input slotId:" << std::endl;
    std::cin >> slotId;
    std::cout << "please input restriction type:" << std::endl;
    std::cin >> tmpType;
    type = static_cast<CallRestrictionType>(tmpType);
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = 0;
    std::cout << "return value:" << ret << std::endl;
}

void SetCallRestriction()
{
    int32_t slotId = 0;
    int32_t tmpType = 0;
    CallRestrictionInfo info;
    std::cout << "------SetCallRestriction------" << std::endl;
    std::cout << "please input slotId:" << std::endl;
    std::cin >> slotId;
    std::cout << "please input restriction type:" << std::endl;
    std::cin >> tmpType;
    info.fac = static_cast<CallRestrictionType>(tmpType);
    std::cout << "is open(1: open, 0: close):" << std::endl;
    std::cin >> tmpType;
    info.mode = static_cast<CallRestrictionMode>(tmpType);
    std::cout << "please input password:" << std::endl;
    std::cin >> info.password;
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = 0;
    std::cout << "return value:" << ret << std::endl;
}

void GetCallTransferInfo()
{
    int32_t slotId = 0;
    int32_t tmpType = 0;
    CallTransferType type;
    std::cout << "------GetCallTransferInfo------" << std::endl;
    std::cout << "please input slotId:" << std::endl;
    std::cin >> slotId;
    std::cout << "please input transfer type:" << std::endl;
    std::cin >> tmpType;
    type = static_cast<CallTransferType>(tmpType);
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = 0;
    std::cout << "return value:" << ret << std::endl;
}

void SetCallTransferInfo()
{
    int32_t slotId = 0;
    int32_t tmpType = 0;
    CallTransferInfo info;
    std::cout << "------SetCallTransferInfo------" << std::endl;
    std::cout << "please input slotId:" << std::endl;
    std::cin >> slotId;
    std::cout << "please input transfer type:" << std::endl;
    std::cin >> tmpType;
    info.type = static_cast<CallTransferType>(tmpType);
    std::cout << "please input transfer setting type:" << std::endl;
    std::cin >> tmpType;
    info.settingType = static_cast<CallTransferSettingType>(tmpType);
    std::cout << "please input phone number:" << std::endl;
    std::cin >> info.transferNum;
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = 0;
    std::cout << "return value:" << ret << std::endl;
}

void IsEmergencyPhoneNumber()
{
    int32_t slotId = 0;
    int32_t errorCode = -1;
    std::u16string phoneNumber;
    std::string tmpStr;
    std::cout << "------IsEmergencyPhoneNumber------" << std::endl;
    std::cout << "please input phone number:" << std::endl;
    phoneNumber.clear();
    tmpStr.clear();
    std::cin >> tmpStr;
    phoneNumber = Str8ToStr16(tmpStr);
    std::cout << "The number is " << tmpStr << std::endl;
    std::cout << "please input slotId:" << std::endl;
    std::cin >> slotId;
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = g_clientPtr->IsEmergencyPhoneNumber(phoneNumber, slotId, errorCode);
    std::cout << "return value:" << ret << std::endl;
    std::cout << "return errorCode:" << errorCode << std::endl;
}

void FormatPhoneNumber()
{
    std::u16string phoneNumber;
    std::u16string countryCode;
    std::u16string formatNumber;
    std::string tmpStr;
    std::cout << "------FormatPhoneNumber------" << std::endl;
    std::cout << "please input phone number:" << std::endl;
    phoneNumber.clear();
    countryCode.clear();
    formatNumber.clear();
    tmpStr.clear();
    std::cin >> tmpStr;
    phoneNumber = Str8ToStr16(tmpStr);
    std::cout << "The number is " << tmpStr << std::endl;
    tmpStr.clear();
    std::cout << "please input countryCode:" << std::endl;
    std::cin >> tmpStr;
    countryCode = Str8ToStr16(tmpStr);
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = g_clientPtr->FormatPhoneNumber(phoneNumber, countryCode, formatNumber);
    std::cout << "return value:" << ret << std::endl;
    std::cout << "return number:" << Str16ToStr8(formatNumber) << std::endl;
}

void FormatPhoneNumberToE164()
{
    std::u16string phoneNumber;
    std::u16string countryCode;
    std::u16string formatNumber;
    std::string tmpStr;
    std::cout << "------FormatPhoneNumberToE164------" << std::endl;
    std::cout << "please input phone number:" << std::endl;
    phoneNumber.clear();
    countryCode.clear();
    formatNumber.clear();
    tmpStr.clear();
    std::cin >> tmpStr;
    phoneNumber = Str8ToStr16(tmpStr);
    std::cout << "The number is " << tmpStr << std::endl;
    tmpStr.clear();
    std::cout << "please input countryCode:" << std::endl;
    std::cin >> tmpStr;
    countryCode = Str8ToStr16(tmpStr);
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return;
    }
    int32_t ret = g_clientPtr->FormatPhoneNumberToE164(phoneNumber, countryCode, formatNumber);
    std::cout << "return value:" << ret << std::endl;
    std::cout << "return number:" << Str16ToStr8(formatNumber) << std::endl;
}

void GetMainCallId()
{
    int callId = 0;
    std::cout << "please input callId:" << std::endl;
    std::cin >> callId;
    int32_t ret = g_clientPtr->GetMainCallId(callId);
    std::cout << "return value:" << ret << std::endl;
}

void GetSubCallIdList()
{
    int callId = 0;
    std::cout << "please input callId:" << std::endl;
    std::cin >> callId;
    std::vector<std::u16string> ret = g_clientPtr->GetSubCallIdList(callId);
    std::vector<std::u16string>::iterator it = ret.begin();
    for (; it != ret.end(); it++) {
        std::cout << "callId:" << Str16ToStr8(*it) << std::endl;
    }
}

void GetCallIdListForConference()
{
    int callId = 0;
    std::cout << "please input callId:" << std::endl;
    std::cin >> callId;
    std::vector<std::u16string> ret = g_clientPtr->GetCallIdListForConference(callId);
    std::vector<std::u16string>::iterator it = ret.begin();
    for (; it != ret.end(); it++) {
        std::cout << "callId:" << Str16ToStr8(*it) << std::endl;
    }
}

int32_t Init()
{
    g_clientPtr = std::make_unique<CallManagerIpcClient>();
    if (g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return TELEPHONY_ERROR;
    }
    if (g_clientPtr == nullptr) {
        std::cout << "new CallManagerIpcClient failed!" << std::endl;
        return TELEPHONY_ERROR;
    }
    int32_t ret = g_clientPtr->Init(TELEPHONY_CALL_MANAGER_SYS_ABILITY_ID);
    if (ret != TELEPHONY_SUCCESS) {
        return ret;
    }

    g_memberFuncMap[OHOS::Telephony::INTERFACE_DIAL_CALL] = &OHOS::Telephony::DialCall;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_ANSWER_CALL] = &OHOS::Telephony::AnswerCall;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_REJECT_CALL] = &OHOS::Telephony::RejectCall;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_HOLD_CALL] = &OHOS::Telephony::HoldCall;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_UNHOLD_CALL] = &OHOS::Telephony::UnHoldCall;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_DISCONNECT_CALL] = &OHOS::Telephony::HangUpCall;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_GET_CALL_STATE] = &OHOS::Telephony::GetCallState;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_SWAP_CALL] = &OHOS::Telephony::SwitchCall;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_HAS_CALL] = &OHOS::Telephony::HasCall;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_IS_EMERGENCY_CALL] = &OHOS::Telephony::IsInEmergencyCall;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_IS_EMERGENCY_NUMBER] = &OHOS::Telephony::IsEmergencyPhoneNumber;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_IS_FORMAT_NUMBER] = &OHOS::Telephony::FormatPhoneNumber;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_IS_FORMAT_NUMBER_E164] = &OHOS::Telephony::FormatPhoneNumberToE164;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_COMBINE_CONFERENCE] = &OHOS::Telephony::CombineConference;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_START_DTMF] = &OHOS::Telephony::StartDtmf;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_STOP_DTMF] = &OHOS::Telephony::StopDtmf;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_GET_CALL_WAITING] = &OHOS::Telephony::GetCallWaiting;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_SET_CALL_WAITING] = &OHOS::Telephony::SetCallWaiting;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_GET_MAINID] = &OHOS::Telephony::GetMainCallId;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_GET_SUBCALL_LIST_ID] = &OHOS::Telephony::GetSubCallIdList;
    g_memberFuncMap[OHOS::Telephony::INTERFACE_GET_CALL_LIST_ID_FOR_CONFERENCE] =
        &OHOS::Telephony::GetCallIdListForConference;
    return TELEPHONY_SUCCESS;
}

void PrintfUsage()
{
    std::cout << "\n\n-----------start--------------\n"
              << "usage:please input a cmd num:\n"
              << "1:dial\n"
              << "2:answer\n"
              << "3:reject\n"
              << "4:hold\n"
              << "5:unhold\n"
              << "6:hangUpCall\n"
              << "7:getCallState\n"
              << "8:switchCall\n"
              << "9:hasCall\n"
              << "12:isInEmergencyCall\n"
              << "13:isEmergencyPhoneNumber\n"
              << "14:formatPhoneNumber\n"
              << "15:formatPhoneNumberToE164\n"
              << "16:combine conference\n"
              << "18:StartDtmf\n"
              << "19:StopDtmf\n"
              << "22:getCallWaiting\n"
              << "23:setCallWaiting\n"
              << "28:GetMainCallId\n"
              << "29:GetSubCallIdList\n"
              << "30:GetCallIdListForConference\n"
              << "1000:exit\n";
}
} // namespace Telephony
} // namespace OHOS

int main()
{
    std::cout << "callManager test start...." << std::endl;
    int32_t interfaceNum = 0;
    const int32_t exitNumber = 1000;
    bool loopFlag = true;
    if (OHOS::Telephony::Init() != OHOS::Telephony::TELEPHONY_SUCCESS) {
        std::cout << "callManager test init failed!" << std::endl;
        return OHOS::Telephony::TELEPHONY_SUCCESS;
    }
    while (loopFlag) {
        OHOS::Telephony::PrintfUsage();
        std::cin >> interfaceNum;
        if (interfaceNum == exitNumber) {
            std::cout << "start to exit now...." << std::endl;
            break;
        }
        auto itFunc = OHOS::Telephony::g_memberFuncMap.find(interfaceNum);
        if (itFunc != OHOS::Telephony::g_memberFuncMap.end()) {
            auto memberFunc = itFunc->second;
            if (memberFunc != nullptr) {
                (*memberFunc)();
            }
            continue;
        }
        std::cout << "err: invalid input!" << std::endl;
    }
    if (OHOS::Telephony::g_clientPtr == nullptr) {
        std::cout << "g_clientPtr is nullptr" << std::endl;
        return OHOS::Telephony::TELEPHONY_FAIL;
    }
    OHOS::Telephony::g_memberFuncMap.clear();
    OHOS::Telephony::g_clientPtr->UnInit();
    std::cout << "exit success" << std::endl;
    return OHOS::Telephony::TELEPHONY_SUCCESS;
}