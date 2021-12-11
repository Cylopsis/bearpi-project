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

#include <cstring>
#include <vector>

#include "js_textdecoder.h"
#include "js_textencoder.h"
#include "js_rational.h"
#include "js_base64.h"

#include "napi/native_api.h"
#include "napi/native_node_api.h"
#include "utils/log.h"

extern const char _binary_util_js_js_start[];
extern const char _binary_util_js_js_end[];
namespace OHOS::Util {
    static std::string temp = "cdfijoOs";
    napi_value RationalNumberClass = nullptr;
    static std::string DealWithPrintf(const std::string &format, const std::vector<std::string> &value)
    {
        size_t i = 0;
        size_t j = 0;
        std::string str;
        size_t formatSize = format.size();
        size_t valueSize = value.size();
        while (i < formatSize && j < valueSize) {
            if (format[i] == '%' && (i + 1 < formatSize && format[i + 1] == '%')) {
                str += '%';
                i += 2; // 2:The array goes back two digits.
            } else if (format[i] == '%' && (i + 1 < formatSize && (temp.find(format[i + 1])) != std::string::npos)) {
                if (format[i + 1] == 'c') {
                    j++;
                } else {
                    str += value[j++];
                }
                i += 2; // 2:The array goes back two digits.
            } else if (format[i] == '%' && (i + 1 < formatSize && (temp.find(format[i + 1])) == std::string::npos)) {
                str += '%';
                i++;
            }
            if (i < formatSize && format[i] != '%') {
                size_t pos = 0;
                if ((pos = format.find('%', i)) == std::string::npos) {
                    str += format.substr(i);
                    break;
                } else {
                    str += format.substr(i, pos - i);
                    i = pos;
                }
            }
        }
        while (j < valueSize) {
            str += " " + value[j++];
        }
        if (i < formatSize) {
            str += format.substr(i);
        }
        return str;
    }

    static napi_value FormatString(napi_env env, std::string &str)
    {
        std::string res;
        size_t strSize = str.size();
        for (size_t i = 0; i < strSize; ++i) {
            if (str[i] == '%' && (i + 1 < strSize && temp.find(str[i + 1]) != std::string::npos)) {
                if (str[i + 1] == 'o') {
                    res += "o ";
                } else if (str[i + 1] == 'O') {
                    res += "O ";
                } else if (str[i + 1] == 'i') {
                    res += "i ";
                } else if (str[i + 1] == 'j') {
                    res += "j ";
                } else if (str[i + 1] == 'd') {
                    res += "d ";
                } else if (str[i + 1] == 's') {
                    res += "s ";
                } else if (str[i + 1] == 'f') {
                    res += "f ";
                } else if (str[i + 1] == 'c') {
                    res += "c ";
                }
                i++;
            } else if (str[i] == '%' && (i + 1 < strSize && str[i + 1] == '%')) {
                i++;
            }
        }
        res = res.substr(0, res.size() - 1);
        napi_value result = nullptr;
        napi_create_string_utf8(env, res.c_str(), res.size(), &result);
        return result;
    }

    static napi_value DealWithFormatString(napi_env env, napi_callback_info info)
    {
        size_t argc = 0;
        napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
        napi_value *argv = nullptr;
        if (argc > 0) {
            argv = new napi_value[argc];
            napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
            char *format = nullptr;
            size_t formatsize = 0;
            napi_get_value_string_utf8(env, argv[0], nullptr, 0, &formatsize);
            if (formatsize > 0) {
                format = new char[formatsize + 1];
                napi_get_value_string_utf8(env, argv[0], format, formatsize + 1, &formatsize);
                std::string str = format;
                delete []format;
                delete []argv;
                argv = nullptr;
                format = nullptr;
                return FormatString(env, str);
            }
        }
        napi_value res = nullptr;
        NAPI_CALL(env, napi_get_undefined(env, &res));
        return res;
    }

    static std::string PrintfString(const std::string &format, const std::vector<std::string> &value)
    {
        std::string printInfo = DealWithPrintf(format, value);
        return printInfo;
    }

