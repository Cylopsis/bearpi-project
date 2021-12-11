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

#include "call_base.h"

#include <securec.h>

#include "telephony_log_wrapper.h"

#include "call_manager_errors.h"
#include "common_type.h"
#include "cellular_call_ipc_interface_proxy.h"
#include "audio_control_manager.h"

namespace OHOS {
namespace Telephony {
CallBase::CallBase()
    : callId_(0), callType_(TYPE_ERR_CALL), videoState_(TYPE_VOICE), accountNumber_(""),
      callRunningState_(CallRunningState::CALL_RUNNING_STATE_CREATE), conferenceState_(TEL_CONFERENCE_IDLE),
      startTime_(0), direction_(CALL_DIRECTION_UNKNOW), policyFlag_(0), callState_(CALL_STATUS_IDLE),
      isSpeakerphoneOn_(false), callEndedType_(CallEndedType::UNKNOWN)
{
    (void)memset_s(&contactInfo_, sizeof(ContactInfo), 0, sizeof(ContactInfo));
}

CallBase::~CallBase() {}

void CallBase::InitOutCallBase(const CallReportInfo &info, AppExecFwk::PacMap &extras, int32_t callId)
{
    videoState_ = (VideoStateType)extras.GetIntValue("videoState");
    accountNumber_ = info.accountNum;
    callId_ = callId;
    direction_ = CALL_DIRECTION_OUT;
}

void CallBase::InitInCallBase(const CallReportInfo &info, int32_t callId)
{
    videoState_ = info.callMode;
    callId_ = callId;
    direction_ = CALL_DIRECTION_IN;
}

int32_t CallBase::DialCallBase()
{
    callRunningState_ = CallRunningState::CALL_RUNNING_STATE_CONNECTING;
    TELEPHONY_LOGD("start to set audio");
    // Set audio, set hands-free
    SetAudio();
    return TELEPHONY_SUCCESS;
}

int32_t CallBase::IncomingCallBase()
{
    return TELEPHONY_SUCCESS;
}

int32_t CallBase::AcceptCallBase()
{
    if (!IsCurrentRinging()) {
        TELEPHONY_LOGW("the device is currently not ringing");
        return TELEPHONY_FAIL;
    }
    DelayedSingleton<AudioControlManager>::GetInstance()->SetVolumeAudible();
    return TELEPHONY_SUCCESS;
}

int32_t CallBase::RejectCallBase()
{
    return TELEPHONY_SUCCESS;
}

int32_t CallBase::HoldCallBase()
{
    int32_t ret = CALL_MANAGER_HOLD_FAILED;
    return ret;
}

int32_t CallBase::UnHoldCallBase()
{
    int32_t ret = CALL_MANAGER_UNHOLD_FAILED;
    return ret;
}

int32_t CallBase::TurnOffVoice(bool silence)
{
    return TELEPHONY_SUCCESS;
}

void CallBase::GetCallAttributeBaseInfo(CallAttributeInfo &info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    (void)memset_s(info.accountNumber, kMaxNumberLen, 0, kMaxNumberLen);
    if (memcpy_s(info.accountNumber, kMaxNumberLen, accountNumber_.c_str(), accountNumber_.length()) == 0) {
        info.speakerphoneOn = isSpeakerphoneOn_;
        info.videoState = videoState_;
        info.startTime = startTime_;
        info.callType = callType_;
        info.callId = callId_;
        info.callState = callState_;
        info.conferenceState = conferenceState_;
    }
}

int32_t CallBase::GetCallID()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return callId_;
}

CallType CallBase::GetCallType()
{
    return callType_;
}

CallRunningState CallBase::GetCallRunningState()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return callRunningState_;
}

