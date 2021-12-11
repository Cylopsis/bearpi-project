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

#include "cs_test.h"

#include <iostream>

#include "iservice_registry.h"
#include "securec.h"
#include "system_ability_definition.h"

namespace OHOS {
namespace Telephony {
using namespace testing::ext;
const int CELLULAR_CALL_SUCCESS = 0;
void CsTest::SetUpTestCase(void)
{
    // step 3: Set Up Test Case
}

void CsTest::TearDownTestCase(void)
{
    // step 3: Tear Down Test Case
}

void CsTest::SetUp(void)
{
    // step 3: input testcase setup step
    requestFuncMap_['A'] = &CsTest::Dial;
    requestFuncMap_['B'] = &CsTest::End;
    requestFuncMap_['C'] = &CsTest::Answer;
    requestFuncMap_['D'] = &CsTest::Reject;
    requestFuncMap_['E'] = &CsTest::Hold;
    requestFuncMap_['F'] = &CsTest::Active;
    requestFuncMap_['G'] = &CsTest::Swap;
    requestFuncMap_['H'] = &CsTest::IsUrgentCall;
    requestFuncMap_['I'] = &CsTest::Join;
    requestFuncMap_['J'] = &CsTest::Split;
    requestFuncMap_['K'] = &CsTest::CallSupplement;
    requestFuncMap_['L'] = &CsTest::InitiateDTMF;
    requestFuncMap_['M'] = &CsTest::CeaseDTMF;
    requestFuncMap_['N'] = &CsTest::TransmitDTMF;
    requestFuncMap_['O'] = &CsTest::TransmitDTMFString;
    requestFuncMap_['P'] = &CsTest::SetCallTransfer;
    requestFuncMap_['Q'] = &CsTest::InquireCallTransfer;
    requestFuncMap_['R'] = &CsTest::SetCallWaiting;
    requestFuncMap_['S'] = &CsTest::InquireCallWaiting;
    requestFuncMap_['T'] = &CsTest::SetCallRestriction;
    requestFuncMap_['U'] = &CsTest::InquireCallRestriction;
    requestFuncMap_['V'] = &CsTest::RegisterCallBack;
}

void CsTest::TearDown(void)
{
    // step 3: input testcase teardown step
}

int CsTest::Dial(const sptr<CellularCallInterface> &telephonyService) const
{
    printf("test Dial entry.\n");
    CellularCallInfo callInfo;
    if (memset_s(&callInfo, sizeof(callInfo), 0, sizeof(callInfo)) != EOK) {
        printf("CellularCallService return, memset_s failed. \n");
        return ERR_PARAMETER_INVALID;
    }
    std::cout << "please enter the phone number:";
    std::cin >> callInfo.phoneNum;
    callInfo.videoState = 1;
    callInfo.callType = TYPE_CS;
    callInfo.slotId = 0;
    telephonyService->Dial(callInfo);
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::End(const sptr<CellularCallInterface> &telephonyService) const
{
    printf("test End entry.\n");
    CellularCallInfo callInfo;
    if (memset_s(&callInfo, sizeof(callInfo), 0, sizeof(callInfo)) != EOK) {
        printf("CellularCallService return, memset_s failed. \n");
        return ERR_PARAMETER_INVALID;
    }

    std::cout << "please enter the phone number:";
    std::cin >> callInfo.phoneNum;
    callInfo.videoState = 1;
    callInfo.callType = TYPE_CS;
    callInfo.slotId = 0;
    std::cout << "please enter the connect index:";
    std::cin >> callInfo.index;

    telephonyService->End(callInfo);
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::Answer(const sptr<CellularCallInterface> &telephonyService) const
{
    printf("test Answer entry.\n");
    CellularCallInfo callInfo;
    if (memset_s(&callInfo, sizeof(callInfo), 0, sizeof(callInfo)) != EOK) {
        printf("CellularCallService return, memset_s failed. \n");
        return ERR_PARAMETER_INVALID;
    }

    std::cout << "please enter the phone number:";
    std::cin >> callInfo.phoneNum;
    callInfo.videoState = 1;
    callInfo.callType = TYPE_CS;
    callInfo.slotId = 0;

    telephonyService->Answer(callInfo);
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::Reject(const sptr<CellularCallInterface> &telephonyService) const
{
    printf("test Reject entry.\n");
    CellularCallInfo callInfo;
    if (memset_s(&callInfo, sizeof(callInfo), 0, sizeof(callInfo)) != EOK) {
        printf("CellularCallService return, memset_s failed. \n");
        return ERR_PARAMETER_INVALID;
    }

    std::cout << "please enter the phone number:";
    std::cin >> callInfo.phoneNum;
    callInfo.videoState = 1;
    callInfo.callType = TYPE_CS;
    callInfo.slotId = 0;
    std::cout << "please enter the connect index:";
    std::cin >> callInfo.index;

    telephonyService->Reject(callInfo);
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::Hold(const sptr<CellularCallInterface> &telephonyService) const
{
    printf("test Hold entry.\n");
    telephonyService->Hold();
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::Active(const sptr<CellularCallInterface> &telephonyService) const
{
    printf("test Active entry.\n");
    telephonyService->Active();
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::Swap(const sptr<CellularCallInterface> &telephonyService) const
{
    printf("test Swap entry.\n");
    telephonyService->Swap();
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::IsUrgentCall(const sptr<CellularCallInterface> &telephonyService) const
{
    printf("test IsUrgentCall entry.\n");
    std::string phoneNum = "110";
    int32_t slotId = 0;
    int32_t errorCode = 0;
    telephonyService->IsUrgentCall(phoneNum, slotId, errorCode);
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::Join(const sptr<CellularCallInterface> &telephonyService) const
{
    std::cout << "test Join entry." << std::endl;
    telephonyService->Join();
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::Split(const sptr<CellularCallInterface> &telephonyService) const
{
    std::cout << "test Split entry." << std::endl;
    std::cout << "please enter the need split phone number:";
    std::string phoneNum;
    std::cin >> phoneNum;
    std::cout << "please enter the need split index:";
    int32_t index;
    std::cin >> index;
    telephonyService->Split(phoneNum, index);
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::CallSupplement(const sptr<CellularCallInterface> &telephonyService) const
{
    std::cout << "test CallSupplement entry." << std::endl;
    std::cout << "please enter the type:";
    int32_t type;
    std::cin >> type;
    telephonyService->CallSupplement(static_cast<CallSupplementType>(type));
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::InitiateDTMF(const sptr<CellularCallInterface> &telephonyService) const
{
    std::cout << "test InitiateDTMF entry." << std::endl;
    char cDTMFCode;
    std::cout << "please enter the DTMF code:";
    std::cin >> cDTMFCode;
    std::string phoneNum;
    std::cout << "please enter the phone number:";
    std::cin >> phoneNum;
    telephonyService->InitiateDTMF(cDTMFCode, phoneNum);
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::CeaseDTMF(const sptr<CellularCallInterface> &telephonyService) const
{
    std::cout << "test CeaseDTMF entry." << std::endl;
    std::string phoneNum;
    std::cout << "please enter the phone number:";
    std::cin >> phoneNum;
    telephonyService->CeaseDTMF(phoneNum);
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::TransmitDTMF(const sptr<CellularCallInterface> &telephonyService) const
{
    std::cout << "test TransmitDTMF entry." << std::endl;
    char cDTMFCode;
    std::cout << "please enter the DTMF code:";
    std::cin >> cDTMFCode;
    std::string phoneNum;
    std::cout << "please enter the phone number:";
    std::cin >> phoneNum;
    telephonyService->TransmitDTMF(cDTMFCode, phoneNum);
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::TransmitDTMFString(const sptr<CellularCallInterface> &telephonyService) const
{
    std::cout << "test TransmitDTMFString entry." << std::endl;
    std::string dtmfCodeStr;
    std::cout << "please enter the DTMF code string:";
    std::cin >> dtmfCodeStr;
    std::string phoneNum;
    std::cout << "please enter the phone number:";
    std::cin >> phoneNum;
    int32_t switchOn;
    std::cout << "please enter the switchOn:";
    std::cin >> switchOn;
    int32_t switchOff;
    std::cout << "please enter the switchOff:";
    std::cin >> switchOff;
    telephonyService->TransmitDTMFString(dtmfCodeStr, phoneNum, switchOn, switchOff);
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::SetCallTransfer(const sptr<CellularCallInterface> &telephonyService) const
{
    printf("test SetCallTransfer entry.\n");
    int32_t slotId = 0;
    CallTransferInfo callTransferInfo;
    std::cout << "please enter the transfer type(0 - 5):";
    int32_t type = 0;
    std::cin >> type;
    callTransferInfo.type = static_cast<CallTransferType>(type);
    std::cout << "please enter the transfer setting type(0 - 4):";
    int32_t settingType = 0;
    std::cin >> settingType;
    callTransferInfo.settingType = static_cast<CallTransferSettingType>(settingType);
    std::cout << "please enter the phone number:";
    std::cin >> callTransferInfo.transferNum;
    telephonyService->SetCallTransfer(callTransferInfo, slotId);
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::InquireCallTransfer(const sptr<CellularCallInterface> &telephonyService) const
{
    printf("test InquireCallTransfer entry.\n");
    int32_t slotId = 0;
    std::cout << "please enter the transfer type(0 - 5):";
    int32_t type = 0;
    std::cin >> type;
    telephonyService->InquireCallTransfer(static_cast<CallTransferType>(type), slotId);
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::SetCallWaiting(const sptr<CellularCallInterface> &telephonyService) const
{
    printf("test SetCallWaiting entry.\n");
    bool activate = true;
    int32_t slotId = 0;
    telephonyService->SetCallWaiting(activate, slotId);
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::InquireCallWaiting(const sptr<CellularCallInterface> &telephonyService) const
{
    printf("test InquireCallWaiting entry.\n");
    int32_t slotId = 0;
    telephonyService->InquireCallWaiting(slotId);
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::SetCallRestriction(const sptr<CellularCallInterface> &telephonyService) const
{
    printf("test SetCallRestriction entry.\n");
    int32_t slotId = 0;
    CallRestrictionInfo info;
    std::cout << "please enter the fac(0 - 4):";
    int32_t fac = 0;
    std::cin >> fac;
    info.fac = static_cast<CallRestrictionType>(fac);
    std::cout << "please enter the phone password:";
    std::cin >> info.password;
    std::cout << "please enter the mode:";
    int32_t mode = 0;
    std::cin >> mode;
    info.mode = static_cast<CallRestrictionMode>(mode);
    telephonyService->SetCallRestriction(info, slotId);
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::InquireCallRestriction(const sptr<CellularCallInterface> &telephonyService) const
{
    printf("test InquireCallRestriction entry.\n");
    int32_t slotId = 0;
    std::cout << "please enter the fac(0 - 4):";
    int32_t facType = 0;
    std::cin >> facType;
    telephonyService->InquireCallRestriction(static_cast<CallRestrictionType>(facType), slotId);
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::RegisterCallBack(const sptr<CellularCallInterface> &telephonyService) const
{
    printf("test RegisterCallBack entry.\n");
    return CELLULAR_CALL_SUCCESS;
}

int CsTest::InputNumForInterface(const sptr<CellularCallInterface> &telephonyService) const
{
    char interfaceNum = '0';
    bool loopFlag = true;
    int ret = -1;
    while (loopFlag) {
        printf(
            "\n**********Unit Test Start**********\n"
            "Usage: please input a cmd num:\n"
            "A:Dial\n"
            "B:End\n"
            "C:Answer\n"
            "D:Reject\n"
            "E:Hold\n"
            "F:Active\n"
            "G:Swap\n"
            "H:IsUrgentCall\n"
            "I:Join\n"
            "J:Split\n"
            "K:CallSupplement\n"
            "L:InitiateDTMF\n"
            "M:CeaseDTMF\n"
            "N:TransmitDTMF\n"
            "O:TransmitDTMFString\n"
            "P:SetCallTransfer\n"
            "Q:InquireCallTransfer\n"
            "R:SetCallWaiting\n"
            "S:InquireCallWaiting\n"
            "T:SetCallRestriction\n"
            "U:InquireCallRestriction\n"
            "V:RegisterCallBack\n"
            "W:Exit\n"
            "***********************************\n"
            "Your choice: ");
        std::cin >> interfaceNum;
        if (interfaceNum == 'W') {
            break;
        }
        auto itFunc = requestFuncMap_.find(interfaceNum);
        if (itFunc != requestFuncMap_.end()) {
            auto requestFunc = itFunc->second;
            if (requestFunc != nullptr) {
                ret = (this->*requestFunc)(telephonyService);
            }
        }
        if (ret != CELLULAR_CALL_SUCCESS) {
            return ret;
        }
    }
    return 0;
}

HWTEST_F(CsTest, cellular_call_interface_001, TestSize.Level1)
{
    sptr<ISystemAbilityManager> systemAbilityMgr =
        SystemAbilityManagerClient::GetInstance().GetSystemAbilityManager();
    if (systemAbilityMgr == nullptr) {
        printf("CellularCallService Get ISystemAbilityManager failed.\n");
        return;
    }
    sptr<IRemoteObject> remote = systemAbilityMgr->CheckSystemAbility(TELEPHONY_CELLULAR_CALL_SYS_ABILITY_ID);
    if (remote == nullptr) {
        printf("CellularCallService Remote service not exists.\n");
        return;
    }
    sptr<CellularCallInterface> telephonyService = iface_cast<CellularCallInterface>(remote);
    printf("HWTEST_F cellular_call_interface_001");
}
} // namespace Telephony
} // namespace OHOS