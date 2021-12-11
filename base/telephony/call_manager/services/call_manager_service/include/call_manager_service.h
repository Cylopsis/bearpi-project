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

#ifndef CALL_MANAGER_SERVICE_H
#define CALL_MANAGER_SERVICE_H

#include <memory>

#include "iremote_stub.h"
#include "iservice_registry.h"
#include "singleton.h"
#include "system_ability.h"
#include "system_ability_definition.h"

#include "call_number_utils.h"
#include "call_setting_manager.h"
#include "call_state_report_proxy.h"
#include "call_manager_service_stub.h"
#include "call_control_manager.h"
#include "audio_control_manager.h"

namespace OHOS {
namespace Telephony {
class CallManagerService : public SystemAbility,
                           public CallManagerServiceStub,
                           public std::enable_shared_from_this<CallManagerService> {
    DECLARE_DELAYED_SINGLETON(CallManagerService)
    DECLARE_SYSTEM_ABILITY(CallManagerService)
public:
    bool Init();
    void UnInit();

    void OnDump() override;
    void OnStart() override;
    void OnStop() override;
    /**
     * register callback
     *
     * @param callback[in], callback function pointer
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t RegisterCallBack(const sptr<ICallAbilityCallback> &callback) override;

    /**
     * Call diale interface
     *
     * @param number[in], dial param.
     * @param extras[in], extras date.
     * @return Returns callId when the value is greater than zero, others on failure.
     */
    int32_t DialCall(std::u16string number, AppExecFwk::PacMap &extras) override;

    /**
     * Answer call
     *
     * @param callId[in], call id
     * @param videoState[in], 0: audio, 1: video
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t AnswerCall(int32_t callId, int32_t videoState) override;

    /**
     * Reject call
     *
     * @param callId[in], call id
     * @param rejectWithMessage[in], Whether to enter the reason for rejection,true:yes false:no
     * @param textMessage[in], The reason you reject the call
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t RejectCall(int32_t callId, bool rejectWithMessage, std::u16string textMessage) override;

    /**
     * Disconnect call
     *
     * @param callId[in], call id
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t HangUpCall(int32_t callId) override;

    /**
     * app get call state
     *
     * @return Returns call state.
     */
    int32_t GetCallState() override;

    /**
     * Hold call
     *
     * @param callId[in], call id
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t HoldCall(int32_t callId) override;

    /**
     * UnHold call
     *
     * @param callId[in], call id
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t UnHoldCall(int32_t callId) override;

    /**
     * Switch call
     *
     * @param callId[in], call id
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t SwitchCall(int32_t callId) override;

    /**
     * Is there Call
     *
     * @return Returns ture on has call, others on there is no call.
     */
    bool HasCall() override;

    /**
     * Can I initiate a call
     *
     * @return Returns ture on can, others on there is not can.
     */
    bool IsNewCallAllowed() override;

    /**
     * Get ringing state
     *
     * @return Returns ringing state.
     */
    bool IsRinging() override;

    /**
     * Is there an emergency call
     *
     * @return Returns ture on Emergency call, false on No emergency call.
     */
    bool IsInEmergencyCall() override;

    /**
     * Start dtmf
     *
     * @param callId[in], call id
     * @param str[in], Characters sent
     * @return Returns 0 on success, others on failure.
     */
    int32_t StartDtmf(int32_t callId, char str) override;

    /**
     * Send dtmf
     *
     * @param callId[in], call id
     * @param str[in], Characters sent
     * @return Returns 0 on success, others on failure.
     */
    int32_t SendDtmf(int32_t callId, char str) override;

    /**
     * Stop dtmf
     *
     * @param callId[in], call id
     * @return Returns 0 on success, others on failure.
     */
    int32_t StopDtmf(int32_t callId) override;

    /**
     * Send dtmf string
     *
     * @param callId[in], call id
     * @param str[in], String sent
     * @param on  DTMF pulse width, the unit is milliseconds, default is 0.
     * @param off DTMF pulse interval, the unit is milliseconds, default is 0.
     * @return Returns 0 on success, others on failure.
     */
    int32_t SendBurstDtmf(int32_t callId, std::u16string str, int32_t on, int32_t off) override;

    /**
     * Get Call Waiting
     * @param slotId
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t GetCallWaiting(int32_t slotId) override;

    /**
     * Set Call Waiting
     * @param slotId
     * @param activate
     * @return Returns kTelephonyNoErr on success, others on failure.
     */
    int32_t SetCallWaiting(int32_t slotId, bool activate) override;

    /**
     * Combine Conference
     *
     * @return Returns TELEPHONY_SUCCESS on success, others on failure.
     */
    int32_t CombineConference(int32_t mainCallId) override;

    bool IsEmergencyPhoneNumber(std::u16string &number, int32_t slotId, int32_t &errorCode) override;
    int32_t FormatPhoneNumber(
        std::u16string &number, std::u16string &countryCode, std::u16string &formatNumber) override;
    int32_t FormatPhoneNumberToE164(
        std::u16string &number, std::u16string &countryCode, std::u16string &formatNumber) override;
    int32_t GetMainCallId(int32_t callId) override;
    std::vector<std::u16string> GetSubCallIdList(int32_t callId) override;
    std::vector<std::u16string> GetCallIdListForConference(int32_t callId) override;

private:
    enum ServiceRunningState {
        STATE_STOPPED = 0,
        STATE_RUNNING,
    };

    ServiceRunningState state_ {ServiceRunningState::STATE_STOPPED};

    std::shared_ptr<CallControlManager> callControlManagerPtr_;
    std::shared_ptr<AudioControlManager> audioControlManagerPtr_;
    std::shared_ptr<CallSettingManager> callSettingManagerPtr_;
    std::shared_ptr<CallNumberUtils> callNumberUtilsPtr_;
    std::mutex lock_;
    const int32_t startTime_ = 1900;
    const int32_t extraMonth_ = 1;
};
} // namespace Telephony
} // namespace OHOS

#endif // CALL_MANAGER_SERVICE_H
