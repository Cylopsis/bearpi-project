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

#ifndef TELEPHONY_CELLULAR_CALL_MMI_HANDLER_H
#define TELEPHONY_CELLULAR_CALL_MMI_HANDLER_H

#include <map>
#include "event_handler.h"
#include "event_runner.h"
#include "telephony_log_wrapper.h"

namespace OHOS {
namespace Telephony {
class MMIHandler : public AppExecFwk::EventHandler {
public:
    /**
     * MMIHandler
     * @param runner
     */
    explicit MMIHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);

    /**
     * ProcessEvent.
     *
     * @param mmi code process
     */
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * ~MMIHandler
     */
    ~MMIHandler() = default;

private:
    /**
     * GetMMIResponse.
     *
     * @param mmi code process
     */
    void GetMMIResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * GetCallWaitResponse.
     *
     * @param mmi code process
     */
    void GetCallWaitResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * SetCallWaitResponse.
     *
     * @param mmi code process
     */
    void SetCallWaitResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * GetClirResponse.
     *
     * @param mmi code process
     */
    void GetClirResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * SetClirResponse.
     *
     * @param mmi code process
     */
    void SetClirResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * GetClipResponse.
     *
     * @param mmi code process
     */
    void GetClipResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * GetCallTransferResponse.
     *
     * @param mmi code process
     */
    void GetCallTransferResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * SetCallTransferResponse.
     *
     * @param mmi code process
     */
    void SetCallTransferResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * GetCallRestrictionResponse.
     *
     * @param mmi code process
     */
    void GetCallRestrictionResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * SetCallRestrictionResponse.
     *
     * @param mmi code process
     */
    void SetCallRestrictionResponse(const AppExecFwk::InnerEvent::Pointer &event);

private:
    using RequestFuncType = void (MMIHandler::*)(const AppExecFwk::InnerEvent::Pointer &event);
    std::map<uint32_t, RequestFuncType> requestFuncMap_;
    static constexpr HiviewDFX::HiLogLabel LOG_LABEL = {LOG_CORE, LOG_DOMAIN, "MMIHandler"};
};
} // namespace Telephony
} // namespace OHOS
#endif // TELEPHONY_CELLULAR_CALL_MMI_HANDLER_H
