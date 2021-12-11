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

#ifndef NAPI_STATE_REGISTRY_H
#define NAPI_STATE_REGISTRY_H

#include <cstring>
#include <initializer_list>
#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "telephony_observer_broker.h"

namespace OHOS {
namespace Telephony {
constexpr int32_t NONE_EVENT_TYPE = 0;
constexpr int32_t LISTEN_NET_WORK_STATE = TelephonyObserverBroker::OBSERVER_MASK_NETWORK_STATE;
constexpr int32_t LISTEN_CALL_STATE = TelephonyObserverBroker::OBSERVER_MASK_CALL_STATE;
constexpr int32_t LISTEN_CELL_INFO = TelephonyObserverBroker::OBSERVER_MASK_CELL_INFO;
constexpr int32_t LISTEN_SIGNAL_STRENGTHS = TelephonyObserverBroker::OBSERVER_MASK_SIGNAL_STRENGTHS;
constexpr int32_t LISTEN_SIM_STATE = TelephonyObserverBroker::OBSERVER_MASK_SIM_STATE;
constexpr int32_t LISTEN_DATA_CONNECTION_STATE = TelephonyObserverBroker::OBSERVER_MASK_DATA_CONNECTION_STATE;

const std::string NET_WORK_STATE_CHANGE = "networkStateChange";
const std::string CALL_STATE_CHANGE = "callStateChange";
const std::string SIGNAL_STRENGTHS_CHANGE = "signalInfoChange";
const std::string SIM_STATE_CHANGE = "simStateChange";
const std::string DATA_CONNECTION_STATE = "dataConnectionStateChange";

constexpr int GSM = 1;
constexpr int CDMA = 2;
constexpr int LTE = 3;
constexpr int TDSCDMA = 4;

enum class DataConnectState : int32_t {
    /**
     * Indicates that a cellular data link is unknown.
     */
    DATA_STATE_UNKNOWN = -1,

    /**
     * Indicates that a cellular data link is disconnected.
     */
    DATA_STATE_DISCONNECTED = 0,

    /**
     * Indicates that a cellular data link is being connected.
     */
    DATA_STATE_CONNECTING = 1,

    /**
     * Indicates that a cellular data link is connected.
     */
    DATA_STATE_CONNECTED = 2,

    /**
     * Indicates that a cellular data link is suspended.
     */
    DATA_STATE_SUSPENDED = 3
};

enum class RatType : int32_t {
    /**
     * Indicates unknown radio access technology (RAT).
     */
    RADIO_TECHNOLOGY_UNKNOWN = 0,

    /**
     * Indicates that RAT is global system for mobile communications (GSM),
     * including GSM, general packet radio system (GPRS), and enhanced data rates
     * for GSM evolution (EDGE).
     */
    RADIO_TECHNOLOGY_GSM = 1,

    /**
     * Indicates that RAT is code division multiple access (CDMA), including
     * Interim Standard 95 (IS95) and Single-Carrier Radio Transmission Technology
     * (1xRTT).
     */
    RADIO_TECHNOLOGY_1XRTT = 2,

    /**
     * Indicates that RAT is wideband code division multiple address (WCDMA).
     */
    RADIO_TECHNOLOGY_WCDMA = 3,

    /**
     * Indicates that RAT is high-speed packet access (HSPA), including HSPA,
     * high-speed downlink packet access (HSDPA), and high-speed uplink packet
     * access (HSUPA).
     */
    RADIO_TECHNOLOGY_HSPA = 4,

    /**
     * Indicates that RAT is evolved high-speed packet access (HSPA+), including
     * HSPA+ and dual-carrier HSPA+ (DC-HSPA+).
     */
    RADIO_TECHNOLOGY_HSPAP = 5,

    /**
     * Indicates that RAT is time division-synchronous code division multiple
     * access (TD-SCDMA).
     */
    RADIO_TECHNOLOGY_TD_SCDMA = 6,

    /**
     * Indicates that RAT is evolution data only (EVDO), including EVDO Rev.0,
     * EVDO Rev.A, and EVDO Rev.B.
     */
    RADIO_TECHNOLOGY_EVDO = 7,

    /**
     * Indicates that RAT is evolved high rate packet data (EHRPD).
     */
    RADIO_TECHNOLOGY_EHRPD = 8,

    /**
     * Indicates that RAT is long term evolution (LTE).
     */
    RADIO_TECHNOLOGY_LTE = 9,

    /**
     * Indicates that RAT is LTE carrier aggregation (LTE-CA).
     */
    RADIO_TECHNOLOGY_LTE_CA = 10,

    /**
     * Indicates that RAT is interworking WLAN (I-WLAN).
     */
    RADIO_TECHNOLOGY_IWLAN = 11,

    /**
     * Indicates that RAT is 5G new radio (NR).
     */
    RADIO_TECHNOLOGY_NR = 12
};

enum class CallState : int32_t {
    /**
     * Indicates an invalid state, which is used when the call state fails to be
     * obtained.
     */
    CALL_STATE_UNKNOWN = -1,

    /**
     * Indicates that there is no ongoing call.
     */
    CALL_STATE_IDLE = 0,

    /**
     * Indicates that an incoming call is ringing or waiting.
     */
    CALL_STATE_RINGING = 1,

    /**
     * Indicates that a least one call is in the dialing, active, or hold state,
     * and there is no new incoming call ringing or waiting.
     */
    CALL_STATE_OFFHOOK = 2
};

enum class NetworkType : int32_t {
    /**
     * Indicates unknown network type.
     */
    NETWORK_TYPE_UNKNOWN,

    /**
     * Indicates that the network type is GSM.
     */
    NETWORK_TYPE_GSM,

    /**
     * Indicates that the network type is CDMA.
     */
    NETWORK_TYPE_CDMA,

    /**
     * Indicates that the network type is WCDMA.
     */
    NETWORK_TYPE_WCDMA,

    /**
     * Indicates that the network type is TD-SCDMA.
     */
    NETWORK_TYPE_TDSCDMA,

    /**
     * Indicates that the network type is LTE.
     */
    NETWORK_TYPE_LTE,

    /**
     * Indicates that the network type is 5G NR.
     */
    NETWORK_TYPE_NR
};
} // namespace Telephony
} // namespace OHOS
#endif // NAPI_STATE_REGISTRY_H