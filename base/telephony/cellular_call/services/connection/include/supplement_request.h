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

#ifndef CELLULAR_CALL_SUPPLEMENT_REQUEST_H
#define CELLULAR_CALL_SUPPLEMENT_REQUEST_H

#include "core_manager.h"
#include "mmi_handler.h"

namespace OHOS {
namespace Telephony {
class SupplementRequest {
public:
    /**
     * Inquire Clip Request
     */
    int32_t InquireClipRequest();

    /**
     * Clir Activate Request
     * @param action
     */
    int32_t SetClirActivateRequest(int32_t action);

    /**
     * Set Clir Deactivate Request
     * @param action
     */
    int32_t SetClirDeactivateRequest(int32_t action);

    /**
     * Inquire Clir Request
     */
    int32_t InquireClirRequest();

    /**
     * Inquire Call Transfer
     *
     * @param type
     */
    int32_t InquireCallTransferRequest(int type);

    /**
     * Set Call Transfer Request
     * @param action
     * @param cause
     * @param transferPhone
     * @param serviceInfoB
     */
    int32_t SetCallTransferRequest(int action, int cause, const std::string &transferPhone, int serviceInfoB);

    /**
     * Inquire Call Restriction
     * @param fac
     */
    int32_t InquireCallRestriction(const std::string &fac);

    /**
     * Set Call Restriction
     * @param fac
     * @param mode
     * @param pw
     */
    int32_t SetCallRestriction(std::string &fac, int32_t mode, std::string &pw);

    /**
     * Set CallWaiting Request
     * @param is activate
     */
    int32_t SetCallWaitingRequest(bool activate);

    /**
     * Inquire Call Waiting Request
     * @param serviceCode
     */
    int32_t InquireCallWaitingRequest();

private:
    /**
     * Get Core for core service
     * @param slotId
     * @return Core for core service
     */
    std::shared_ptr<Core> GetCore(int32_t slotId);

    /**
     * GetMMIHandler
     *
     * @return std::shared_ptr<MMIHandler>
     */
    std::shared_ptr<MMIHandler> GetMMIHandler() const;

private:
    int slotId_ = CoreManager::DEFAULT_SLOT_ID;
    static constexpr HiviewDFX::HiLogLabel LOG_LABEL = {LOG_CORE, LOG_DOMAIN, "SupplementRequest"};
};
} // namespace Telephony
} // namespace OHOS
#endif // CELLULAR_CALL_SUPPLEMENT_REQUEST_H
