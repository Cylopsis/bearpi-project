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

#include "js_base64.h"
#include <cstring>
#include <sys/types.h>
#include "utils/log.h"
#include "securec.h"
#include "napi/native_api.h"
#include "napi/native_node_api.h"

namespace OHOS::Util {
    namespace {
        static const size_t TRAGET_TWO = 2;
        static const size_t TRAGET_THREE = 3;
        static const size_t TRAGET_FOUR = 4;
        static const size_t TRAGET_SIX = 6;
        static const size_t TRAGET_EIGHT = 8;
        const char base[] = {
            65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82,
            83, 84, 85, 86, 87, 88, 89, 90, 97, 98, 99, 100, 101, 102, 103, 104, 105,
            106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120,
            121, 122, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 43, 47, 61
        };
        const char base0[] = {
            65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82,
            83, 84, 85, 86, 87, 88, 89, 90, 97, 98, 99, 100, 101, 102, 103, 104, 105,
            106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120,
            121, 122, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 45, 95, 61
        };
    }
    Base64::Base64(napi_env env_) : env(env_) {}

    /* base64 encode */
    napi_value Base64::Encode(napi_value src, napi_value flags)
    {
        napi_typedarray_type type;
        size_t byteOffset = 0;
        size_t length = 0;
        void *resultData = nullptr;
        napi_value resultBuffer = nullptr;
        NAPI_CALL(env, napi_get_typedarray_info(env, src, &type, &length, &resultData, &resultBuffer, &byteOffset));
        inputEncode = static_cast<const unsigned char*>(resultData) + byteOffset;
        int32_t iflag = 0;
        NAPI_CALL(env, napi_get_value_int32(env, flags, &iflag));
        size_t flag = 0;
        flag = static_cast<size_t>(iflag);
        const unsigned char *rets = EncodeAchieve(inputEncode, length, flag);
        void *data = nullptr;
        napi_value arrayBuffer = nullptr;
        size_t bufferSize = outputLen;
        NAPI_CALL(env, napi_create_arraybuffer(env, bufferSize, &data, &arrayBuffer));
        if (memcpy_s(data, bufferSize, reinterpret_cast<const void*>(rets), bufferSize) != 0) {
            FreeMemory(rets);
            HILOG_ERROR("copy ret to arraybuffer error");
            return nullptr;
        }
        napi_value result = nullptr;
        NAPI_CALL(env, napi_create_typedarray(env, napi_uint8_array, bufferSize, arrayBuffer, 0, &result));
        FreeMemory(rets);
        return result;
    }

    /* base64 encodeToString */
    napi_value Base64::EncodeToString(napi_value src, napi_value flags)
    {
        napi_typedarray_type type;
        size_t byteOffset = 0;
        size_t length = 0;
        void *resultData = nullptr;
        napi_value resultBuffer = nullptr;
        NAPI_CALL(env, napi_get_typedarray_info(env, src, &type, &length, &resultData, &resultBuffer, &byteOffset));
        inputEncode = static_cast<const unsigned char*>(resultData) + byteOffset;
        int32_t iflag = 0;
        NAPI_CALL(env, napi_get_value_int32(env, flags, &iflag));
        size_t flag = 0;
        flag = static_cast<size_t>(iflag);
        unsigned char *ret = EncodeAchieve(inputEncode, length, flag);
        char *rstring = nullptr;
        if (outputLen > 0) {
            rstring = new char[outputLen + 1];
            if (memset_s(rstring, outputLen + 1, '\0', outputLen + 1) != 0) {
                FreeMemory(ret);
                FreeMemory(rstring);
                napi_throw_error(env, "-1", "decode rstring memset_s failed");
            }
        } else {
            napi_throw_error(env, "-2", "outputLen is error !");
        }
        for (size_t i = 0; i < outputLen; i++) {
            rstring[i] = char(ret[i]);
        }
        std::string finalString = rstring;
        const char *outString = finalString.c_str();
        const char *encString = static_cast<const char*>(outString);
        napi_value resultStr = nullptr;
        NAPI_CALL(env, napi_create_string_utf8(env, encString, strlen(encString), &resultStr));
        FreeMemory(ret);
        FreeMemory(rstring);
        return resultStr;
    }

