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

#ifndef FOUNDATION_CCRUNTIME_TEXTCODER_JS_TEXTENCODER_H
#define FOUNDATION_CCRUNTIME_TEXTCODER_JS_TEXTENCODER_H

#include <string>

#include "napi/native_api.h"
#include "napi/native_node_api.h"
namespace OHOS::Util {
    class TextEncoder {
    public:
        explicit TextEncoder(napi_env env);

        virtual ~TextEncoder() {}

        napi_value GetEncoding() const;
        napi_value Encode(napi_value src) const;
        napi_value EncodeInto(napi_value src, napi_value dest) const;

    private:
        napi_env env_;
        std::string encoding_;
    };
}
#endif /* FOUNDATION_CCRUNTIME_TEXTCODER_JS_TEXTENCODER_H */