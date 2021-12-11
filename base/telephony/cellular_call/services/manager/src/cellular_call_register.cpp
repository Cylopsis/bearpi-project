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

#include "cellular_call_register.h"

#include "call_status_callback_proxy.h"

namespace OHOS {
namespace Telephony {
CellularCallRegister::CellularCallRegister() : callManagerCallBack_(nullptr) {}
CellularCallRegister::~CellularCallRegister() = default;

void CellularCallRegister::ReportCallsInfo(const CallsReportInfo &callsReportInfo)
{
    TELEPHONY_LOGD("ReportCallsInfo entry.");
    if (callManagerCallBack_ == nullptr) {
        TELEPHONY_LOGE("ReportCallsInfo return, callManagerCallBack_ is nullptr, report fail!");
        return;
    }
    callManagerCallBack_->OnUpdateCallsReportInfo(callsReportInfo);
}

int32_t CellularCallRegister::RegisterCallManagerCallBack(const sptr<ICallStatusCallback> &callback)
{
    callManagerCallBack_ = callback;
    return TELEPHONY_SUCCESS;
}

void CellularCallRegister::ReportSingleCallInfo(CellularCallConnectionCS &pCs, TelCallState callState)
{
    TELEPHONY_LOGD("ReportSingleCallInfo entry");
    CallReportInfo cellularCallReportInfo = pCs.GetCallReportInfo();
    cellularCallReportInfo.state = callState;
    if (callManagerCallBack_ == nullptr) {
        TELEPHONY_LOGE("ReportSingleCallInfo return, callManagerCallBack_ is nullptr, report fail!");
        return;
    }
    callManagerCallBack_->OnUpdateCallReportInfo(cellularCallReportInfo);
}

int32_t CellularCallRegister::UnRegisterCallManagerCallBack()
{
    callManagerCallBack_ = nullptr;
    return TELEPHONY_SUCCESS;
}

void CellularCallRegister::ReportEventResultInfo(CellularCallEventInfo &info)
{
    TELEPHONY_LOGD("ReportEventResultInfo entry");
    if (callManagerCallBack_ == nullptr) {
        TELEPHONY_LOGE("ReportEventResultInfo return, callManagerCallBack_ is nullptr, report fail!");
        return;
    }
    callManagerCallBack_->OnUpdateEventResultInfo(info);
}

void CellularCallRegister::ReportGetWaitingResult(CallWaitResponse &callWaitResponse)
{
    TELEPHONY_LOGD("ReportGetWaitingResult entry");
    if (callManagerCallBack_ == nullptr) {
        TELEPHONY_LOGE("ReportGetWaitingResult return, callManagerCallBack_ is nullptr, report fail!");
        return;
    }
    callManagerCallBack_->OnUpdateGetWaitingResult(callWaitResponse);
}

void CellularCallRegister::ReportSetWaitingResult(CallWaitResponse &callWaitResponse)
{
    TELEPHONY_LOGD("ReportSetWaitingResult entry");
    if (callManagerCallBack_ == nullptr) {
        TELEPHONY_LOGE("ReportSetWaitingResult return, callManagerCallBack_ is nullptr, report fail!");
        return;
    }
    callManagerCallBack_->OnUpdateSetWaitingResult(callWaitResponse.result);
}

void CellularCallRegister::ReportGetRestrictionResult(const CallRestrictionResponse &response)
{
    TELEPHONY_LOGD("ReportGetRestrictionResult entry");
    if (callManagerCallBack_ == nullptr) {
        TELEPHONY_LOGE("ReportGetRestrictionResult return, callManagerCallBack_ is nullptr, report fail!");
        return;
    }
    callManagerCallBack_->OnUpdateGetRestrictionResult(response);
}

void CellularCallRegister::ReportSetRestrictionResult(int32_t result)
{
    TELEPHONY_LOGD("ReportSetRestrictionResult entry");
    if (callManagerCallBack_ == nullptr) {
        TELEPHONY_LOGE("ReportSetRestrictionResult return, callManagerCallBack_ is nullptr, report fail!");
        return;
    }
    callManagerCallBack_->OnUpdateSetRestrictionResult(result);
}

void CellularCallRegister::ReportGetTransferResult(CallTransferResponse &callTransferResponse)
{
    TELEPHONY_LOGD("ReportGetTransferResult entry");
    if (callManagerCallBack_ == nullptr) {
        TELEPHONY_LOGE("ReportGetTransferResult return, callManagerCallBack_ is nullptr, report fail!");
        return;
    }
    callManagerCallBack_->OnUpdateGetTransferResult(callTransferResponse);
}

void CellularCallRegister::ReportSetTransferResult(int32_t result)
{
    TELEPHONY_LOGD("ReportSetTransferResult entry");
    if (callManagerCallBack_ == nullptr) {
        TELEPHONY_LOGE("ReportSetTransferResult return, callManagerCallBack_ is nullptr, report fail!");
        return;
    }
    callManagerCallBack_->OnUpdateSetTransferResult(result);
}

void CellularCallRegister::ReportGetClipResult(ClipResponse &clipResponse)
{
    TELEPHONY_LOGD("ReportGetClipResult entry");
    if (callManagerCallBack_ == nullptr) {
        TELEPHONY_LOGE("ReportGetClipResult return, callManagerCallBack_ is nullptr, report fail!");
        return;
    }
    callManagerCallBack_->OnUpdateGetCallClipResult(clipResponse);
}

void CellularCallRegister::ReportGetClirResult(ClirResponse &clirResponse)
{
    TELEPHONY_LOGD("ReportGetClirResult entry");
    if (callManagerCallBack_ == nullptr) {
        TELEPHONY_LOGE("ReportSetTransferResult return, callManagerCallBack_ is nullptr, report fail!");
        return;
    }
    callManagerCallBack_->OnUpdateGetCallClirResult(clirResponse);
}

void CellularCallRegister::ReportSetClirResult(ClirResponse &clirResponse)
{
    TELEPHONY_LOGD("ReportSetClirResult entry");
    if (callManagerCallBack_ == nullptr) {
        TELEPHONY_LOGE("ReportSetTransferResult return, callManagerCallBack_ is nullptr, report fail!");
        return;
    }
    callManagerCallBack_->OnUpdateSetCallClirResult(clirResponse.result);
}
} // namespace Telephony
} // namespace OHOS