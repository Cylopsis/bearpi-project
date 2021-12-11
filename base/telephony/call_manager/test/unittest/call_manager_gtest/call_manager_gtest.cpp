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

#include "call_manager_gtest.h"

#include <cstring>
#include <gtest/gtest.h>
#include <string>

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
constexpr int32_t SIM1_SLOTID = 0;
constexpr int32_t SIM1_SLOTID_NO_CARD = 0;
constexpr int32_t RETURN_VALUE_IS_ZERO = 0;
constexpr int32_t INVALID_NEGATIVE_ID = -100;
constexpr int32_t INVALID_POSITIVE_ID = 100;
constexpr int32_t CALL_MANAGER_ERROR = -1;
/********************************************* Test DialCall()***********************************************/

/**
 * @tc.number   Telephony_CallManager_DialCall_0100
 * @tc.name     make a normal phone call with null telephone numbers
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_DialCall_0100, Function | MediumTest | Level3)
{
    std::string phoneNumber = "";
    AppExecFwk::PacMap dialInfo;
    dialInfo.PutIntValue("accountId", 0);
    dialInfo.PutIntValue("videoState", (int32_t)VideoStateType::TYPE_VOICE);
    dialInfo.PutIntValue("dialScene", CALL_NORMAL);
    int32_t ret = CallManagerGtest::clientPtr_->DialCall(Str8ToStr16(phoneNumber), dialInfo);
    EXPECT_NE(ret, RETURN_VALUE_IS_ZERO);
}

/********************************************* Test AnswerCall() ***********************************************/
/**
 * @tc.number   Telephony_CallManager_AnswerCall_0100
 * @tc.name     test AnswerCall with the callId does not exist
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_AnswerCall_0100, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_NEGATIVE_ID;
    int32_t videoState = (int32_t)VideoStateType::TYPE_VOICE;
    EXPECT_NE(CallManagerGtest::clientPtr_->AnswerCall(callId, videoState), RETURN_VALUE_IS_ZERO)
        << "callId = " << callId;
}

/**
 * @tc.number   Telephony_CallManager_AnswerCall_0200
 * @tc.name     test AnswerCall with the videoState does not exist
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_AnswerCall_0200, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_POSITIVE_ID;
    int32_t videoState = INVALID_NEGATIVE_ID;
    EXPECT_NE(CallManagerGtest::clientPtr_->AnswerCall(callId, videoState), RETURN_VALUE_IS_ZERO)
        << "callId = " << callId;
}

/********************************************* Test RejectCall() ***********************************************/
/**
 * @tc.number   Telephony_CallManager_RejectCall_0100
 * @tc.name     test RejectCall with the callId does not exist
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_RejectCall_0100, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_NEGATIVE_ID;
    std::u16string textMessage = Str8ToStr16("this is a test message");
    EXPECT_NE(CallManagerGtest::clientPtr_->RejectCall(callId, true, textMessage), RETURN_VALUE_IS_ZERO)
        << "callId = " << callId;
}

/******************************************* Test HangUpCall() *********************************************/
/**
 * @tc.number   Telephony_CallManager_HangUpCall_0100
 * @tc.name     test disconnect call with wrong callId
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_HangUpCall_0100, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_NEGATIVE_ID;
    EXPECT_NE(CallManagerGtest::clientPtr_->HangUpCall(callId), RETURN_VALUE_IS_ZERO) << "callId = " << callId;
}

/******************************************* Test GetCallState() *********************************************/
/**
 * @tc.number   Telephony_CallManager_GetCallState_0100
 * @tc.name     test GetCallState() without  call
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_GetCallState_0100, Function | MediumTest | Level3)
{
    EXPECT_EQ(CallManagerGtest::clientPtr_->GetCallState(), (int32_t)CallStateToApp::CALL_STATE_IDLE);
}

/******************************************* Test HoldCall() *********************************************/
/**
 * @tc.number   Telephony_CallManager_HoldCall_0100
 * @tc.name     Hold calls for non-existent call ID
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_HoldCall_0100, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_NEGATIVE_ID;
    EXPECT_NE(CallManagerGtest::clientPtr_->HoldCall(callId), RETURN_VALUE_IS_ZERO) << "callId = " << callId;
}

/******************************************* Test UnHoldCall() *********************************************/
/**
 * @tc.number   Telephony_CallManager_UnHoldCall_0100
 * @tc.name     Replies calls to a call ID that does not exist , return non 0
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_UnHoldCall_0100, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_NEGATIVE_ID;
    EXPECT_NE(CallManagerGtest::clientPtr_->UnHoldCall(callId), RETURN_VALUE_IS_ZERO) << "callId = " << callId;
}

/******************************************* Test SwitchCall() *********************************************/
/**
 * @tc.number   Telephony_CallManager_SwitchCall_0100
 * @tc.name     Test returns 0 after switching call
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_SwitchCall_0100, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_NEGATIVE_ID;
    EXPECT_NE(CallManagerGtest::clientPtr_->SwitchCall(callId), RETURN_VALUE_IS_ZERO);
}

/********************************************* Test HasCall() ***********************************************/
/**
 * @tc.number   Telephony_CallManager_HasCall_0100
 * @tc.name     Free time test Hascall() , return false
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_HasCall_0100, Function | MediumTest | Level3)
{
    int32_t callState = CallManagerGtest::clientPtr_->GetCallState();
    int32_t idleState = (int32_t)CallStateToApp::CALL_STATE_IDLE;
    ASSERT_EQ(callState, idleState);
    EXPECT_NE(CallManagerGtest::clientPtr_->HasCall(), true);
}

/********************************* Test IsNewCallAllowed() ***************************************/
/**
 * @tc.number   Telephony_CallManager_IsNewCallAllowed_0100
 * @tc.name     The call failed after making a call without the card
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_IsNewCallAllowed_0100, Function | MediumTest | Level3)
{
    int32_t callState = CallManagerGtest::clientPtr_->GetCallState();
    int32_t idleState = (int32_t)CallStateToApp::CALL_STATE_IDLE;
    ASSERT_EQ(callState, idleState);
    EXPECT_EQ(CallManagerGtest::clientPtr_->IsNewCallAllowed(), true);
}

/********************************************* Test IsRinging() ***********************************************/
/**
 * @tc.number   Telephony_CallManager_IsRinging_0100
 * @tc.name     There is no call,return true
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_IsRinging_0100, Function | MediumTest | Level3)
{
    int32_t callState = CallManagerGtest::clientPtr_->GetCallState();
    int32_t idleState = (int32_t)CallStateToApp::CALL_STATE_IDLE;
    ASSERT_EQ(callState, idleState);
    EXPECT_NE(CallManagerGtest::clientPtr_->IsRinging(), true);
}

/***************************************** Test CombineConference() ********************************************/
/**
 * @tc.number   Telephony_CallManager_CombineConference_0100
 * @tc.name     Import callId "@&%￥",test CombineConference(),return non 0
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_CombineConference_0100, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_NEGATIVE_ID;
    EXPECT_NE(CallManagerGtest::clientPtr_->CombineConference(callId), RETURN_VALUE_IS_ZERO)
        << "callId = " << callId;
}

/**
 * @tc.number   Telephony_CallManager_CombineConference_0200
 * @tc.name     Import callId "100",test CombineConference(),return non 0
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_CombineConference_0200, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_POSITIVE_ID;
    EXPECT_NE(CallManagerGtest::clientPtr_->CombineConference(callId), RETURN_VALUE_IS_ZERO)
        << "callId = " << callId;
}

/********************************************* Test GetMainCallId() ***********************************************/
/**
 * @tc.number   Telephony_CallManager_GetMainCallId_0100
 * @tc.name     Import callId "abcd",test GetMainCallId(),return non 0
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_GetMainCallId_0100, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_NEGATIVE_ID;
    EXPECT_NE(CallManagerGtest::clientPtr_->GetMainCallId(callId), RETURN_VALUE_IS_ZERO) << "callId = " << callId;
}

/**
 * @tc.number   Telephony_CallManager_GetMainCallId_0200
 * @tc.name     Import callId "100",test GetMainCallId(),return non 0
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_GetMainCallId_0200, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_POSITIVE_ID;
    EXPECT_NE(CallManagerGtest::clientPtr_->GetMainCallId(callId), RETURN_VALUE_IS_ZERO) << "callId = " << callId;
}

/***************************************** Test GetSubCallIdList() ******************************************/

