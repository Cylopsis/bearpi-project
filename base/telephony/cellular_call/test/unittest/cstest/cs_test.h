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

#ifndef TELEPHONY_CS_TEST_H
#define TELEPHONY_CS_TEST_H

#include "gtest/gtest.h"

#include "cellular_call_data_struct.h"
#include "cellular_call_interface.h"

namespace OHOS {
namespace Telephony {
class CsTest : public testing::Test {
public:
    static void SetUpTestCase();
    static void TearDownTestCase();
    void SetUp();
    void TearDown();

    int Dial(const sptr<CellularCallInterface> &telephonyService) const;
    int End(const sptr<CellularCallInterface> &telephonyService) const;
    int Answer(const sptr<CellularCallInterface> &telephonyService) const;
    int Reject(const sptr<CellularCallInterface> &telephonyService) const;
    int Hold(const sptr<CellularCallInterface> &telephonyService) const;
    int Active(const sptr<CellularCallInterface> &telephonyService) const;
    int Swap(const sptr<CellularCallInterface> &telephonyService) const;
    int IsUrgentCall(const sptr<CellularCallInterface> &telephonyService) const;
    int Join(const sptr<CellularCallInterface> &telephonyService) const;
    int Split(const sptr<CellularCallInterface> &telephonyService) const;
    int CallSupplement(const sptr<CellularCallInterface> &telephonyService) const;
    int InitiateDTMF(const sptr<CellularCallInterface> &telephonyService) const;
    int CeaseDTMF(const sptr<CellularCallInterface> &telephonyService) const;
    int TransmitDTMF(const sptr<CellularCallInterface> &telephonyService) const;
    int TransmitDTMFString(const sptr<CellularCallInterface> &telephonyService) const;
    int SetCallTransfer(const sptr<CellularCallInterface> &telephonyService) const;
    int InquireCallTransfer(const sptr<CellularCallInterface> &telephonyService) const;
    int SetCallWaiting(const sptr<CellularCallInterface> &telephonyService) const;
    int InquireCallWaiting(const sptr<CellularCallInterface> &telephonyService) const;
    int SetCallRestriction(const sptr<CellularCallInterface> &telephonyService) const;
    int InquireCallRestriction(const sptr<CellularCallInterface> &telephonyService) const;
    int RegisterCallBack(const sptr<CellularCallInterface> &telephonyService) const;
    int InputNumForInterface(const sptr<CellularCallInterface> &telephonyService) const;

private:
    using RequestFuncType = int (CsTest::*)(const sptr<CellularCallInterface> &telephonyService) const;
    std::map<char, RequestFuncType> requestFuncMap_;
};
} // namespace Telephony
} // namespace OHOS

#endif // TELEPHONY_CS_TEST_H