int32_t CallBase::SetTelCallState(TelCallState nextState)
{
    std::lock_guard<std::mutex> lock(mutex_);
    if (callState_ == nextState) {
        TELEPHONY_LOGI("Call state duplication %d", nextState);
        return CALL_MANAGER_NOT_NEW_STATE;
    }
    callState_ = nextState;
    switch (nextState) {
        case TelCallState::CALL_STATUS_DIALING:
            callRunningState_ = CallRunningState::CALL_RUNNING_STATE_DIALING;
            break;
        case TelCallState::CALL_STATUS_INCOMING:
            callRunningState_ = CallRunningState::CALL_RUNNING_STATE_RINGING;
            break;
        case TelCallState::CALL_STATUS_WAITING:
            callRunningState_ = CallRunningState::CALL_RUNNING_STATE_RINGING;
            break;
        case TelCallState::CALL_STATUS_ACTIVE:
            callRunningState_ = CallRunningState::CALL_RUNNING_STATE_ACTIVE;
            break;
        case TelCallState::CALL_STATUS_HOLDING:
            callRunningState_ = CallRunningState::CALL_RUNNING_STATE_HOLD;
            break;
        case TelCallState::CALL_STATUS_DISCONNECTED:
            callRunningState_ = CallRunningState::CALL_RUNNING_STATE_ENDED;
            break;
        case TelCallState::CALL_STATUS_DISCONNECTING:
            callRunningState_ = CallRunningState::CALL_RUNNING_STATE_ENDING;
            break;
        case TelCallState::CALL_STATUS_ALERTING:
            callRunningState_ = CallRunningState::CALL_RUNNING_STATE_DIALING;
            break;
        default:
            break;
    }
    return TELEPHONY_SUCCESS;
}

TelCallState CallBase::GetTelCallState()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return callState_;
}

int32_t CallBase::SetTelConferenceState(TelConferenceState state)
{
    std::lock_guard<std::mutex> lock(mutex_);
    conferenceState_ = state;
    return TELEPHONY_SUCCESS;
}

TelConferenceState CallBase::GetTelConferenceState()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return conferenceState_;
}

VideoStateType CallBase::GetVideoStateType()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return videoState_;
}

void CallBase::SetPolicyFlag(int64_t policyFlag)
{
    std::lock_guard<std::mutex> lock(mutex_);
    policyFlag_ = policyFlag;
}

int64_t CallBase::GetPolicyFlag()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return policyFlag_;
}

bool CallBase::GetCallerInfo(ContactInfo &info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    info = contactInfo_;
    return true;
}

void CallBase::SetCallerInfo(const ContactInfo &info)
{
    std::lock_guard<std::mutex> lock(mutex_);
    contactInfo_ = info;
}

CallEndedType CallBase::GetCallEndedType()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return callEndedType_;
}

int32_t CallBase::SetCallEndedType(CallEndedType callEndedType)
{
    std::lock_guard<std::mutex> lock(mutex_);
    callEndedType_ = callEndedType;
    return TELEPHONY_SUCCESS;
}

bool CallBase::CheckVoicemailNumber(std::string phoneNumber)
{
    return false;
}

void CallBase::SetAudio()
{
    // Get from the configuration file
    bool useSpeakerWhenDocked = true;
    /**
     * Determine if hands-free should be enabled during docking.
     * When the device is docked and the Bluetooth or wired headset is not in use,
     * please turn on the hands-free
     */
    bool useSpeakerForDock = IsSpeakerphoneEnabled();

    SetSpeakerphoneOn(useSpeakerWhenDocked || useSpeakerForDock);
    // Confirm whether the speaker is turned on
    if (isSpeakerphoneOn_) {
        DelayedSingleton<AudioControlManager>::GetInstance()->SetAudioDevice(DEVICE_SPEAKER);
    }
}

bool CallBase::IsSpeakerphoneEnabled()
{
    // Gets whether the device can be started from the configuration
    return true;
}

bool CallBase::IsCurrentRinging()
{
    return (callRunningState_ == CallRunningState::CALL_RUNNING_STATE_RINGING) ? true : false;
}

std::string CallBase::GetAccountNumber()
{
    return accountNumber_;
}

int32_t CallBase::SetSpeakerphoneOn(bool speakerphoneOn)
{
    isSpeakerphoneOn_ = speakerphoneOn;
    return TELEPHONY_SUCCESS;
}

bool CallBase::IsSpeakerphoneOn()
{
    return isSpeakerphoneOn_;
}

bool CallBase::IsAliveState()
{
    return !(callState_ == TelCallState::CALL_STATUS_IDLE || callState_ == TelCallState::CALL_STATUS_DISCONNECTED ||
        callState_ == TelCallState::CALL_STATUS_DISCONNECTING);
}
} // namespace Telephony
} // namespace OHOS