/**
 * @tc.number   Telephony_CallManager_GetSubCallIdList_0100
 * @tc.name     Import callId "abcd",test GetSubCallIdList(),return non 0
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_GetSubCallIdList_0100, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_NEGATIVE_ID;
    std::vector<std::u16string> ans = CallManagerGtest::clientPtr_->GetSubCallIdList(callId);
    bool isEmpty = ans.empty();
    EXPECT_EQ(isEmpty, true);
    if (!ans.empty()) {
        ans.clear();
    }
}

/**
 * @tc.number   Telephony_CallManager_GetSubCallIdList_0200
 * @tc.name     Import callId "100",test GetSubCallIdList(),return non 0
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_GetSubCallIdList_0200, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_POSITIVE_ID;
    std::vector<std::u16string> ans = CallManagerGtest::clientPtr_->GetSubCallIdList(callId);
    bool isEmpty = ans.empty();
    EXPECT_EQ(isEmpty, true);
    if (!ans.empty()) {
        ans.clear();
    }
}

/************************************ Test GetCallIdListForConference() ***************************************/

/**
 * @tc.number   Telephony_CallManager_GetCallIdListForConference_0100
 * @tc.name     Import callId "abcd",test GetCallIdListForConference(),return non 0
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_GetCallIdListForConference_0100, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_NEGATIVE_ID;
    std::vector<std::u16string> ans = CallManagerGtest::clientPtr_->GetCallIdListForConference(callId);
    bool isEmpty = ans.empty();
    EXPECT_EQ(isEmpty, true);
    if (!ans.empty()) {
        ans.clear();
    }
}

/**
 * @tc.number   Telephony_CallManager_GetCallIdListForConference_0200
 * @tc.name     Import callId "100",test GetCallIdListForConference(),return non 0
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_GetCallIdListForConference_0200, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_POSITIVE_ID;
    std::vector<std::u16string> ans = CallManagerGtest::clientPtr_->GetCallIdListForConference(callId);
    bool isEmpty = ans.empty();
    EXPECT_EQ(isEmpty, true);
    if (!ans.empty()) {
        ans.clear();
    }
}
/************************************* Test IsEmergencyPhoneNumber() ***************************************/
/**
 * @tc.number   Telephony_CallManager_IsEmergencyPhoneNumber_0100
 * @tc.name     Call one phonynumber "0-0-0",test IsEmergencyPhoneNumber(),return false
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_IsEmergencyPhoneNumber_0100, Function | MediumTest | Level3)
{
    int32_t slotId = SIM1_SLOTID;
    std::string number = "0-0-0";
    std::u16string phoneNumber = Str8ToStr16(number);
    int32_t error = CALL_MANAGER_ERROR;
    EXPECT_NE(CallManagerGtest::clientPtr_->IsEmergencyPhoneNumber(phoneNumber, slotId, error), true);
}

/**
 * @tc.number   Telephony_CallManager_IsEmergencyPhoneNumber_0200
 * @tc.name     Call one phonynumber "112",test IsEmergencyPhoneNumber(),return false
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_IsEmergencyPhoneNumber_0200, Function | MediumTest | Level3)
{
    int32_t slotId = SIM1_SLOTID;
    std::string number = "112";
    std::u16string phoneNumber = Str8ToStr16(number);
    int32_t error = CALL_MANAGER_ERROR;
    EXPECT_EQ(CallManagerGtest::clientPtr_->IsEmergencyPhoneNumber(phoneNumber, slotId, error), true);
}

/**
 * @tc.number   Telephony_CallManager_IsEmergencyPhoneNumber_0300
 * @tc.name     Call one phonynumber "911",test IsEmergencyPhoneNumber(),return false
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_IsEmergencyPhoneNumber_0300, Function | MediumTest | Level3)
{
    int32_t slotId = SIM1_SLOTID;
    std::string number = "911";
    std::u16string phoneNumber = Str8ToStr16(number);
    int32_t error = CALL_MANAGER_ERROR;
    EXPECT_EQ(CallManagerGtest::clientPtr_->IsEmergencyPhoneNumber(phoneNumber, slotId, error), true);
}

/**
 * @tc.number   Telephony_CallManager_IsEmergencyPhoneNumber_0400
 * @tc.name     Call one phonynumber "08",test IsEmergencyPhoneNumber(),return false
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_IsEmergencyPhoneNumber_0400, Function | MediumTest | Level3)
{
    int32_t slotId = SIM1_SLOTID_NO_CARD;
    std::string number = "08";
    std::u16string phoneNumber = Str8ToStr16(number);
    int32_t error = CALL_MANAGER_ERROR;
    EXPECT_EQ(CallManagerGtest::clientPtr_->IsEmergencyPhoneNumber(phoneNumber, slotId, error), true);
}

/**
 * @tc.number   Telephony_CallManager_IsEmergencyPhoneNumber_0500
 * @tc.name     Call one phonynumber "118",test IsEmergencyPhoneNumber(),return false
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_IsEmergencyPhoneNumber_0500, Function | MediumTest | Level3)
{
    int32_t slotId = SIM1_SLOTID_NO_CARD;
    std::string number = "118";
    std::u16string phoneNumber = Str8ToStr16(number);
    int32_t error = CALL_MANAGER_ERROR;
    EXPECT_EQ(CallManagerGtest::clientPtr_->IsEmergencyPhoneNumber(phoneNumber, slotId, error), true);
}

/**
 * @tc.number   Telephony_CallManager_IsEmergencyPhoneNumber_0600
 * @tc.name     Call one phonynumber "119",test IsEmergencyPhoneNumber(),return false
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_IsEmergencyPhoneNumber_0600, Function | MediumTest | Level3)
{
    int32_t slotId = SIM1_SLOTID;
    std::string number = "119";
    std::u16string phoneNumber = Str8ToStr16(number);
    int32_t error = CALL_MANAGER_ERROR;
    EXPECT_EQ(CallManagerGtest::clientPtr_->IsEmergencyPhoneNumber(phoneNumber, slotId, error), true);
}

/**
 * @tc.number   Telephony_CallManager_IsEmergencyPhoneNumber_0700
 * @tc.name     Call one phonynumber "999",test IsEmergencyPhoneNumber(),return false
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_IsEmergencyPhoneNumber_0700, Function | MediumTest | Level3)
{
    int32_t slotId = SIM1_SLOTID_NO_CARD;
    std::string number = "999";
    std::u16string phoneNumber = Str8ToStr16(number);
    int32_t error = CALL_MANAGER_ERROR;
    EXPECT_EQ(CallManagerGtest::clientPtr_->IsEmergencyPhoneNumber(phoneNumber, slotId, error), true);
}

/********************************************* Test GetCallWaiting() ***********************************************/

