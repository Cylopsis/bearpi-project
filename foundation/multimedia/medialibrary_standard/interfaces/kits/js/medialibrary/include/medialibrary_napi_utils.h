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

#ifndef MEDIALIBRARY_NAPI_UTILS_H
#define MEDIALIBRARY_NAPI_UTILS_H

#include "napi/native_api.h"
#include "napi/native_node_api.h"

#define GET_JS_ARGS(env, info, argc, argv, thisVar)                 \
    do {                                                            \
        void* data;                                                 \
        napi_get_cb_info(env, info, &argc, argv, &thisVar, &data);  \
    } while (0)

#define GET_JS_OBJ_WITH_ZERO_ARGS(env, info, status, thisVar)                       \
    do {                                                                            \
        void* data;                                                                 \
        status = napi_get_cb_info(env, info, nullptr, nullptr, &thisVar, &data);    \
    } while (0)

#define GET_JS_ASYNC_CB_REF(env, arg, count, cbRef)         \
    do {                                                    \
        napi_valuetype valueType = napi_undefined;          \
        napi_typeof(env, arg, &valueType);                  \
        if (valueType == napi_function) {                   \
            napi_create_reference(env, arg, count, &cbRef); \
        } else {                                            \
            NAPI_ASSERT(env, false, "type mismatch");       \
        }                                                   \
    } while (0);

#define ASSERT_NULLPTR_CHECK(env, result)       \
    do {                                        \
        if (result == nullptr) {                \
            napi_get_undefined(env, &result);   \
            return result;                      \
        }                                       \
    } while (0);

#define NAPI_CREATE_PROMISE(env, callbackRef, deferred, result)     \
    do {                                                            \
        if (callbackRef == nullptr) {                               \
            napi_create_promise(env, &deferred, &result);           \
        }                                                           \
    } while (0);

#define NAPI_CREATE_RESOURCE_NAME(env, resource, resourceName)                      \
    do {                                                                            \
        napi_create_string_utf8(env, resourceName, NAPI_AUTO_LENGTH, &resource);    \
    } while (0);

/* Constants for array index */
const int PARAM0 = 0;
const int PARAM1 = 1;

/* Constants for array size */
const int ARGS_ONE = 1;
const int ARGS_TWO = 2;
const int SIZE = 100;
const int32_t REFERENCE_COUNT_ONE = 1;

namespace OHOS {
const std::string ALBUM_ROOT_PATH = "/data/media";

enum AssetType {
    TYPE_AUDIO = 0,
    TYPE_VIDEO = 1,
    TYPE_IMAGE = 2,
    TYPE_ALBUM = 3,
};

enum AlbumType {
    TYPE_VIDEO_ALBUM = 0,
    TYPE_IMAGE_ALBUM = 1,
    TYPE_NONE = 2,
};

/* Util class used by napi asynchronous methods for making call to js callback function */
class MediaLibraryNapiUtils {
public:
    static void InvokeJSAsyncMethod(napi_env env, napi_deferred deferred, napi_value result[],
                                    size_t argc, napi_ref callbackRef, napi_async_work work)
    {
        napi_value retVal;
        napi_value callback = nullptr;

        /* Deferred is used when JS Callback method expects a promise value */
        if (deferred) {
            napi_resolve_deferred(env, deferred, result[PARAM1]);
        } else {
            napi_get_reference_value(env, callbackRef, &callback);
            napi_call_function(env, nullptr, callback, argc, result, &retVal);
            napi_delete_reference(env, callbackRef);
        }
        napi_delete_async_work(env, work);
    }
};
} // namespace OHOS
#endif /* MEDIALIBRARY_NAPI_UTILS_H */
