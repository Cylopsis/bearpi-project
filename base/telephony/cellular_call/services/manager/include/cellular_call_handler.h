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

#ifndef CELLULAR_CALL_HANDLER_H
#define CELLULAR_CALL_HANDLER_H

#include <memory>

#include "cellular_call_data_struct.h"
#include "telephony_log_wrapper.h"
#include "cs_control.h"

namespace OHOS {
namespace Telephony {
class CellularCallHandler : public AppExecFwk::EventHandler {
public:
    /**
     * CellularCallHandler constructor
     * @param runner
     * @param server
     */
    explicit CellularCallHandler(const std::shared_ptr<AppExecFwk::EventRunner> &runner);

    /**
     * ~CellularCallHandler destructor
     */
    ~CellularCallHandler() = default;

    /**
     * ProcessEvent.
     *
     * @param CellularCallRadioResponseEvent, Process Radio Response Event .
     */
    void ProcessEvent(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * Set Slot Id
     * @param id
     */
    void SetSlotId(int32_t id);

    /**
     * SetCSControl
     */
    void SetCSControl(std::shared_ptr<CSControl>);

    /**
     * GetCSControl
     * @return std::shared_ptr<CSControl>
     */
    std::shared_ptr<CSControl> GetCSControl();

    /**
     * GetCallDataResponse.
     *
     * @param code process
     */
    void GetCallDataResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * DealCallsDataResponse.
     *
     * @param code process
     */
    void DealCallsDataResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * DialResponse.
     *
     * @param code process
     */
    void DialResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * End Response.
     *
     * @param AppExecFwk::InnerEvent::Pointer
     */
    void EndResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * Reject Response.
     *
     * @param AppExecFwk::InnerEvent::Pointer
     */
    void RejectResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * Accept Response.
     *
     * @param AppExecFwk::InnerEvent::Pointer
     */
    void AcceptResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * Hold Response.
     *
     * @param AppExecFwk::InnerEvent::Pointer
     */
    void HoldResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * Active Response.
     *
     * @param AppExecFwk::InnerEvent::Pointer
     */
    void ActiveResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * Swap Response.
     *
     * @param AppExecFwk::InnerEvent::Pointer
     */
    void SwapResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * Join Response.
     *
     * @param AppExecFwk::InnerEvent::Pointer
     */
    void JoinResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * Split Response.
     *
     * @param AppExecFwk::InnerEvent::Pointer
     */
    void SplitResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * Supplement Response.
     *
     * @param AppExecFwk::InnerEvent::Pointer
     */
    void SupplementResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * SendDTMFResponse.
     *
     * @param code process
     */
    void SendDTMFResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * StartDTMFResponse.
     *
     * @param code process
     */
    void StartDTMFResponse(const AppExecFwk::InnerEvent::Pointer &event);

    /**
     * StopDTMFResponse.
     *
     * @param code process
     */
    void StopDTMFResponse(const AppExecFwk::InnerEvent::Pointer &event);

public:
    const uint32_t ASYNCHRONOUS_REGISTER_ID = 1002;

private:
    /**
     * Prevent violent requests
     * @return bool
     */
    bool IsCanRequestCallsData();

    /**
     * Current Time Millis
     * @return int64_t Time Millis
     */
    int64_t CurrentTimeMillis();

    /**
     * DealCallsDataRequest
     */
    void DealCallsDataRequest(const AppExecFwk::InnerEvent::Pointer &event);
    /**
     * ThreadRegister
     */
    void AsynchronousRegister(const AppExecFwk::InnerEvent::Pointer &event);

private:
    int32_t slotId_ = CoreManager::DEFAULT_SLOT_ID;
    std::shared_ptr<CSControl> pCSControl_;
    int64_t lastTime_ = 0L;
    int64_t FAST_DELAY_TIME = 250;
    int64_t lastCallsDataFlag_ = 0L;
    const uint32_t GET_CALL_DATA_ID = 1001;
    int64_t delayTime_ = 500;
    using RequestFuncType = void (CellularCallHandler::*)(const AppExecFwk::InnerEvent::Pointer &event);
    std::map<uint32_t, RequestFuncType> requestFuncMap_;
    static constexpr HiviewDFX::HiLogLabel LOG_LABEL = {LOG_CORE, LOG_DOMAIN, "CellularCallHandler"};
};
} // namespace Telephony
} // namespace OHOS

#endif
