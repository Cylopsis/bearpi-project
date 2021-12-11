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

#ifndef BLUETOOTH_CALL_MANAGER_H
#define BLUETOOTH_CALL_MANAGER_H

#include "call_state_listener_base.h"

#include "singleton.h"

namespace OHOS {
namespace Telephony {
class BluetoothCallManager : public CallStateListenerBase,
                             public std::enable_shared_from_this<BluetoothCallManager> {
    DECLARE_DELAYED_SINGLETON(BluetoothCallManager)
public:
    void Init();
    bool ConnectBtSco();
    bool DisconnectBtSco();
    bool AnswerBtCall();
    bool HungUpBtCall();
    void NewCallCreated(sptr<CallBase> &callObjectPtr) override;
    void CallDestroyed(sptr<CallBase> &callObjectPtr) override;
    void IncomingCallActivated(sptr<CallBase> &callObjectPtr) override;
    void IncomingCallHungUp(sptr<CallBase> &callObjectPtr, bool isSendSms, std::string content) override;
    void CallStateUpdated(sptr<CallBase> &callObjectPtr, TelCallState priorState, TelCallState nextState) override;
    int32_t SendCallState(TelCallState callState, const std::string &number, VideoStateType videoState);
    void SendCallList();
};
} // namespace Telephony
} // namespace OHOS
#endif