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

#include "call_status_manager.h"

#include <securec.h>

#include "call_manager_errors.h"
#include "telephony_log_wrapper.h"

#include "cellular_call_info_handler.h"
#include "cs_call.h"
#include "ims_call.h"
#include "ott_call.h"
#include "audio_control_manager.h"
#include "call_control_manager.h"

namespace OHOS {
namespace Telephony {
CallStatusManager::CallStatusManager()
{
    (void)memset_s(&callReportInfo_, sizeof(CallReportInfo), 0, sizeof(CallReportInfo));
    (void)memset_s(&callsReportInfo_, sizeof(CallsReportInfo), 0, sizeof(CallsReportInfo));
}

CallStatusManager::~CallStatusManager()
{
    UnInit();
}

int32_t CallStatusManager::Init()
{
    callsReportInfo_.callVec.clear();
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusManager::UnInit()
{
    callsReportInfo_.callVec.clear();
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusManager::HandleCallReportInfo(const CallReportInfo &info)
{
    int32_t ret = TELEPHONY_FAIL;
    callReportInfo_ = info;
    switch (info.state) {
        case TelCallState::CALL_STATUS_ACTIVE:
            ret = ActiveHandle(info);
            break;
        case TelCallState::CALL_STATUS_HOLDING:
            ret = HoldingHandle(info);
            break;
        case TelCallState::CALL_STATUS_DIALING:
            ret = DialingHandle(info);
            break;
        case TelCallState::CALL_STATUS_ALERTING:
            ret = AlertHandle(info);
            break;
        case TelCallState::CALL_STATUS_INCOMING:
            ret = IncomingHandle(info);
            break;
        case TelCallState::CALL_STATUS_WAITING:
            ret = IncomingHandle(info);
            break;
        case TelCallState::CALL_STATUS_DISCONNECTED:
            ret = DisconnectedHandle(info);
            break;
        case TelCallState::CALL_STATUS_DISCONNECTING:
            ret = DisconnectingHandle(info);
            break;
        default:
            TELEPHONY_LOGE("Invalid call state!");
            break;
    }
    return ret;
}

int32_t CallStatusManager::HandleCallsReportInfo(const CallsReportInfo &info)
{
    bool flag = false;
    TELEPHONY_LOGD("call list size:%{public}zu,slotId:%d", info.callVec.size(), info.slotId);
    for (auto &it : info.callVec) {
        for (auto &it1 : callsReportInfo_.callVec) {
            if (strcmp(it.accountNum, it1.accountNum) == 0) {
                TELEPHONY_LOGD("state:%d", it.state);
                if (it.state != it1.state) {
                    TELEPHONY_LOGI("handle updated call state:%d", it.state);
                    HandleCallReportInfo(it);
                }
                flag = true;
                break;
            }
        }
        if (!flag || callsReportInfo_.callVec.empty()) {
            TELEPHONY_LOGI("handle new call state:%d", it.state);
            HandleCallReportInfo(it);
        }
        flag = false;
    }

    for (auto &it2 : callsReportInfo_.callVec) {
        for (auto &it3 : info.callVec) {
            if (strcmp(it2.accountNum, it3.accountNum) == 0) {
                TELEPHONY_LOGD("state:%d", it2.state);
                flag = true;
                break;
            }
        }
        if (!flag) {
            it2.state = TelCallState::CALL_STATUS_DISCONNECTED;
            HandleCallReportInfo(it2);
        }
        flag = false;
    }
    callsReportInfo_.callVec.clear();
    callsReportInfo_ = info;
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusManager::HandleEventResultReportInfo(const CellularCallEventInfo &info)
{
    if (info.eventType != EVENT_REQUEST_RESULT_TYPE) {
        return TELEPHONY_FAIL;
    }
    CallEventInfo eventInfo;
    eventInfo.eventId = CallAbilityEventId::EVENT_UNKNOW_ID;
    switch (info.eventId) {
        case RESULT_INITIATE_DTMF_SUCCESS:
            eventInfo.eventId = CallAbilityEventId::EVENT_START_DTMF_SUCCESS;
            break;
        case RESULT_INITIATE_DTMF_FAILED:
            break;
        case RESULT_CEASE_DTMF_SUCCESS:
            break;
        case RESULT_CEASE_DTMF_FAILED:
            break;
        case RESULT_TRANSMIT_DTMF_SUCCESS:
            break;
        case RESULT_TRANSMIT_DTMF_FAILED:
            break;
        case RESULT_TRANSMIT_DTMF_STRING_SUCCESS:
            break;
        case RESULT_TRANSMIT_DTMF_STRING_FAILED:
            break;
        default:
            TELEPHONY_LOGD("error, Unknown event");
            break;
    }
    if (eventInfo.eventId != CallAbilityEventId::EVENT_UNKNOW_ID) {
        DelayedSingleton<CallControlManager>::GetInstance()->NotifyCallEventUpdated(eventInfo);
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusManager::IncomingHandle(const CallReportInfo &info)
{
    TELEPHONY_LOGD("handle incoming state");
    int32_t ret = TELEPHONY_FAIL;
#ifdef ABILITY_SIM_SUPPORT
    // According to the SIM card information to determine whether triple-management
    IsTripleManaged(accountId);
#endif
    std::string numberStr(info.accountNum);
    if (numberStr.empty()) {
        TELEPHONY_LOGE("phone number is NULL!");
        return CALL_MANAGER_PHONE_NUMBER_NULL;
    }

    if (IsCallExist(numberStr)) {
        TELEPHONY_LOGE("the call already exists!");
        return CALL_MANAGER_CALL_EXIST;
    }
    int32_t callId = GetNewCallId();
    ret = CreateNewCall(info, callId);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("CreateNewCall failed!");
        return TELEPHONY_FAIL;
    }
#ifdef ABILITY_DATABASE_SUPPORT
    // allow list filtering
    // Get the contact data from the database
    GetCallerInfoDate(ContactInfo);
    SetCallerInfo(contactInfo);
#endif
    sptr<CallBase> call = GetOneCallObject(callId);
    if (call == nullptr) {
        TELEPHONY_LOGE("Call is NULL");
        return CALL_MANAGER_CALL_NULL;
    }
    ret = UpdateCallState(call, info.state);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("UpdateCallState failed!");
        return ret;
    }
    ret = FilterResultsDispose(call);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("FilterResultsDispose failed!");
    }
    return ret;
}

int32_t CallStatusManager::DialingHandle(const CallReportInfo &info)
{
    TELEPHONY_LOGD("handle dialing state");
    int32_t ret = TELEPHONY_FAIL;
    std::string number(info.accountNum);
    if (IsCallExist(number)) {
        TELEPHONY_LOGW("this call has created yet!");
        return TELEPHONY_SUCCESS;
    }
    ret = DelayedSingleton<CallControlManager>::GetInstance()->CreateNewCall(info);
    if (ret != TELEPHONY_SUCCESS) {
        return ret;
    }

    std::string tmpStr(info.accountNum);
    sptr<CallBase> call = GetOneCallObject(tmpStr);
    if (call == nullptr) {
        TELEPHONY_LOGE("Call is NULL");
        return CALL_MANAGER_CALL_NULL;
    }
    ret = call->DialingProcess();
    if (ret != TELEPHONY_SUCCESS) {
        return ret;
    }
    DelayedSingleton<CallControlManager>::GetInstance()->NotifyNewCallCreated(call);

    ret = UpdateCallState(call, TelCallState::CALL_STATUS_DIALING);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("UpdateCallState failed, errCode:%{public}d", ret);
    }
    return ret;
}

int32_t CallStatusManager::ActiveHandle(const CallReportInfo &info)
{
    TELEPHONY_LOGD("handle active state");
    int32_t ret = TELEPHONY_FAIL;
    std::string tmpStr(info.accountNum);
    sptr<CallBase> call = GetOneCallObject(tmpStr);
    if (call == nullptr) {
        TELEPHONY_LOGE("Call is NULL");
        return CALL_MANAGER_CALL_NULL;
    }
    ret = UpdateCallState(call, TelCallState::CALL_STATUS_ACTIVE);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("UpdateCallState failed, errCode:%{public}d", ret);
        return ret;
    }
    call->SubCallCombineToConference();
#ifdef AUDIO_SUPPORT
    ToSpeakerPhone(call);
    DelayedSingleton<AudioControlManager>::GetInstance()->SetVolumeAudible();
#endif
    TELEPHONY_LOGD("handle active state success");
    return ret;
}

int32_t CallStatusManager::HoldingHandle(const CallReportInfo &info)
{
    TELEPHONY_LOGD("handle holding state");
    int32_t ret = TELEPHONY_FAIL;
    std::string tmpStr(info.accountNum);
    sptr<CallBase> call = GetOneCallObject(tmpStr);
    if (call == nullptr) {
        TELEPHONY_LOGE("Call is NULL");
        return CALL_MANAGER_CALL_NULL;
    }
    ret = UpdateCallState(call, TelCallState::CALL_STATUS_HOLDING);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("UpdateCallState failed, errCode:%{public}d", ret);
    }
    call->SubCallSeparateFromConference();
    return ret;
}

int32_t CallStatusManager::WaitingHandle(const CallReportInfo &info)
{
    TELEPHONY_LOGD("handle waiting state");
    int32_t ret = TELEPHONY_FAIL;
    std::string tmpStr(info.accountNum);
    sptr<CallBase> call = GetOneCallObject(tmpStr);
    if (call == nullptr) {
        TELEPHONY_LOGE("Call is NULL");
        return CALL_MANAGER_CALL_NULL;
    }
    ret = UpdateCallState(call, TelCallState::CALL_STATUS_WAITING);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("UpdateCallState failed, errCode:%{public}d", ret);
    }
    return ret;
}