    static napi_value Printf(napi_env env, napi_callback_info info)
    {
        napi_value result = nullptr;
        size_t argc = 0;
        napi_get_cb_info(env, info, &argc, nullptr, nullptr, nullptr);
        napi_value *argv = nullptr;
        if (argc > 0) {
            argv = new napi_value[argc];
            napi_get_cb_info(env, info, &argc, argv, nullptr, nullptr);
            char *format = nullptr;
            size_t formatsize = 0;
            napi_get_value_string_utf8(env, argv[0], nullptr, 0, &formatsize);
            if (formatsize > 0) {
                format = new char[formatsize + 1];
            }
            napi_get_value_string_utf8(env, argv[0], format, formatsize + 1, &formatsize);
            std::string printInfo;
            std::vector<std::string> value;
            for (size_t i = 1; i < argc; i++) {
                char *valueString = nullptr;
                size_t valuesize = 0;
                napi_get_value_string_utf8(env, argv[i], nullptr, 0, &valuesize);
                if (valuesize > 0) {
                    valueString = new char[valuesize + 1];
                }
                napi_get_value_string_utf8(env, argv[i], valueString, valuesize + 1, &valuesize);
                value.push_back(valueString);
                delete []valueString;
                valueString = nullptr;
            }
            printInfo = PrintfString(format, value);
            napi_create_string_utf8(env, printInfo.c_str(), printInfo.size(), &result);
            delete []format;
            delete []argv;
            argv = nullptr;
            format = nullptr;
            return result;
        }
        napi_value res = nullptr;
        NAPI_CALL(env, napi_get_undefined(env, &res));
        return res;
    }

