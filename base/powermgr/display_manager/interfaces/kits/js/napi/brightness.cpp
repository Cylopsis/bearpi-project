/*
 * Copyright (c) 2021 Huawei Device Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdio>
#include <cstdlib>
#include <string>

#include <napi/native_api.h>
#include <napi/native_node_api.h>

#include "display_manager.h"
#include "hilog_wrapper.h"

using namespace OHOS::DisplayMgr;

struct BrightnessAsyncCallbackInfo {
    napi_env env;
    napi_async_work asyncWork;
    int32_t value;
};

static napi_value SetValue(napi_env env, napi_callback_info info)
{
    DISPLAY_HILOGD(MODULE_JS_NAPI, "enter");
    size_t argc = 1;
    napi_value args[1] = { 0 };
    napi_value jsthis;
    void *data = nullptr;

    napi_status status = napi_get_cb_info(env, info, &argc, args, &jsthis, &data);
    NAPI_ASSERT(env, (status == napi_ok) && (argc >= 1), "Failed to get cb info");

    napi_valuetype type;
    NAPI_CALL(env, napi_typeof(env, args[0], &type));

    NAPI_ASSERT(env, type == napi_number, "Wrong argument type. Numbers expected.");

    int32_t value = 0;
    NAPI_CALL(env, napi_get_value_int32(env, args[0], &value));

    BrightnessAsyncCallbackInfo* asyncCallbackInfo = new BrightnessAsyncCallbackInfo {
        .env = env,
        .asyncWork = nullptr,
    };

    asyncCallbackInfo->value = value;

    napi_value resourceName;
    napi_create_string_latin1(env, "setValue", NAPI_AUTO_LENGTH, &resourceName);

    napi_create_async_work(
        env,
        nullptr,
        resourceName,
        [](napi_env env, void *data) {
            BrightnessAsyncCallbackInfo* asyncCallbackInfo = (BrightnessAsyncCallbackInfo *)data;
            if (!DisplayManager::SetBrightness(asyncCallbackInfo->value)) {
                DISPLAY_HILOGE(MODULE_JS_NAPI, "Failed to set brightness");
            } else {
                DISPLAY_HILOGD(MODULE_JS_NAPI, "Succeed to set brightness");
            }
        },
        [](napi_env env, napi_status status, void *data) {
            BrightnessAsyncCallbackInfo* asyncCallbackInfo = (BrightnessAsyncCallbackInfo *)data;
            napi_delete_async_work(env, asyncCallbackInfo->asyncWork);
            delete asyncCallbackInfo;
        },
        (void *)asyncCallbackInfo,
        &asyncCallbackInfo->asyncWork);

    NAPI_CALL(env, napi_queue_async_work(env, asyncCallbackInfo->asyncWork));

    DISPLAY_HILOGD(MODULE_JS_NAPI, "return");
    return nullptr;
}

EXTERN_C_START
static napi_value Init(napi_env env, napi_value exports)
{
    napi_property_descriptor desc[] = {
        DECLARE_NAPI_FUNCTION("setValue", SetValue),
    };
    NAPI_CALL(env, napi_define_properties(env, exports, sizeof(desc) / sizeof(desc[0]), desc));

    DISPLAY_HILOGD(MODULE_JS_NAPI, "return");

    return exports;
}
EXTERN_C_END

static napi_module g_module = {
    .nm_version = 1,
    .nm_flags = 0,
    .nm_filename = "brightness",
    .nm_register_func = Init,
    .nm_modname = "brightness",
    .nm_priv = ((void *)0),
    .reserved = { 0 }
};

extern "C" __attribute__((constructor)) void RegisterModule(void)
{
    napi_module_register(&g_module);
}
