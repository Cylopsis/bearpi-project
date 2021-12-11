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

#ifndef CELLULAR_CALL_INFO_HANDLER_H
#define CELLULAR_CALL_INFO_HANDLER_H

#include <memory>
#include <mutex>

#include "event_handler.h"
#include "event_runner.h"
#include "singleton.h"

#include "call_status_manager.h"

namespace OHOS {
namespace Telephony {
class CellularCallInfoHandler : public AppExecFwk::EventHandler {
public:
    CellularCallInfoHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);
    virtual ~CellularCallInfoHandler();
    void Init();
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event);

private:
    using CallManagerServiceFunc = void (CellularCallInfoHandler::*)(const AppExecFwk::InnerEvent::Pointer &event);
    void ReportCallInfo(const AppExecFwk::InnerEvent::Pointer &event);
    void ReportCallsInfo(const AppExecFwk::InnerEvent::Pointer &event);
    void ReportEventInfo(const AppExecFwk::InnerEvent::Pointer &event);
    std::map<uint32_t, CallManagerServiceFunc> memberFuncMap_;
    std::unique_ptr<CallStatusManager> callStatusManagerPtr_;
};

class CellularCallInfoHandlerService : public std::enable_shared_from_this<CellularCallInfoHandlerService> {
    DECLARE_DELAYED_SINGLETON(CellularCallInfoHandlerService)
public:
    void Start();
    int32_t UpdateCallReportInfo(const CallReportInfo &info);
    int32_t UpdateCallsReportInfo(const CallsReportInfo &info);
    int32_t UpdateEventResultInfo(const CellularCallEventInfo &info);

    enum CellularCallInfoInterfaceType {
        HANDLER_UPDATE_CELLULAR_CALL_INFO = 0,
        HANDLER_UPDATE_CELLULAR_CS_CALL_INFO,
        HANDLER_UPDATE_CELLULAR_EVENT_RESULT_INFO,
    };

private:
    std::shared_ptr<AppExecFwk::EventRunner> eventLoop_;
    std::shared_ptr<CellularCallInfoHandler> handler_;
    std::mutex mutex_;
};
} // namespace Telephony
} // namespace OHOS

#endif // CELLULAR_CALL_INFO_HANDLER_H