    static napi_value GetErrorString(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        napi_value result = nullptr;
        std::string errInfo;
        size_t argc = 1;
        napi_value argv = nullptr;
        NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &argv, &thisVar, nullptr));
        uint32_t err = 0;
        NAPI_CALL(env, napi_get_value_uint32(env, argv, &err));
        errInfo = strerror(err);
        NAPI_CALL(env, napi_create_string_utf8(env, errInfo.c_str(), errInfo.size(), &result));
        return result;
    }

    static void SetVec(const napi_status fatSta, const napi_status bomSta, const bool fat, const bool bom,
        std::vector<int> &paraVec)
    {
        if (paraVec.size() != 2) {
            return;
        }
        if (fatSta == napi_ok) {
            if (fat) {
                paraVec[0] = 1;
            } else {
                paraVec[0] = 0;
            }
        }
        if (bomSta == napi_ok) {
            if (bom) {
                paraVec[1] = 1;
            } else {
                paraVec[1] = 0;
            }
        }
    }

    static napi_value GetSecPara(napi_env env, napi_value valData, std::vector<int> &paraVec)
    {
        napi_value messageKeyFatal = nullptr;
        const char *messageKeyStrFatal = "fatal";
        napi_value messageKeyIgnorebom = nullptr;
        const char *messageKeyStrIgnorebom = "ignoreBOM";
        napi_value resultFatal = nullptr;
        napi_value resultIgnorebom = nullptr;
        bool bResultFat = false;
        bool bResultIgnbom = false;
        NAPI_CALL(env, napi_create_string_utf8(env, messageKeyStrFatal, strlen(messageKeyStrFatal),
            &messageKeyFatal));
        NAPI_CALL(env, napi_create_string_utf8(env, messageKeyStrIgnorebom, strlen(messageKeyStrIgnorebom),
            &messageKeyIgnorebom));
        NAPI_CALL(env, napi_get_property(env, valData, messageKeyFatal, &resultFatal));
        NAPI_CALL(env, napi_get_property(env, valData, messageKeyIgnorebom, &resultIgnorebom));
        napi_status naFat = napi_get_value_bool(env, resultFatal, &bResultFat);
        napi_status naBom = napi_get_value_bool(env, resultIgnorebom, &bResultIgnbom);
        SetVec(naFat, naBom, bResultFat, bResultIgnbom, paraVec);
        return nullptr;
    }

    static napi_value TextdecoderConstructor(napi_env env, napi_callback_info info)
    {
        size_t tempArgc = 0;
        napi_value thisVar = nullptr;
        napi_get_cb_info(env, info, &tempArgc, nullptr, &thisVar, nullptr);
        size_t argc = 0;
        void *data = nullptr;
        char *type = nullptr;
        size_t typeLen = 0;
        std::vector<int> paraVec(2, 0); // 2: Specifies the size of the container to be applied for.
        if (tempArgc == 1) {
            argc = 1;
            napi_value argv = nullptr;
            NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &argv, nullptr, &data));
            // first para
            NAPI_CALL(env, napi_get_value_string_utf8(env, argv, nullptr, 0, &typeLen));
            if (typeLen > 0) {
                type = new char[typeLen + 1]();
            }
            NAPI_CALL(env, napi_get_value_string_utf8(env, argv, type, typeLen + 1, &typeLen));
        } else if (tempArgc == 2) { // 2: The number of parameters is 2.
            argc = 2; // 2: The number of parameters is 2.
            napi_value argv[2] = { 0 };
            NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, &data));
            // first para
            NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], nullptr, 0, &typeLen));
            if (typeLen > 0) {
                type = new char[typeLen + 1]();
            }
            NAPI_CALL(env, napi_get_value_string_utf8(env, argv[0], type, typeLen + 1, &typeLen));
            // second para
            GetSecPara(env, argv[1], paraVec);
        }
        std::string enconding = "utf-8";
        if (type != nullptr) {
            enconding = type;
        }
        delete []type;
        type = nullptr;
        auto objectInfo = new TextDecoder(env, enconding, paraVec);
        NAPI_CALL(env, napi_wrap(
            env, thisVar, objectInfo,
            [](napi_env env, void *data, void *hint) {
                auto objInfo = (TextDecoder*)data;
                if (objInfo != nullptr) {
                    delete objInfo;
                }
            },
            nullptr, nullptr));
        return thisVar;
    }

    static napi_value TextdecoderDecode(napi_env env, napi_callback_info info)
    {
        size_t tempArgc = 2;
        napi_value thisVar = nullptr;
        napi_get_cb_info(env, info, &tempArgc, nullptr, &thisVar, nullptr);
        size_t argc = 0;
        void *dataPara = nullptr;
        napi_typedarray_type type;
        size_t length = 0;
        void *data = nullptr;
        napi_value arraybuffer = nullptr;
        size_t byteOffset = 0;
        bool iStream = false;
        TextDecoder *textDecoder = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&textDecoder));
        napi_value valStr = nullptr;
        if (tempArgc == 1) {
            argc = 1;
            napi_value argv = nullptr;
            NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &argv, nullptr, &dataPara));
            // first para
            NAPI_CALL(env, napi_get_typedarray_info(env, argv, &type, &length, &data, &arraybuffer, &byteOffset));
            valStr = textDecoder->Decode(argv, iStream);
        } else if (tempArgc == 2) { // 2: The number of parameters is 2.
            argc = 2; // 2: The number of parameters is 2.
            napi_value argv[2] = { 0 };
            NAPI_CALL(env, napi_get_cb_info(env, info, &argc, argv, nullptr, &dataPara));
            // first para
            NAPI_CALL(env, napi_get_typedarray_info(env, argv[0], &type, &length, &data, &arraybuffer, &byteOffset));
            // second para
            napi_value messageKeyStream = nullptr;
            const char *messageKeyStrStream = "stream";

            napi_value resultStream = nullptr;
            NAPI_CALL(env, napi_create_string_utf8(env, messageKeyStrStream, strlen(messageKeyStrStream),
                &messageKeyStream));
            NAPI_CALL(env, napi_get_property(env, argv[1], messageKeyStream, &resultStream));
            NAPI_CALL(env, napi_get_value_bool(env, resultStream, &iStream));
            valStr = textDecoder->Decode(argv[0], iStream);
        }
        return valStr;
    }

    static napi_value TextdecoderGetEncoding(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
        TextDecoder *textDecoder = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&textDecoder));
        napi_value retVal = textDecoder->GetEncoding();
        return retVal;
    }

    static napi_value TextdecoderGetFatal(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
        TextDecoder *textDecoder = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&textDecoder));
        napi_value retVal = textDecoder->GetFatal();
        return retVal;
    }

    static napi_value TextdecoderGetIgnoreBOM(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
        TextDecoder *textDecoder = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&textDecoder));
        napi_value retVal = textDecoder->GetIgnoreBOM();
        return retVal;
    }

    // Encoder
    static napi_value TextEncoderConstructor(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        void *data = nullptr;
        NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, &data));

        auto object = new TextEncoder(env);
        NAPI_CALL(env, napi_wrap(
            env, thisVar, object,
            [](napi_env env, void *data, void *hint) {
                auto obj = (TextEncoder*)data;
                if (obj != nullptr) {
                    delete obj;
                }
            },
            nullptr, nullptr));
        return thisVar;
    }

    static napi_value GetEncoding(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));

        TextEncoder *object = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&object));

        return object->GetEncoding();
    }

    static napi_value Encode(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        size_t requireArgc = 1;
        size_t argc = 1;
        napi_value args = nullptr;
        NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &args, &thisVar, nullptr));

        NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

        napi_valuetype valuetype;
        NAPI_CALL(env, napi_typeof(env, args, &valuetype));

        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected.");

        TextEncoder *object = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&object));

        napi_value result = object->Encode(args);

        return result;
    }

    static napi_value EncodeInto(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        size_t requireArgc = 2;
        size_t argc = 2;
        napi_value args[2] = { nullptr };
        NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &thisVar, nullptr));

        NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");

        napi_valuetype valuetype0;
        NAPI_CALL(env, napi_typeof(env, args[0], &valuetype0));

        napi_typedarray_type valuetype1;
        size_t length = 0;
        void *data = nullptr;
        napi_value arraybuffer = nullptr;
        size_t byteOffset = 0;
        NAPI_CALL(env, napi_get_typedarray_info(env, args[1], &valuetype1, &length, &data, &arraybuffer, &byteOffset));

        NAPI_ASSERT(env, valuetype0 == napi_string, "Wrong argument type. String expected.");
        NAPI_ASSERT(env, valuetype1 == napi_uint8_array, "Wrong argument type. napi_uint8_array expected.");

        TextEncoder *object = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&object));

        napi_value result = object->EncodeInto(args[0], args[1]);

        return result;
    }

    static napi_value RationalNumberConstructor(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        void *data = nullptr;
        size_t argc = 3;
        napi_value args[2] = { nullptr };
        NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &thisVar, &data));
        int num = 0;
        int den = 0;
        napi_get_value_int32(env, args[0], &num);
        napi_get_value_int32(env, args[1], &den);
        auto objectInfo = new RationalNumber(env, num, den);
        NAPI_CALL(env, napi_wrap(
            env, thisVar, objectInfo,
            [](napi_env env, void *data, void *hint) {
                auto objInfo = (RationalNumber*)data;
                if (objInfo != nullptr) {
                    delete objInfo;
                }
            },
            nullptr, nullptr));
        return thisVar;
    }

    static napi_value CreateRationalFromString(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        size_t requireArgc = 1;
        size_t argc = 1;
        napi_value args = nullptr;
        NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &args, &thisVar, nullptr));
        NAPI_ASSERT(env, argc >= requireArgc, "Wrong nuamber of arguments");
        napi_valuetype valuetype;
        NAPI_CALL(env, napi_typeof(env, args, &valuetype));
        NAPI_ASSERT(env, valuetype == napi_string, "Wrong argument type. String expected");
        RationalNumber *object = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&object));
        if (RationalNumberClass == nullptr) {
            napi_throw_error(env, "NullException", "RationalNumberClass must not be null!");
        }
        return object->CreateRationalFromString(args, RationalNumberClass);
    }

    static napi_value CompareTo(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        size_t requireArgc = 1;
        size_t argc = 1;
        napi_value args = nullptr;
        NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &args, &thisVar, nullptr));
        NAPI_ASSERT(env, argc >= requireArgc, "Wrong nuamber of arguments");
        RationalNumber *object = nullptr;
        napi_unwrap(env, thisVar, (void**)&object);
        return object->CompareTo(args);
    }

    static napi_value Equals(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        size_t requireArgc = 1;
        size_t argc = 1;
        napi_value args = nullptr;
        NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &args, &thisVar, nullptr));
        NAPI_ASSERT(env, argc >= requireArgc, "Wrong nuamber of arguments");
        RationalNumber *object = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&object));
        return object->Equals(args);
    }

    static napi_value Value(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
        RationalNumber *object = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&object));
        return object->Value();
    }

    static napi_value GetCommonDivisor(napi_env env, napi_callback_info info)
    {
        size_t argc = 2;
        napi_value argv[2] = {0};
        napi_value thisVar = nullptr;
        void *data = nullptr;
        napi_value args = nullptr;
        napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);
        NAPI_CALL(env, napi_get_cb_info(env, info, &argc, &args, &thisVar, nullptr));
        napi_valuetype valuetype1;
        napi_valuetype valuetype2;
        NAPI_CALL(env, napi_typeof(env, argv[0], &valuetype1));
        NAPI_CALL(env, napi_typeof(env, argv[1], &valuetype2));
        NAPI_ASSERT(env, valuetype1 == napi_number, "Wrong argument type. String expected");
        NAPI_ASSERT(env, valuetype2 == napi_number, "Wrong argument type. String expected");
        RationalNumber *object = nullptr;
        napi_unwrap(env, thisVar, (void**)&object);
        napi_value result = nullptr;
        result = object->GetCommonDivisor(argv[0], argv[1]);
        return result;
    }

    static napi_value GetDenominator(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
        RationalNumber *object = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&object));
        napi_value result = object->GetDenominator();
        return result;
    }

    static napi_value GetNumerator(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
        RationalNumber *object = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&object));
        napi_value result = object->GetNumerator();
        return result;
    }

    static napi_value IsFinite(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr));
        RationalNumber *object = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&object));
        napi_value result = object->IsFinite();
        return result;
    }

    static napi_value IsNaN(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
        RationalNumber *object = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&object));
        napi_value result = object->IsNaN();
        return result;
    }
    static napi_value IsZero(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
        RationalNumber *object = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&object));
        napi_value result = object->IsZero();
        return result;
    }

    static napi_value ToString(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, nullptr);
        RationalNumber *object = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&object));
        napi_value result = object->ToString();
        return result;
    }

    static napi_value RationalNumberInit(napi_env env, napi_value exports)
    {
        const char *RationalNumberClassName = "RationalNumber";
        static napi_property_descriptor RationalNumberDesc[] = {
            DECLARE_NAPI_FUNCTION("createRationalFromString", CreateRationalFromString),
            DECLARE_NAPI_FUNCTION("compareTo", CompareTo),
            DECLARE_NAPI_FUNCTION("equals", Equals),
            DECLARE_NAPI_FUNCTION("value", Value),
            DECLARE_NAPI_FUNCTION("getCommonDivisor", GetCommonDivisor),
            DECLARE_NAPI_FUNCTION("getDenominator", GetDenominator),
            DECLARE_NAPI_FUNCTION("getNumerator", GetNumerator),
            DECLARE_NAPI_FUNCTION("isFinite", IsFinite),
            DECLARE_NAPI_FUNCTION("isNaN", IsNaN),
            DECLARE_NAPI_FUNCTION("isZero", IsZero),
            DECLARE_NAPI_FUNCTION("toString", ToString),
        };
        NAPI_CALL(env, napi_define_class(env, RationalNumberClassName, strlen(RationalNumberClassName),
                                         RationalNumberConstructor, nullptr,
                                         sizeof(RationalNumberDesc) / sizeof(RationalNumberDesc[0]),
                                         RationalNumberDesc, &RationalNumberClass));
        static napi_property_descriptor desc[] = {
            DECLARE_NAPI_PROPERTY("RationalNumber", RationalNumberClass)
        };
        NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
        return exports;
    }

    static napi_value TextcoderInit(napi_env env, napi_value exports)
    {
        const char *textEncoderClassName = "TextEncoder";
        napi_value textEncoderClass = nullptr;
        static napi_property_descriptor textEncoderDesc[] = {
            DECLARE_NAPI_GETTER("encoding", GetEncoding),
            DECLARE_NAPI_FUNCTION("encode", Encode),
            DECLARE_NAPI_FUNCTION("encodeInto", EncodeInto),
        };
        NAPI_CALL(env, napi_define_class(env, textEncoderClassName, strlen(textEncoderClassName),
                                         TextEncoderConstructor, nullptr,
                                         sizeof(textEncoderDesc) / sizeof(textEncoderDesc[0]),
                                         textEncoderDesc, &textEncoderClass));

        const char *textDecoderClassName = "TextDecoder";
        napi_value textDecoderClass = nullptr;
        static napi_property_descriptor textdecoderDesc[] = {
            DECLARE_NAPI_FUNCTION("decode", TextdecoderDecode),
            DECLARE_NAPI_GETTER("encoding", TextdecoderGetEncoding),
            DECLARE_NAPI_GETTER("fatal", TextdecoderGetFatal),
            DECLARE_NAPI_GETTER("ignoreBOM", TextdecoderGetIgnoreBOM),
        };
        NAPI_CALL(env, napi_define_class(env, textDecoderClassName, strlen(textDecoderClassName),
                                         TextdecoderConstructor, nullptr,
                                         sizeof(textdecoderDesc) / sizeof(textdecoderDesc[0]),
                                         textdecoderDesc, &textDecoderClass));

        static napi_property_descriptor desc[] = {
            DECLARE_NAPI_PROPERTY("TextEncoder", textEncoderClass),
            DECLARE_NAPI_PROPERTY("TextDecoder", textDecoderClass),
        };

        NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
        return exports;
    }

    static napi_value Base64Constructor(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        void *data = nullptr;
        NAPI_CALL(env, napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, &data));
        auto objectInfo = new Base64(env);
        napi_wrap(
            env, thisVar, objectInfo,
            [](napi_env env, void *data, void *hint) {
                auto objInfo = (Base64*)data;
                if (objInfo != nullptr) {
                    delete objInfo;
                }
            },
            nullptr, nullptr);
        return thisVar;
    }

    static napi_value EncodeBase64(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        size_t requireArgc = 2;
        size_t argc = 2;
        napi_value args[2] = { nullptr };
        NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &thisVar, nullptr));
        NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");
        napi_typedarray_type valuetype0;
        size_t length = 0;
        void *data = nullptr;
        napi_value arraybuffer = nullptr;
        size_t byteOffset = 0;
        NAPI_CALL(env, napi_get_typedarray_info(env, args[0], &valuetype0, &length, &data, &arraybuffer, &byteOffset));
        napi_valuetype valuetype1;
        NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));
        NAPI_ASSERT(env, valuetype0 == napi_uint8_array, "Wrong argument type. napi_uint8_array expected.");
        NAPI_ASSERT(env, valuetype1 == napi_number, "Wrong argument type. Nmuber expected.");
        Base64 *object = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&object));
        napi_value result = object->Encode(args[0], args[1]);
        return result;
    }

    static napi_value EncodeToString(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        size_t requireArgc = 2;
        size_t argc = 2;
        napi_value args[2] = { nullptr };
        NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &thisVar, nullptr));
        NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");
        napi_typedarray_type valuetype0;
        size_t length = 0;
        void *data = nullptr;
        napi_value arraybuffer = nullptr;
        size_t byteOffset = 0;
        NAPI_CALL(env, napi_get_typedarray_info(env, args[0], &valuetype0, &length, &data, &arraybuffer, &byteOffset));
        napi_valuetype valuetype1;
        NAPI_CALL(env, napi_typeof(env, args[1], &valuetype1));
        NAPI_ASSERT(env, valuetype0 == napi_uint8_array, "Wrong argument type. napi_uint8_array expected.");
        NAPI_ASSERT(env, valuetype1 == napi_number, "Wrong argument type. Nmuber expected.");
        Base64 *object = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&object));
        napi_value result = object->EncodeToString(args[0], args[1]);
        return result;
    }

    static napi_value DecodeBase64(napi_env env, napi_callback_info info)
    {
        napi_value thisVar = nullptr;
        size_t requireArgc = 2;
        size_t argc = 2;
        napi_value args[2] = { nullptr };
        NAPI_CALL(env, napi_get_cb_info(env, info, &argc, args, &thisVar, nullptr));
        NAPI_ASSERT(env, argc >= requireArgc, "Wrong number of arguments");
        napi_typedarray_type valuetype0;
        napi_valuetype valuetype1;
        size_t length = 0;
        void *data = nullptr;
        napi_value arraybuffer = nullptr;
        size_t byteOffset = 0;
        NAPI_CALL(env, napi_typeof(env, args[0], &valuetype1));
        if (valuetype1 != napi_valuetype::napi_string) {
            NAPI_CALL(env, napi_get_typedarray_info(env, args[0], &valuetype0, &length,
                                                    &data, &arraybuffer, &byteOffset));
        }
        if ((valuetype1 != napi_valuetype::napi_string) && (valuetype0 != napi_typedarray_type::napi_uint8_array)) {
            napi_throw_error(env, nullptr, "The parameter type is incorrect");
        }
        napi_valuetype valuetype2;
        NAPI_CALL(env, napi_typeof(env, args[1], &valuetype2));
        NAPI_ASSERT(env, valuetype2 == napi_number, "Wrong argument type. Nmuber expected.");
        Base64 *object = nullptr;
        NAPI_CALL(env, napi_unwrap(env, thisVar, (void**)&object));
        napi_value result = object->Decode(args[0], args[1]);
        return result;
    }

    static napi_value Base64Init(napi_env env, napi_value exports)
    {
        const char *base64ClassName = "Base64";
        napi_value base64Class = nullptr;
        static napi_property_descriptor base64Desc[] = {
            DECLARE_NAPI_FUNCTION("encode", EncodeBase64),
            DECLARE_NAPI_FUNCTION("encodeToString", EncodeToString),
            DECLARE_NAPI_FUNCTION("decode", DecodeBase64),
        };
        NAPI_CALL(env, napi_define_class(env, base64ClassName, strlen(base64ClassName), Base64Constructor,
                                         nullptr, sizeof(base64Desc) / sizeof(base64Desc[0]), base64Desc,
                                         &base64Class));
        static napi_property_descriptor desc[] = {
            DECLARE_NAPI_PROPERTY("Base64", base64Class)
        };
        NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
        return exports;
    }

    static napi_value UtilInit(napi_env env, napi_value exports)
    {
        static napi_property_descriptor desc[] = {
            DECLARE_NAPI_FUNCTION("printf", Printf),
            DECLARE_NAPI_FUNCTION("geterrorstring", GetErrorString),
            DECLARE_NAPI_FUNCTION("dealwithformatstring", DealWithFormatString),
        };
        NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));
        TextcoderInit(env, exports);
        RationalNumberInit(env, exports);
        Base64Init(env, exports);
        return exports;
    }

    // util module define
    static napi_module utilModule = {
        .nm_version = 1,
        .nm_flags = 0,
        .nm_filename = nullptr,
        .nm_register_func = UtilInit,
        .nm_modname = "util",
        .nm_priv = ((void*)0),
        .reserved = {0},
    };

    // util module register
    extern "C"
    __attribute__((constructor)) void RegisterModule()
    {
        napi_module_register(&utilModule);
    }

    // util JS register
    extern "C"
    __attribute__((visibility("default"))) void NAPI_util_GetJSCode(const char **buf, int *buflen)
    {
        if (buf != nullptr) {
            *buf = _binary_util_js_js_start;
        }
        if (buflen != nullptr) {
            *buflen = _binary_util_js_js_end - _binary_util_js_js_start;
        }
    }
}