/**
 * @tc.number   Telephony_CallManager_GetCallWaiting_0100
 * @tc.name     Import slotId 1,test GetCallWaiting(),return 0
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_GetCallWaiting_0100, Function | MediumTest | Level3)
{
    int32_t slotId = SIM1_SLOTID;
    EXPECT_EQ(CallManagerGtest::clientPtr_->GetCallWaiting(slotId), RETURN_VALUE_IS_ZERO);
}

/********************************************* Test StartDtmf() ***********************************************/

/**
 * @tc.number   Telephony_CallManager_StartDtmf_0100
 * @tc.name     Import callId abcd,test StartDtmf(),return non 0
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_StartDtmf_0100, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_NEGATIVE_ID;
    char str = '1';
    EXPECT_NE(CallManagerGtest::clientPtr_->StartDtmf(callId, str), RETURN_VALUE_IS_ZERO);
}

/**
 * @tc.number   Telephony_CallManager_StartDtmf_0200
 * @tc.name     Import callId 100, test StartDtmf(),return non 0
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_StartDtmf_0200, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_POSITIVE_ID;
    char str = '1';
    EXPECT_NE(CallManagerGtest::clientPtr_->StartDtmf(callId, str), RETURN_VALUE_IS_ZERO);
}

/********************************************* Test StopDtmf() ***********************************************/

