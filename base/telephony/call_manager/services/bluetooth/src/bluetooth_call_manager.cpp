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

#include "bluetooth_call_manager.h"

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

#include "audio_control_manager.h"
#include "call_control_manager.h"

#ifdef ABILITY_BLUETOOTH_SUPPORT
#include "bluetooth_mgr_client.h"
#endif

namespace OHOS {
namespace Telephony {
BluetoothCallManager::BluetoothCallManager() {}

BluetoothCallManager::~BluetoothCallManager() {}

void BluetoothCallManager::NewCallCreated(sptr<CallBase> &callObjectPtr)
{
    SendCallState(
        callObjectPtr->GetTelCallState(), callObjectPtr->GetAccountNumber(), callObjectPtr->GetVideoStateType());
}

void BluetoothCallManager::CallDestroyed(sptr<CallBase> &callObjectPtr)
{
    SendCallState(
        callObjectPtr->GetTelCallState(), callObjectPtr->GetAccountNumber(), callObjectPtr->GetVideoStateType());
}

void BluetoothCallManager::CallStateUpdated(
    sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState)
{
    SendCallState(
        callObjectPtr->GetTelCallState(), callObjectPtr->GetAccountNumber(), callObjectPtr->GetVideoStateType());
}

bool BluetoothCallManager::ConnectBtSco()
{
#ifdef ABILITY_BLUETOOTH_SUPPORT
    return BluetoothMgr::BluetoothMgrClient::GetInstance()->ConnectBtSco();
#endif
    return true;
}

bool BluetoothCallManager::DisconnectBtSco()
{
#ifdef ABILITY_BLUETOOTH_SUPPORT
    return BluetoothMgr::BluetoothMgrClient::GetInstance()->DisconnectBtSco();
#endif
    return true;
}

bool BluetoothCallManager::AnswerBtCall()
{
    auto call = DelayedSingleton<AudioControlManager>::GetInstance()->GetRingingCall();
    if (call == nullptr) {
        return false;
    }
    return DelayedSingleton<CallControlManager>::GetInstance()->AnswerCall(
        call->GetCallID(), (int32_t)(call->GetVideoStateType())) == TELEPHONY_SUCCESS;
}

bool BluetoothCallManager::HungUpBtCall()
{
    std::set<sptr<CallBase>> callList = DelayedSingleton<AudioControlManager>::GetInstance()->GetCallList();
    bool result = true;
    for (auto call : callList) {
        if (call->GetTelCallState() == TelCallState::CALL_STATUS_ACTIVE) {
            result = DelayedSingleton<CallControlManager>::GetInstance()->HangUpCall(call->GetCallID()) ==
                TELEPHONY_SUCCESS;
            if (result == false) {
                break;
            }
        }
    }
    return result;
}

int32_t BluetoothCallManager::SendCallState(
    TelCallState callState, const std::string &number, VideoStateType videoState)
{
#ifdef ABILITY_BLUETOOTH_SUPPORT
    int32_t alertingCallNum = DelayedSingleton<AudioControlManager>::GetInstance()->GetAlertingCallNumber();
    int32_t activeCallNum = DelayedSingleton<AudioControlManager>::GetInstance()->GetActiveCallNumber();
    int32_t incomingCallNum = DelayedSingleton<AudioControlManager>::GetInstance()->GetIncomingCallNumber();
    int32_t holdingCallNum = DelayedSingleton<AudioControlManager>::GetInstance()->GetHoldingCallNumber();
    return BluetoothMgr::BluetoothMgrClient::GetInstance()->CallStateChanged(alertingCallNum, activeCallNum,
        incomingCallNum, holdingCallNum, (int32_t)callState, number, (int32_t)videoState);
#endif
    return TELEPHONY_SUCCESS;
}

void BluetoothCallManager::SendCallList()
{
#ifdef ABILITY_BLUETOOTH_SUPPORT
    std::set<sptr<CallBase>> callList = DelayedSingleton<AudioControlManager>::GetInstance()->GetCallList();
    for (auto call : callList) {
        BluetoothMgr::BluetoothMgrClient::GetInstance()->CallInfoResponce(call->GetCallID(),
            (int32_t)(call->GetTelCallState()), call->GetAccountNumber(), (int32_t)(call->GetVideoStateType()));
    }
#endif
}

void BluetoothCallManager::IncomingCallActivated(sptr<CallBase> &callObjectPtr) {}

void BluetoothCallManager::IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content)
{}
} // namespace Telephony
} // namespace OHOS