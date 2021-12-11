/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
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

#ifndef BASE_COMPILERUNTIME_JS_UTIL_MODULE_RATIONALNUMBER_CLASS_H
#define BASE_COMPILERUNTIME_JS_UTIL_MODULE_RATIONALNUMBER_CLASS_H

#include <cstring>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
namespace OHOS::Util {
    class RationalNumber {
    public:
        explicit RationalNumber(napi_env env, int numerator, int denominator);
        virtual ~RationalNumber(){}
        napi_value CreateRationalFromString(napi_value str, napi_value RationalNumberClass) const;
        napi_value CompareTo(napi_value rational) const;
        napi_value Equals(napi_value obj) const;
        napi_value Value() const;
        napi_value GetCommonDivisor(napi_value num1, napi_value num2) const;
        napi_value GetDenominator() const;
        napi_value GetNumerator() const;
        napi_value IsFinite() const;
        napi_value IsNaN() const;
        napi_value IsZero() const;
        napi_value ToString() const;
    private:
        int mnum = 0;
        int mden = 0;
        napi_env env_;
        napi_value CreateObj(int num1, int num2, napi_value RationalNumberClass) const;
    };
}
#endif