int32_t CallStatusManager::AlertHandle(const CallReportInfo &info)
{
    TELEPHONY_LOGD("handle alerting state");
    int32_t ret = TELEPHONY_FAIL;
    std::string tmpStr(info.accountNum);
    sptr<CallBase> call = GetOneCallObject(tmpStr);
    if (call == nullptr) {
        TELEPHONY_LOGE("Call is NULL");
        return CALL_MANAGER_CALL_NULL;
    }
    ret = UpdateCallState(call, TelCallState::CALL_STATUS_ALERTING);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("UpdateCallState failed, errCode:%{public}d", ret);
        return ret;
    }
#ifdef AUDIO_SUPPORT
    ToSpeakerPhone(call);
    TurnOffMute(call);
    DelayedSingleton<AudioControlManager>::GetInstance()->SetVolumeAudible();
#endif
    return ret;
}

int32_t CallStatusManager::DisconnectingHandle(const CallReportInfo &info)
{
    TELEPHONY_LOGD("handle disconnecting state");
    int32_t ret = TELEPHONY_FAIL;
    std::string tmpStr(info.accountNum);
    sptr<CallBase> call = GetOneCallObject(tmpStr);
    if (call == nullptr) {
        TELEPHONY_LOGE("Call is NULL");
        return CALL_MANAGER_CALL_NULL;
    }
    ret = UpdateCallState(call, TelCallState::CALL_STATUS_DISCONNECTING);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("UpdateCallState failed, errCode:%{public}d", ret);
    }
    return ret;
}

