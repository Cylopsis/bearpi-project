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

#include "call_status_callback.h"

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

#include "call_ability_report_ipc_proxy.h"
#include "cellular_call_info_handler.h"

namespace OHOS {
namespace Telephony {
CallStatusCallback::CallStatusCallback() {}

CallStatusCallback::~CallStatusCallback() {}

int32_t CallStatusCallback::OnUpdateCallReportInfo(const CallReportInfo &info)
{
    int32_t ret = DelayedSingleton<CellularCallInfoHandlerService>::GetInstance()->UpdateCallReportInfo(info);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("UpdateCallReportInfo failed! errCode:%{public}d", ret);
    } else {
        TELEPHONY_LOGI("UpdateCallReportInfo success! state:%d", info.state);
    }
    return ret;
}

int32_t CallStatusCallback::OnUpdateCallsReportInfo(const CallsReportInfo &info)
{
    int32_t ret = DelayedSingleton<CellularCallInfoHandlerService>::GetInstance()->UpdateCallsReportInfo(info);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("UpdateCallsReportInfo failed! errCode:%{public}d", ret);
    } else {
        TELEPHONY_LOGI("UpdateCallsReportInfo success!");
    }
    return ret;
}

int32_t CallStatusCallback::OnUpdateDisconnectedCause(const DisconnectedDetails &cause)
{
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusCallback::OnUpdateEventResultInfo(const CellularCallEventInfo &info)
{
    int32_t ret = DelayedSingleton<CellularCallInfoHandlerService>::GetInstance()->UpdateEventResultInfo(info);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("OnUpdateEventResultInfo failed! errCode:%{public}d", ret);
    } else {
        TELEPHONY_LOGI("OnUpdateEventResultInfo success!");
    }
    return ret;
}

int32_t CallStatusCallback::OnUpdateGetWaitingResult(const CallWaitResponse &response)
{
    TELEPHONY_LOGI("GetWaitingResult  status = %{public}d, classCw = %{public}d, result = %{public}d",
        response.status, response.classCw, response.result);
    CallResultReportId reportId = CallResultReportId::GET_CALL_WAITING_REPORT_ID;
    AppExecFwk::PacMap resultInfo;
    resultInfo.PutIntValue("result", response.result);
    resultInfo.PutIntValue("status", response.status);
    resultInfo.PutIntValue("classCw", response.classCw);
    DelayedSingleton<CallAbilityReportIpcProxy>::GetInstance()->ReportSupplementResult(reportId, resultInfo);
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusCallback::OnUpdateSetWaitingResult(int32_t result)
{
    TELEPHONY_LOGI("SetWaitingResult  result = %{public}d", result);
    CallResultReportId reportId = CallResultReportId::SET_CALL_WAITING_REPORT_ID;
    AppExecFwk::PacMap resultInfo;
    resultInfo.PutIntValue("result", result);
    DelayedSingleton<CallAbilityReportIpcProxy>::GetInstance()->ReportSupplementResult(reportId, resultInfo);
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusCallback::OnUpdateGetRestrictionResult(const CallRestrictionResponse &result)
{
    TELEPHONY_LOGI("GetRestrictionResult  status = %{public}d, classCw = %{public}d, result = %{public}d",
        result.status, result.classCw, result.result);
    CallResultReportId reportId = CallResultReportId::GET_CALL_RESTRICTION_REPORT_ID;
    AppExecFwk::PacMap resultInfo;
    resultInfo.PutIntValue("result", result.result);
    resultInfo.PutIntValue("status", result.status);
    resultInfo.PutIntValue("classCw", result.classCw);
    return DelayedSingleton<CallAbilityReportIpcProxy>::GetInstance()->ReportSupplementResult(reportId, resultInfo);
}

int32_t CallStatusCallback::OnUpdateSetRestrictionResult(int32_t result)
{
    TELEPHONY_LOGI("SetRestrictionResult  result = %{public}d", result);
    CallResultReportId reportId = CallResultReportId::SET_CALL_RESTRICTION_REPORT_ID;
    AppExecFwk::PacMap resultInfo;
    resultInfo.PutIntValue("result", result);
    return DelayedSingleton<CallAbilityReportIpcProxy>::GetInstance()->ReportSupplementResult(reportId, resultInfo);
}

int32_t CallStatusCallback::OnUpdateGetTransferResult(const CallTransferResponse &response)
{
    CallResultReportId reportId = CallResultReportId::GET_CALL_TRANSFER_REPORT_ID;
    AppExecFwk::PacMap resultInfo;
    resultInfo.PutIntValue("result", response.result);
    resultInfo.PutIntValue("status", response.status);
    resultInfo.PutIntValue("classx", response.classx);
    resultInfo.PutStringValue("number", response.number);
    resultInfo.PutIntValue("type", response.type);
    return DelayedSingleton<CallAbilityReportIpcProxy>::GetInstance()->ReportSupplementResult(reportId, resultInfo);
}

int32_t CallStatusCallback::OnUpdateSetTransferResult(int32_t result)
{
    TELEPHONY_LOGI("SetTransferResult  result = %{public}d", result);
    CallResultReportId reportId = CallResultReportId::GET_CALL_TRANSFER_REPORT_ID;
    AppExecFwk::PacMap resultInfo;
    resultInfo.PutIntValue("result", result);
    return DelayedSingleton<CallAbilityReportIpcProxy>::GetInstance()->ReportSupplementResult(reportId, resultInfo);
}

int32_t CallStatusCallback::OnUpdateGetCallClipResult(const ClipResponse &clipResponse)
{
    TELEPHONY_LOGI("GetCallClipResult  action = %{public}d, clipStat = %{public}d, result = %{public}d",
        clipResponse.action, clipResponse.clipStat, clipResponse.result);
    CallResultReportId reportId = CallResultReportId::GET_CALL_CLIP_ID;
    AppExecFwk::PacMap resultInfo;
    resultInfo.PutIntValue("result", clipResponse.result);
    resultInfo.PutIntValue("action", clipResponse.action);
    resultInfo.PutIntValue("clipStat", clipResponse.clipStat);
    return DelayedSingleton<CallAbilityReportIpcProxy>::GetInstance()->ReportSupplementResult(reportId, resultInfo);
}

int32_t CallStatusCallback::OnUpdateGetCallClirResult(const ClirResponse &clirResponse)
{
    TELEPHONY_LOGI("GetCallClirResult  action = %{public}d, clirStat = %{public}d, result = %{public}d",
        clirResponse.action, clirResponse.clirStat, clirResponse.result);
    CallResultReportId reportId = CallResultReportId::GET_CALL_CLIR_ID;
    AppExecFwk::PacMap resultInfo;
    resultInfo.PutIntValue("result", clirResponse.result);
    resultInfo.PutIntValue("action", clirResponse.action);
    resultInfo.PutIntValue("clirStat", clirResponse.clirStat);
    return DelayedSingleton<CallAbilityReportIpcProxy>::GetInstance()->ReportSupplementResult(reportId, resultInfo);
}

int32_t CallStatusCallback::OnUpdateSetCallClirResult(int32_t result)
{
    TELEPHONY_LOGI("GetCallClirResult  result = %{public}d", result);
    CallResultReportId reportId = CallResultReportId::SET_CALL_CLIR_ID;
    AppExecFwk::PacMap resultInfo;
    resultInfo.PutIntValue("result", result);
    return DelayedSingleton<CallAbilityReportIpcProxy>::GetInstance()->ReportSupplementResult(reportId, resultInfo);
}
} // namespace Telephony
} // namespace OHOS