/**
 * @tc.number   Telephony_CallManager_StopDtmf_0100
 * @tc.name     Import callId abcd,test StopDtmf(),return non 0
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_StopDtmf_0100, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_NEGATIVE_ID;
    EXPECT_NE(CallManagerGtest::clientPtr_->StopDtmf(callId), RETURN_VALUE_IS_ZERO);
}

/**
 * @tc.number   Telephony_CallManager_StopDtmf_0200
 * @tc.name     Import callId 100, test StopDtmf(),return non 0
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_StopDtmf_0200, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_POSITIVE_ID;
    EXPECT_NE(CallManagerGtest::clientPtr_->StopDtmf(callId), RETURN_VALUE_IS_ZERO);
}

/********************************************* Test SendDtmf() ***********************************************/

/**
 * @tc.number   Telephony_CallManager_SendDtmf_0100
 * @tc.name     Import callId &%!~ , test SendDtmf(),return non 0
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_SendDtmf_0100, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_NEGATIVE_ID;
    char str = '2';
    EXPECT_NE(CallManagerGtest::clientPtr_->SendDtmf(callId, str), RETURN_VALUE_IS_ZERO);
}

/**
 * @tc.number   Telephony_CallManager_SendDtmf_0200
 * @tc.name     Import callId 100 , test SendDtmf(),return 0
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_SendDtmf_0200, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_POSITIVE_ID;
    char str = '2';
    EXPECT_NE(CallManagerGtest::clientPtr_->SendDtmf(callId, str), RETURN_VALUE_IS_ZERO);
}

/********************************************* Test SendBurstDtmf() ***********************************************/