int32_t CallStatusManager::DisconnectedHandle(const CallReportInfo &info)
{
    TELEPHONY_LOGD("handle disconnected state");
    int32_t ret = TELEPHONY_FAIL;
    std::string tmpStr(info.accountNum);
    sptr<CallBase> call = GetOneCallObject(tmpStr);
    if (call == nullptr) {
        TELEPHONY_LOGE("Call is NULL");
        return CALL_MANAGER_CALL_NULL;
    }
    ret = UpdateCallState(call, TelCallState::CALL_STATUS_DISCONNECTED);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("UpdateCallState failed, errCode:%{public}d", ret);
        return ret;
    }
    DeleteOneCallObject(call->GetCallID());
    return ret;
}

int32_t CallStatusManager::ConferenceHandle()
{
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusManager::UpdateCallState(sptr<CallBase> &call, TelCallState nextState)
{
    TELEPHONY_LOGD("UpdateCallState Enter");
    int32_t ret = TELEPHONY_FAIL;
    if (call == nullptr) {
        TELEPHONY_LOGE("Call is NULL");
        return CALL_MANAGER_CALL_NULL;
    }
    TelCallState priorState = call->GetTelCallState();
    TELEPHONY_LOGD("priorState:%d, nextState:%d", priorState, nextState);
    // need DTMF judge
    ret = call->SetTelCallState(nextState);
    if (ret != TELEPHONY_SUCCESS) {
        TELEPHONY_LOGE("SetTelCallState");
        return CALL_MANAGER_CALL_NULL;
    }
    TELEPHONY_LOGD("SetTelCallState success!");
    // State radio
    if (!DelayedSingleton<CallControlManager>::GetInstance()->NotifyCallStateUpdated(call, priorState, nextState)) {
        TELEPHONY_LOGE(
            "NotifyCallStateUpdated failed! priorState:%{public}d,nextState:%{public}d", priorState, nextState);
        return TELEPHONY_FAIL;
    }
    TELEPHONY_LOGD("UpdateCallState Leave");
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusManager::ToSpeakerPhone(sptr<CallBase> &call)
{
    int32_t ret = TELEPHONY_FAIL;
    if (call == nullptr) {
        TELEPHONY_LOGE("Call is NULL");
        return CALL_MANAGER_CALL_NULL;
    }
    if (call->GetCallRunningState() == CallRunningState::CALL_RUNNING_STATE_DIALING) {
        TELEPHONY_LOGI("Call is CALL_STATUS_DIALING");
        return ret;
    }
    if (call->IsSpeakerphoneOn()) {
        DelayedSingleton<AudioControlManager>::GetInstance()->SetAudioDevice(DEVICE_SPEAKER);
        ret = call->SetSpeakerphoneOn(false);
    }
    return ret;
}

int32_t CallStatusManager::TurnOffMute(sptr<CallBase> &call)
{
    if (call->GetEmergencyState() || HasEmergencyCall()) {
        DelayedSingleton<AudioControlManager>::GetInstance()->SetMute(false);
    } else {
        DelayedSingleton<AudioControlManager>::GetInstance()->SetMute(true);
    }
    return TELEPHONY_SUCCESS;
}

int32_t CallStatusManager::CreateNewCall(const CallReportInfo &info, int32_t callId)
{
    sptr<CallBase> callPtr = nullptr;
    switch (info.callType) {
        case CallType::TYPE_CS: {
            std::unique_ptr<CSCall> csCall = std::make_unique<CSCall>();
            if (csCall != nullptr) {
                csCall->InCallInit(info, callId);
                callPtr = csCall.release();
                AddOneCallObject(callPtr);
                TELEPHONY_LOGI("CreateNewCall success");
                return TELEPHONY_SUCCESS;
            }
            break;
        }
        case CallType::TYPE_IMS: {
            std::unique_ptr<IMSCall> imsCall = std::make_unique<IMSCall>();
            if (imsCall != nullptr) {
                imsCall->InCallInit(info, callId);
                callPtr = imsCall.release();
                AddOneCallObject(callPtr);
                TELEPHONY_LOGI("CreateNewCall success");
                return TELEPHONY_SUCCESS;
            }
            break;
        }
        case CallType::TYPE_OTT: {
            std::unique_ptr<OTTCall> ottCall = std::make_unique<OTTCall>();
            if (ottCall != nullptr) {
                ottCall->InCallInit(info, callId);
                callPtr = ottCall.release();
                AddOneCallObject(callPtr);
                TELEPHONY_LOGI("CreateNewCall success");
                return TELEPHONY_SUCCESS;
            }
            break;
        }
        default:
            TELEPHONY_LOGE("Invalid call type!");
            break;
    }
    return CALL_MANAGER_CREATE_CALL_OBJECT_FAIL;
}
} // namespace Telephony
} // namespace OHOS