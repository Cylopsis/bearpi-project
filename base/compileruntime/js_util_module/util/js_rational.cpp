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

#include "js_rational.h"
#include <climits>
#include <string>
#include <cctype>
#include "utils/log.h"
#include "securec.h"
namespace OHOS::Util {
    RationalNumber::RationalNumber(napi_env env, int molecule, int denominator)
    {
        env_ = env;
        napi_value result = nullptr;
        int num = molecule;
        int den = denominator;
        num = den < 0 ?  num * (-1) : num;
        den = den < 0 ?  den * (-1) : den;
        if (den == 0) {
            if (num > 0) {
                mnum = 1;
                mden = 0;
            } else if (num < 0) {
                mnum = -1;
                mden = 0;
            } else {
                mnum = 0;
                mden = 0;
            }
        } else if (num == 0) {
            mnum = 0;
            mden = 1;
        } else {
            napi_value num1 = nullptr;
            napi_value num2 = nullptr;
            napi_create_int32(env_, num, &num1);
            napi_create_int32(env_, den, &num2);
            result = GetCommonDivisor(num1, num2);
            int gnum = 0;
            napi_get_value_int32(env_, result, &gnum);
            if (gnum != 0) {
                mnum = num / gnum;
                mden = den / gnum;
            }
        }
    }

    napi_value RationalNumber::CreateRationalFromString(napi_value str, napi_value RationalNumberClass) const
    {
        size_t len = 0;
        int flag = 0;
        napi_get_value_string_utf8(env_, str, nullptr, 0, &len);
        char *buffer = nullptr;
        if (len > 0) {
            buffer = new char[len + 1];
            if (memset_s(buffer, len + 1, '\0', len + 1) != 0) {
                napi_throw_error(env_, "-1", "memset_s failed");
            }
        } else {
            napi_throw_error(env_, "NullPointerException", "string must not be null!");
        }
        napi_get_value_string_utf8(env_, str, buffer, len + 1, &len);
        std::string buf = "";
        if (buffer != nullptr) {
            buf = buffer;
            delete []buffer;
            buffer = nullptr;
        }
        if (buf.compare("NaN") == 0) {
            return CreateObj(0, 0, RationalNumberClass);
        }
        size_t colon = buf.find(':');
        size_t semicolon = buf.find('/');
        if ((colon == std::string::npos && semicolon == std::string::npos)
            || (colon != std::string::npos && semicolon != std::string::npos)) {
            napi_throw_error(env_, "invalidRational", "string invalid!");
        }
        size_t index = (colon != std::string::npos) ? colon : semicolon;
        std::string s1 = buf.substr(0, index);
        std::string s2 = buf.substr(index + 1, buf.size());
        for (int i = 1; i < s1.size(); i++) {
            if (((s1[0] == '+') || (s1[0] == '-') || (isdigit(s1[0]))) && (isdigit(s1[i]))) {
                flag = 1;
            } else {
                napi_throw_error(env_, "invalidRational", "string invalid!");
            }
        }
        int num1 = stoi(s1) * flag;
        for (int i = 1; i < s2.size(); i++) {
            if (((s2[0] == '+') || (s2[0] == '-') || (isdigit(s2[0]))) && (isdigit(s2[i]))) {
                flag = 1;
            } else {
                napi_throw_error(env_, "invalidRational", "string invalid!");
            }
        }
        int num2 = stoi(s2) * flag;
        return CreateObj(num1, num2, RationalNumberClass);
    }

    napi_value RationalNumber::CreateObj(int num1, int num2, napi_value RationalNumberClass) const
    {
        napi_value argvs[2] = { nullptr };
        NAPI_CALL(env_, napi_create_int32(env_, num1, &argvs[0]));
        NAPI_CALL(env_, napi_create_int32(env_, num2, &argvs[1]));
        size_t argc = 2;
        napi_value res = nullptr;
        NAPI_CALL(env_, napi_new_instance(env_, RationalNumberClass, argc, argvs, &res));
        return res;
    }

    napi_value RationalNumber::CompareTo(napi_value rational) const
    {
        RationalNumber *other = nullptr;
        NAPI_CALL(env_, napi_unwrap(env_, rational, reinterpret_cast<void**>(&other)));
        if (mnum == other->mnum && mden == other->mden) {
            napi_value result = nullptr;
            NAPI_CALL(env_, napi_create_int32(env_, 0, &result));
            return result;
        } else if (mnum == 0 && mden == 0) {
            napi_value result = nullptr;
            NAPI_CALL(env_, napi_create_int32(env_, 1, &result));
            return result;
        } else if ((other->mnum == 0) && (other->mden == 0)) {
            napi_value result = nullptr;
            NAPI_CALL(env_, napi_create_int32(env_, -1, &result));
            return result;
        } else if ((mden == 0 && mnum > 0) || (other->mden == 0 && other->mnum < 0)) {
            napi_value result = nullptr;
            NAPI_CALL(env_, napi_create_int32(env_, 1, &result));
            return result;
        } else if ((mden == 0 && mnum < 0) || (other->mden == 0 && other->mnum > 0)) {
            napi_value result = nullptr;
            NAPI_CALL(env_, napi_create_int32(env_, -1, &result));
            return result;
        }
        long thisnum = static_cast<long>(mnum) * other->mden;
        long othernum = static_cast<long>(other->mnum) * mden;
        if (thisnum < othernum) {
            napi_value result = nullptr;
            NAPI_CALL(env_, napi_create_int32(env_, -1, &result));
            return result;
        } else if (thisnum > othernum) {
            napi_value result = nullptr;
            NAPI_CALL(env_, napi_create_int64(env_, 1, &result));
            return result;
        } else {
            napi_value result = nullptr;
            NAPI_CALL(env_, napi_create_int32(env_, 0, &result));
            return result;
        }
    }