/**
 * @tc.number   Telephony_CallManager_SendBurstDtmf_0100
 * @tc.name     Import callId abcd,test SendBurstDtmf(),return non 0
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_SendBurstDtmf_0100, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_NEGATIVE_ID;
    std::string key = "1";
    std::u16string str = Str8ToStr16(key);
    int32_t on = 0;
    int32_t off = 0;
    EXPECT_NE(CallManagerGtest::clientPtr_->SendBurstDtmf(callId, str, on, off), RETURN_VALUE_IS_ZERO);
}

/**
 * @tc.number   Telephony_CallManager_SendBurstDtmf_0200
 * @tc.name     Import callId 100 ,test SendBurstDtmf(),return non 0
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_SendBurstDtmf_0200, Function | MediumTest | Level3)
{
    int32_t callId = INVALID_POSITIVE_ID;
    std::string key = "1";
    std::u16string str = Str8ToStr16(key);
    int32_t on = 0;
    int32_t off = 0;
    EXPECT_NE(CallManagerGtest::clientPtr_->SendBurstDtmf(callId, str, on, off), RETURN_VALUE_IS_ZERO);
}

/******************************** Test FormatPhoneNumber() * **************************************/

/**
 * @tc.number   Telephony_CallManager_FormatPhoneNumber_0100
 * @tc.name     Import phonyNumber 01085198748,test FormatPhoneNumber(),return 010-8519-8748
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_FormatPhoneNumber_0100, Function | MediumTest | Level3)
{
    std::string number = "01085198748";
    std::string Code = "Kr";
    std::string formatBefore = "";
    std::u16string phonyNumber = Str8ToStr16(number);
    std::u16string countryCode = Str8ToStr16(Code);
    std::u16string formatNumber = Str8ToStr16(formatBefore);
    EXPECT_EQ(CallManagerGtest::clientPtr_->FormatPhoneNumber(phonyNumber, countryCode, formatNumber),
        RETURN_VALUE_IS_ZERO);
}

/**
 * @tc.number   Telephony_CallManager_FormatPhoneNumber_0200
 * @tc.name     Import countryCode KR,test FormatPhoneNumber(),return 010-8519-8748
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_FormatPhoneNumber_0200, Function | MediumTest | Level3)
{
    std::string number = "010-8519-8748";
    std::string Code = "KR";
    std::string formatBefore = "";
    std::u16string phonyNumber = Str8ToStr16(number);
    std::u16string countryCode = Str8ToStr16(Code);
    std::u16string formatNumber = Str8ToStr16(formatBefore);
    EXPECT_NE(CallManagerGtest::clientPtr_->FormatPhoneNumber(phonyNumber, countryCode, formatNumber),
        RETURN_VALUE_IS_ZERO);
}

/**
 * @tc.number   Telephony_CallManager_FormatPhoneNumber_0300
 * @tc.name     Import phonyNumber (03)38122111,test FormatPhoneNumber(),return 03-3812-2111
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_FormatPhoneNumber_0300, Function | MediumTest | Level3)
{
    std::string number = "(03)38122111";
    std::string Code = "JP";
    std::string formatBefore = "";
    std::u16string phonyNumber = Str8ToStr16(number);
    std::u16string countryCode = Str8ToStr16(Code);
    std::u16string formatNumber = Str8ToStr16(formatBefore);
    EXPECT_EQ(CallManagerGtest::clientPtr_->FormatPhoneNumber(phonyNumber, countryCode, formatNumber),
        RETURN_VALUE_IS_ZERO);
}

/**
 * @tc.number   Telephony_CallManager_FormatPhoneNumber_0400
 * @tc.name     Import phonyNumber 13888888888,test FormatPhoneNumber(),return 138 8888 8888
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_FormatPhoneNumber_0400, Function | MediumTest | Level3)
{
    std::string number = "13888888888";
    std::string Code = "CN";
    std::string formatBefore = "";
    std::u16string phonyNumber = Str8ToStr16(number);
    std::u16string countryCode = Str8ToStr16(Code);
    std::u16string formatNumber = Str8ToStr16(formatBefore);
    EXPECT_EQ(CallManagerGtest::clientPtr_->FormatPhoneNumber(phonyNumber, countryCode, formatNumber),
        RETURN_VALUE_IS_ZERO);
}

/**
 * @tc.number   Telephony_CallManager_FormatPhoneNumber_0500
 * @tc.name     Import phonyNumber +81338122111,test FormatPhoneNumber(),return 03-3812-2111
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_FormatPhoneNumber_0500, Function | MediumTest | Level3)
{
    std::string number = "+81338122111";
    std::string Code = "jp";
    std::string formatBefore = "";
    std::u16string phonyNumber = Str8ToStr16(number);
    std::u16string countryCode = Str8ToStr16(Code);
    std::u16string formatNumber = Str8ToStr16(formatBefore);
    EXPECT_EQ(CallManagerGtest::clientPtr_->FormatPhoneNumber(phonyNumber, countryCode, formatNumber),
        RETURN_VALUE_IS_ZERO);
}

/**
 * @tc.number   Telephony_CallManager_FormatPhoneNumber_0600
 * @tc.name     Import phonyNumber 666666999999,test FormatPhoneNumber(),return 666666999999
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_FormatPhoneNumber_0600, Function | MediumTest | Level3)
{
    std::string number = "666666999999";
    std::string Code = "CN";
    std::string formatBefore = "";
    std::u16string phonyNumber = Str8ToStr16(number);
    std::u16string countryCode = Str8ToStr16(Code);
    std::u16string formatNumber = Str8ToStr16(formatBefore);
    EXPECT_NE(CallManagerGtest::clientPtr_->FormatPhoneNumber(phonyNumber, countryCode, formatNumber),
        RETURN_VALUE_IS_ZERO);
}

/**
 * @tc.number   Telephony_CallManager_FormatPhoneNumber_0700
 * @tc.name     Import countryCode abcdefg,test FormatPhoneNumber(),return 83886082
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_FormatPhoneNumber_0700, Function | MediumTest | Level3)
{
    std::string number = "13888888888";
    std::string Code = "abcdefg";
    std::string formatBefore = "";
    std::u16string phonyNumber = Str8ToStr16(number);
    std::u16string countryCode = Str8ToStr16(Code);
    std::u16string formatNumber = Str8ToStr16(formatBefore);
    EXPECT_NE(CallManagerGtest::clientPtr_->FormatPhoneNumber(phonyNumber, countryCode, formatNumber),
        RETURN_VALUE_IS_ZERO);
}

/******************************* Test FormatPhoneNumberToE164() ***************************************/