    unsigned char *Base64::EncodeAchieve(const unsigned char *input, size_t inputLen, size_t iflag)
    {
        size_t inp = 0;
        size_t temp = 0;
        size_t bitWise = 0;
        unsigned char *ret = nullptr;
        unsigned char *bosom = nullptr;
        outputLen = (inputLen / TRAGET_THREE) * TRAGET_FOUR;
        if ((inputLen % TRAGET_THREE) > 0) {
            outputLen += TRAGET_FOUR;
        }
        if (outputLen > 0) {
            ret = new unsigned char[outputLen + 1];
            if (memset_s(ret, outputLen + 1, '\0', outputLen + 1) != 0) {
                FreeMemory(ret);
                napi_throw_error(env, "-1", "ret path memset_s failed");
            }
        } else {
            napi_throw_error(env, "-2", "outputLen is error !");
        }
        bosom = ret;
        while (inp < inputLen) {
            temp = 0;
            bitWise = 0;
            while (temp < TRAGET_THREE) {
                if (inp >= inputLen) {
                    break;
                }
                bitWise = ((bitWise << TRAGET_EIGHT) | (input[inp] & XFF_FLG));
                inp++;
                temp++;
            }
            bitWise = (bitWise << ((TRAGET_THREE - temp) * TRAGET_EIGHT));
            for (size_t i = 0; i < TRAGET_FOUR; i++) {
                if (temp < i && iflag == 0) {
                    *bosom = base[BIT_FLG];
                } else if (temp < i && iflag != 0) {
                    *bosom = base0[BIT_FLG];
                } else if (temp >= i && iflag == 0) {
                    *bosom = base[(bitWise >> ((TRAGET_THREE - i) * TRAGET_SIX)) & SIXTEEN_FLG];
                } else if (temp >= i && iflag != 0) {
                    *bosom = base0[(bitWise >> ((TRAGET_THREE - i) * TRAGET_SIX)) & SIXTEEN_FLG];
                }
                bosom++;
            }
        }
        *bosom = '\0';
        return ret;
    }

    /* base64 decode */
    napi_value Base64::Decode(napi_value src, napi_value flags)
    {
        napi_valuetype valuetype = napi_undefined;
        napi_typeof(env, src, &valuetype);
        napi_typedarray_type type;
        size_t byteOffset = 0;
        size_t length = 0;
        void *resultData = nullptr;
        napi_value resultBuffer = nullptr;
        char *inputString = nullptr;
        if (valuetype != napi_valuetype::napi_string) {
            NAPI_CALL(env, napi_get_typedarray_info(env, src, &type, &length, &resultData, &resultBuffer, &byteOffset));
        }
        int32_t iflag = 0;
        NAPI_CALL(env, napi_get_value_int32(env, flags, &iflag));
        size_t flag = 0;
        flag = static_cast<size_t>(iflag);
        if (valuetype == napi_valuetype::napi_string) {
            size_t prolen = 0;
            napi_get_value_string_utf8(env, src, nullptr, 0, &prolen);
            if (prolen > 0) {
                inputString = new char[prolen + 1];
                if (memset_s(inputString, prolen + 1, '\0', prolen + 1) != 0) {
                    FreeMemory(inputString);
                    napi_throw_error(env, "-1", "decode inputString memset_s failed");
                }
            } else {
                napi_throw_error(env, "-2", "prolen is error !");
            }
            napi_get_value_string_utf8(env, src, inputString, prolen + 1, &prolen);
            pret = DecodeAchieve(inputString, prolen, flag);
        } else if (type == napi_typedarray_type::napi_uint8_array) {
            inputDecode = static_cast<const char*>(resultData) + byteOffset;
            pret = DecodeAchieve(inputDecode, length, flag);
        }
        void *data = nullptr;
        napi_value arrayBuffer = nullptr;
        size_t bufferSize = decodeOutLen;
        NAPI_CALL(env, napi_create_arraybuffer(env, bufferSize, &data, &arrayBuffer));
        if (memcpy_s(data, bufferSize, reinterpret_cast<const void*>(pret), bufferSize) != 0) {
            FreeMemory(inputString);
            FreeMemory(pret);
            HILOG_ERROR("copy retDecode to arraybuffer error");
            return nullptr;
        }
        napi_value result = nullptr;
        NAPI_CALL(env, napi_create_typedarray(env, napi_uint8_array, bufferSize, arrayBuffer, 0, &result));
        FreeMemory(inputString);
        FreeMemory(pret);
        return result;
    }