    napi_value RationalNumber::Equals(napi_value obj) const
    {
        RationalNumber *object = nullptr;
        napi_status status = napi_unwrap(env_, obj, reinterpret_cast<void**>(&object));
        bool flag = false;
        long thisnum = static_cast<long>(mnum) * object->mden;
        long objnum = static_cast<long>(object->mnum) * mden;
        if (status != napi_ok) {
            napi_value result = nullptr;
            NAPI_CALL(env_, napi_get_boolean(env_, flag, &result));
            return result;
        }
        if (mnum == object->mnum && mden == object->mden) {
            flag = true;
            napi_value result = nullptr;
            NAPI_CALL(env_, napi_get_boolean(env_, flag, &result));
            return result;
        } else if ((thisnum == objnum) && (mnum != 0 && mden != 0) && (object->mnum != 0 && object->mden != 0)) {
            flag = true;
            napi_value result = nullptr;
            NAPI_CALL(env_, napi_get_boolean(env_, flag, &result));
            return result;
        } else if ((mnum == 0 && mden != 0) && (object->mnum == 0 && object->mden != 0)) {
            flag = true;
            napi_value result = nullptr;
            NAPI_CALL(env_, napi_get_boolean(env_, flag, &result));
            return result;
        } else if ((mnum > 0 && mden == 0) && (object->mnum > 0 && object->mden == 0)) {
            flag = true;
            napi_value result = nullptr;
            NAPI_CALL(env_, napi_get_boolean(env_, flag, &result));
            return result;
        } else if ((mnum < 0 && mden == 0) && (object->mnum < 0 && object->mden == 0)) {
            flag = true;
            napi_value result = nullptr;
            NAPI_CALL(env_, napi_get_boolean(env_, flag, &result));
            return result;
        } else {
            napi_value result = nullptr;
            NAPI_CALL(env_, napi_get_boolean(env_, flag, &result));
            return result;
        }
    }

    napi_value RationalNumber::Value() const
    {
        if (mnum > 0 && mden == 0) {
            napi_value result = nullptr;
            NAPI_CALL(env_, napi_create_int32(env_, INT_MAX, &result));
            return result;
        } else if (mnum < 0 && mden == 0) {
            napi_value result = nullptr;
            NAPI_CALL(env_, napi_create_int32(env_, INT_MIN, &result));
            return result;
        } else if ((mnum == 0) && (mden == 0)) {
            napi_value result = nullptr;
            NAPI_CALL(env_, napi_create_int32(env_, 0, &result));
            return result;
        } else {
            if (mnum % mden == 0) {
                int val = mnum / mden;
                napi_value result = nullptr;
                NAPI_CALL(env_, napi_create_int32(env_, val, &result));
                return result;
            } else {
                double num = mnum;
                double den = mden;
                double res = num / den;
                napi_value result = nullptr;
                NAPI_CALL(env_, napi_create_double(env_, res, &result));
                return result;
            }
        }
    }

    napi_value RationalNumber::GetCommonDivisor(napi_value num1, napi_value num2) const
    {
        int temp = 0;
        int number1 = 0;
        int number2 = 0;
        napi_get_value_int32(env_, num1, &number1);
        napi_get_value_int32(env_, num2, &number2);
        if (number1 == 0 || number2 == 0) {
            napi_throw_error(env_, "invalidnumber", "Parameter cannot be zero!");
        }
        if (number1 < number2) {
            temp = number1;
            number1 = number2;
            number2 = temp;
        }
        while (number1 % number2 != 0) {
            temp = number1 % number2;
            number1 = number2;
            number2 = temp;
        }
        napi_value result = nullptr;
        NAPI_CALL(env_, napi_create_int32(env_, number2, &result));
        return result;
    }

    napi_value RationalNumber::GetDenominator() const
    {
        napi_value result = nullptr;
        NAPI_CALL(env_, napi_create_int32(env_, mden, &result));
        return result;
    }

    napi_value RationalNumber::GetNumerator() const
    {
        napi_value result = nullptr;
        NAPI_CALL(env_, napi_create_int32(env_, mnum, &result));
        return result;
    }

    napi_value RationalNumber::IsFinite() const
    {
        bool flag = false;
        if (mden != 0) {
            flag = true;
        }
        napi_value result = nullptr;
        NAPI_CALL(env_, napi_get_boolean(env_, flag, &result));
        return result;
    }

    napi_value RationalNumber::IsNaN() const
    {
        bool flag = false;
        if ((mnum == 0) && (mden == 0)) {
            flag = true;
        }
        napi_value result = nullptr;
        NAPI_CALL(env_, napi_get_boolean(env_, flag, &result));
        return result;
    }

    napi_value RationalNumber::IsZero() const
    {
        bool flag = false;
        if ((mnum == 0) && (mden != 0)) {
            flag = true;
        }
        napi_value result = nullptr;
        NAPI_CALL(env_, napi_get_boolean(env_, flag, &result));
        return result;
    }

    napi_value RationalNumber::ToString() const
    {
        std::string buf;
        if (mnum == 0 && mden == 0) {
            buf = "NaN";
        } else if (mnum > 0 && mden == 0) {
            buf = "Infinity";
        } else if (mnum < 0 && mden == 0) {
            buf = "-Infinity";
        } else {
            buf = std::to_string(mnum) + "/" + std::to_string(mden);
        }
        napi_value res = nullptr;
        napi_create_string_utf8(env_, buf.c_str(), buf.size(), &res);
        return res;
    }
}