/**
 * @tc.number   Telephony_CallManager_FormatPhoneNumberToE164_0100
 * @tc.name     Import phonyNumber 01085198748,test FormatPhoneNumberToE164(),return +821085198748
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_FormatPhoneNumberToE164_0100, Function | MediumTest | Level3)
{
    std::string number = "01085198748";
    std::string Code = "Kr";
    std::string formatBefore = "";
    std::u16string phonyNumber = Str8ToStr16(number);
    std::u16string countryCode = Str8ToStr16(Code);
    std::u16string formatNumber = Str8ToStr16(formatBefore);
    EXPECT_EQ(CallManagerGtest::clientPtr_->FormatPhoneNumberToE164(phonyNumber, countryCode, formatNumber),
        RETURN_VALUE_IS_ZERO);
}

/**
 * @tc.number   Telephony_CallManager_FormatPhoneNumberToE164_0200
 * @tc.name     Import countryCode KR,test FormatPhoneNumberToE164(),return +81338122111
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_FormatPhoneNumberToE164_0200, Function | MediumTest | Level3)
{
    std::string number = "(03)38122111";
    std::string Code = "JP";
    std::string formatBefore = "";
    std::u16string phonyNumber = Str8ToStr16(number);
    std::u16string countryCode = Str8ToStr16(Code);
    std::u16string formatNumber = Str8ToStr16(formatBefore);
    EXPECT_EQ(CallManagerGtest::clientPtr_->FormatPhoneNumberToE164(phonyNumber, countryCode, formatNumber),
        RETURN_VALUE_IS_ZERO);
}

/**
 * @tc.number   Telephony_CallManager_FormatPhoneNumberToE164_0300
 * @tc.name     Import phonyNumber 13888888888,test FormatPhoneNumberToE164(),return +8613888888888
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_FormatPhoneNumberToE164_0300, Function | MediumTest | Level3)
{
    std::string number = "13888888888";
    std::string Code = "cn";
    std::string formatBefore = "";
    std::u16string phonyNumber = Str8ToStr16(number);
    std::u16string countryCode = Str8ToStr16(Code);
    std::u16string formatNumber = Str8ToStr16(formatBefore);
    EXPECT_EQ(CallManagerGtest::clientPtr_->FormatPhoneNumberToE164(phonyNumber, countryCode, formatNumber),
        RETURN_VALUE_IS_ZERO);
}

/**
 * @tc.number   Telephony_CallManager_FormatPhoneNumberToE164_0400
 * @tc.name     Import phonyNumber +81338122111,test FormatPhoneNumberToE164(),return +81338122111
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_FormatPhoneNumberToE164_0400, Function | MediumTest | Level3)
{
    std::string number = "+81338122111";
    std::string Code = "jp";
    std::string formatBefore = "";
    std::u16string phonyNumber = Str8ToStr16(number);
    std::u16string countryCode = Str8ToStr16(Code);
    std::u16string formatNumber = Str8ToStr16(formatBefore);
    EXPECT_NE(CallManagerGtest::clientPtr_->FormatPhoneNumberToE164(phonyNumber, countryCode, formatNumber),
        RETURN_VALUE_IS_ZERO);
}

/**
 * @tc.number   Telephony_CallManager_FormatPhoneNumberToE164_0500
 * @tc.name     Import phonyNumber 03-3812-2111,test FormatPhoneNumberToE164(),return +81338122111
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_FormatPhoneNumberToE164_0500, Function | MediumTest | Level3)
{
    std::string number = "03-3812-2111";
    std::string Code = "JP";
    std::string formatBefore = "";
    std::u16string phonyNumber = Str8ToStr16(number);
    std::u16string countryCode = Str8ToStr16(Code);
    std::u16string formatNumber = Str8ToStr16(formatBefore);
    EXPECT_EQ(CallManagerGtest::clientPtr_->FormatPhoneNumberToE164(phonyNumber, countryCode, formatNumber),
        RETURN_VALUE_IS_ZERO);
}

/**
 * @tc.number   Telephony_CallManager_FormatPhoneNumberToE164_0600
 * @tc.name     Import phonyNumber 666666999999,test FormatPhoneNumberToE164(),return 83886082
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_FormatPhoneNumberToE164_0600, Function | MediumTest | Level3)
{
    std::string number = "666666999999";
    std::string Code = "CN";
    std::string formatBefore = "";
    std::u16string phonyNumber = Str8ToStr16(number);
    std::u16string countryCode = Str8ToStr16(Code);
    std::u16string formatNumber = Str8ToStr16(formatBefore);
    EXPECT_NE(CallManagerGtest::clientPtr_->FormatPhoneNumberToE164(phonyNumber, countryCode, formatNumber),
        RETURN_VALUE_IS_ZERO);
}

/**
 * @tc.number   Telephony_CallManager_FormatPhoneNumberToE164_0700
 * @tc.name     Import countryCode abcdefg,test FormatPhoneNumberToE164(),return 83886082
 * @tc.desc     Function test
 */
HWTEST_F(CallManagerGtest, Telephony_CallManager_FormatPhoneNumberToE164_0700, Function | MediumTest | Level3)
{
    std::string number = "13888888888";
    std::string Code = "abcdefg";
    std::string formatBefore = "";
    std::u16string phonyNumber = Str8ToStr16(number);
    std::u16string countryCode = Str8ToStr16(Code);
    std::u16string formatNumber = Str8ToStr16(formatBefore);
    EXPECT_NE(CallManagerGtest::clientPtr_->FormatPhoneNumberToE164(phonyNumber, countryCode, formatNumber),
        RETURN_VALUE_IS_ZERO);
}
} // namespace Telephony
} // namespace OHOS