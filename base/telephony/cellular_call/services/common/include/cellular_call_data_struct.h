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

#ifndef CELLULAR_CALL_DATA_STRUCT_H
#define CELLULAR_CALL_DATA_STRUCT_H

#include <list>
#include <memory>

#include "cellular_call_types.h"

namespace OHOS {
namespace Telephony {
enum CLIRMode {
    DEFAULT = 0,
    TRANSFER = 1,
    INHIBITION = 2,
};

struct DialRequestStruct {
    std::string phoneNum;
    CLIRMode clirMode;
};

enum MMIHandlerId {
    EVENT_MMI_Id = 0,
    EVENT_INQUIRE_MMI = 1,
    EVENT_SET_MMI = 2,
    EVENT_INQUIRE_CLIR_MMI = 3,
    EVENT_INQUIRE_CALL_FORWARD_MMI = 4,
    EVENT_SET_CALL_FORWARD_MMI = 5,
    EVENT_DEAL_USSD_MMI = 6,
};

/**
 * 3GPP TS 22.030 V4.0.0 (2001-03)  6.5.2 Structure of the MMI
 * The following sequence of functions shall be used for the control of Supplementary Services:
 *  SELECT:	Entry of the procedure information (may be a digit or a sequence of characters).
 *  SEND:	Transmission of the information to the network.
 *  INDICATION:	Call progress indications.
 */
struct MMIData {
    std::string fullString;
    std::string actionString;
    std::string serviceCode;
    // 3GPP TS 22.030 V4.0.0 (2001-03)  6.5.2 Structure of the MMI
    // This structure consists of the following parts:
    // Service Code, SC( (2 or 3 digits);
    // Supplementary Information, SI (variable length).
    std::string serviceInfoA;
    std::string serviceInfoB;
    std::string serviceInfoC;
    std::string pwdString;
    std::string dialString;
};

/**
 * 3GPP TS 22.030 V4.0.0 (2001-03)
 * Tele- and Bearer Service Supplementary Information codes (SIb).
Alternate and speech/data services are included with the equivalent data service.
Basic Service
group number (note)	Telecommunication Service	MMI Service Code
1 to 12	        All tele and bearer services	no code required
                Teleservices
1 to 6, 12	    All teleservices	            10
1	            Telephony 	                    11
2 to 6	        All data teleservices	        12
6	            Facsimile services	            13
2	            Short Message Services	        16
1, 3 to 6, 12	All teleservices except SMS	    19
12	            Voice group services
                Voice Group Call Service (VGCS)	17
                Voice Broadcast Service (VBS)	18
                Bearer Service
7 to 11	        All bearer services	            20
7	            All async services	            21
8	            All sync services	            22
8	            All data circuit sync	        24
7	            All data circuit async	        25
13	            All GPRS bearer services	    99
*/
enum CFServiceCode {
    ALL_TELE_SERVICES = 10,
    TELE_SERVICES = 11,
    ALL_DATA_TELE_SERVICES = 12,
    FACSIMILE_SERVICES = 13,
    SHORT_MESSAGE_SERVICES = 16,
    ALL_TELE_SERVICES_EXCEPT_SMS = 19,
    ALL_BEARER_SERVICES = 20,
    ALL_ASYNC_SERVICES = 21,
    ALL_SYNC_SERVICES = 22,
    ALL_DATA_CIRCUIT_SYNC = 24,
    ALL_DATA_CIRCUIT_ASYNC = 25,
    ALL_GPRS_BEARER_SERVICES = 99,
};
/**
* 27007-430_2001 7.11	Call forwarding number and conditions +CCFC
* 3GPP TS 22.082 [4]
* <classx> is a sum of integers each representing a class of information (default 7):
* 1	voice (telephony)
* 2	data (refers to all bearer services; with <mode>=2 this may refer only to some bearer service if TA does
not support values 16, 32, 64 and 128)
* 4	fax (facsimile services)
* 8	short message service
* 16	data circuit sync
* 32	data circuit async
* 64	dedicated packet access
* 128	dedicated PAD access
*/
enum CFInformation {
    NONE = 0,
    VOICE = 1,
    FAX = 4,
    SHORT_MESSAGE_SERVICE = 8,
    DATA_CIRCUIT_SYNC = 16,
    DATA_CIRCUIT_ASYNC = 32,
    DEDICATED_PACKET_ACCESS = 64,
    DEDICATED_PAD_ACCESS = 128,
};
} // namespace Telephony
} // namespace OHOS

#endif // CELLULAR_CALL_DATA_STRUCT_H