    unsigned char *Base64::DecodeAchieve(const char *input, size_t inputLen, size_t iflag)
    {
        retLen = (inputLen / TRAGET_FOUR) * TRAGET_THREE;
        decodeOutLen = retLen;
        size_t equalCount = 0;
        unsigned char *bosom = nullptr;
        size_t inp = 0;
        size_t temp = 0;
        size_t bitWise = 0;
        if (*(input + inputLen - 1) == '=') {
            equalCount++;
        }
        if (*(input + inputLen - TRAGET_TWO) == '=') {
            equalCount++;
        }
        if (*(input + inputLen - TRAGET_THREE) == '=') {
            equalCount++;
        }
        retLen = DecodeOut(equalCount, retLen);
        if (retLen > 0) {
            retDecode = new unsigned char[retLen + 1];
            if (memset_s(retDecode, retLen + 1, '\0', retLen + 1) != 0) {
                FreeMemory(retDecode);
                napi_throw_error(env, "-1", "decode retDecode memset_s failed");
            }
        } else {
            napi_throw_error(env, "-2", "retLen is error !");
        }
        bosom = retDecode;
        while (inp < (inputLen - equalCount)) {
            temp = 0;
            bitWise = 0;
            while (temp < TRAGET_FOUR) {
                if (inp >= (inputLen - equalCount)) {
                    break;
                }
                bitWise = (bitWise << TRAGET_SIX) | (Finds(input[inp], iflag));
                inp++;
                temp++;
            }
            bitWise = bitWise << ((TRAGET_FOUR - temp) * TRAGET_SIX);
            for (size_t i = 0; i < TRAGET_THREE; i++) {
                if (i == temp) {
                    break;
                }
                *bosom = static_cast<char>((bitWise >> ((TRAGET_TWO - i) * TRAGET_EIGHT)) & XFF_FLG);
                bosom++;
            }
        }
        *bosom = '\0';
        return retDecode;
    }

    size_t Base64::DecodeOut(size_t equalCount, size_t retLen)
    {
        size_t temp = retLen;
        if (equalCount == 1) {
            decodeOutLen -= 1;
        }
        if (equalCount == TRAGET_TWO) {
            decodeOutLen -= TRAGET_TWO;
        }
        switch (equalCount) {
            case 0:
                temp += TRAGET_FOUR;
                break;
            case 1:
                temp += TRAGET_FOUR;
                break;
            case TRAGET_TWO:
                temp += TRAGET_THREE;
                break;
            default:
                temp += TRAGET_TWO;
                break;
        }
        return temp;
    }

    /* Decoding lookup function */
    size_t Base64::Finds(char ch, size_t iflag)
    {
        size_t couts = 0;
        if (iflag == 0) {
        // 65:Number of elements in the encoding table.
            for (size_t i = 0; i < 65; i++) {
                if (base[i] == ch) {
                    couts = i;
                }
            }
        } else {
        // 65:Number of elements in the encoding table.
            for (size_t i = 0; i < 65; i++) {
                if (base0[i] == ch) {
                    couts = i;
                }
            }
        }
        return couts;
    }

    /* Memory cleanup function */
    void Base64::FreeMemory(const unsigned char *address)
    {
        const unsigned char *temp = address;
        if (temp != nullptr) {
            delete[] temp;
            temp = nullptr;
        }
    }
    void Base64::FreeMemory(const char *address)
    {
        const char *temp = address;
        if (temp != nullptr) {
            delete[] temp;
            temp = nullptr;
        }
    }
}