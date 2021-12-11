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
#include <sys/sysinfo.h>
#include <unistd.h>
#include "napi/native_api.h"
#include "napi/native_node_api.h"

#ifndef BASE_COMPILERUNTIME_JS_UTIL_MODULE_BASE64_CLASS_H
#define BASE_COMPILERUNTIME_JS_UTIL_MODULE_BASE64_CLASS_H

namespace OHOS::Util {
    class Base64 {
    public:
        enum ConverterFlags {
            BIT_FLG = 0x40,
            SIXTEEN_FLG = 0x3F,
            XFF_FLG = 0xFF,
        };
    public:
        explicit Base64(napi_env env);
        virtual ~Base64(){}
        napi_value Encode(napi_value src, napi_value flags);
        napi_value EncodeToString(napi_value src, napi_value flags);
        napi_value Decode(napi_value src, napi_value flags);
    private:
        napi_env env;
        unsigned char *DecodeAchieve(const char *input, size_t inputLen, size_t iflag);
        unsigned char *EncodeAchieve(const unsigned char *input, size_t inputLen, size_t iflag);
        size_t Finds(char ch, size_t iflag);
        size_t DecodeOut(size_t equalCount, size_t retLen);
        void FreeMemory(const unsigned char *address);
        void FreeMemory(const char *address);
        size_t retLen = 0;
        size_t decodeOutLen = 0;
        size_t outputLen = 0;
        unsigned char *pret = nullptr;
        const unsigned char *inputEncode = nullptr;
        const char *inputDecode = nullptr;
        unsigned char *retDecode = nullptr;
    };
}
#endif