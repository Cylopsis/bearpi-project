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

#ifndef NATIVE_COMMON_TYPE_H
#define NATIVE_COMMON_TYPE_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#include "call_manager_errors.h"

#include "call_manager_inner_type.h"

namespace OHOS {
namespace Telephony {
const int kBoolValueIsFalse = 0;
const int kIsTure = 1;
const int kNativeVersion = 1;
const int kNativeFlags = 0;
const int kOnlyOneValue = 1;
const int kTwoValueLimit = 2;
const int kValueMaximumLimit = 3;
const int kFourValueMaximumLimit = 4;
const int kFiveValueMaximumLimit = 5;
const int kArrayIndexFirst = 0;
const int kArrayIndexSecond = 1;
const int kArrayIndexThird = 2;
const int kArrayIndexFourth = 3;
const int kDataLengthOne = 1;
const int kDataLengthTwo = 2;
const int kDtmfDefaultOff = 10;
const int kPhoneNumberMaximumLimit = 31;
const int kMessageContentMaximumLimit = 160;
const int kDefaultSlotId = 0;

struct AsyncContext {
    napi_env env;
    napi_async_work work;
    napi_deferred deferred;
    napi_ref callbackRef;
    int32_t callId;
    int32_t result;
    char number[kMaxNumberLen];
    size_t numberLen;
    napi_value value[kValueMaximumLimit];
    size_t valueLen;
    int32_t errorCode = TELEPHONY_SUCCESS;
};

struct DialAsyncContext : AsyncContext {
    int32_t accountId;
    int32_t videoState;
    int32_t dialScene;
    int32_t dialType;
};

struct AnswerAsyncContext : AsyncContext {
    int32_t videoState;
};

struct RejectAsyncContext : AsyncContext {
    bool isSendSms;
    std::string messageContent;
};

struct ListAsyncContext : AsyncContext {
    std::vector<std::u16string> listResult;
};

struct DtmfAsyncContext : AsyncContext {
    int32_t on;
    int32_t off;
    std::string code;
};

struct SupplementAsyncContext : AsyncContext {
    int32_t slotId;
    int32_t type;
    int32_t mode;
    std::string content;
    bool flag;
};

struct UtilsAsyncContext : AsyncContext {
    int32_t slotId;
    std::u16string formatNumber;
    std::string code;
};

struct EventListener {
    napi_env env;
    napi_value thisVar = nullptr;
    napi_ref callbackRef = nullptr;
    napi_deferred deferred = nullptr;
    bool isExist;
};

enum CallWaitingStatus {
    CALL_WAITING_DISABLE = 0,
    CALL_WAITING_ENABLE,
};

struct ReceiveDataWorker {
    napi_env env;
    napi_ref ref = 0;
    CallAttributeInfo info;
    EventListener callback;
};
} // namespace Telephony
} // namespace OHOS

#endif // NAPI_CALL_MANAGER_TYPES_H