/*
 * Copyright (c) 2020 Huawei Device Co., Ltd.
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

#include "app_module.h"
#include "ace_log.h"
#include "js_app_context.h"
#ifdef FEATURE_SCREEN_ON_VISIBLE
#include "js_async_work.h"
#include "product_adapter.h"
#endif

#include "hdf_sbuf.h"
#include "hdf_io_service_if.h"

#define E53_IS1_SERVICE "hdf_e53_is1"
#define E53_SF1_SERVICE "hdf_e53_sf1"
#define E53_SC2_SERVICE "hdf_e53_sc2"
#define E53_SC1_SERVICE "hdf_e53_sc1"
#define E53_IA1_SERVICE "hdf_e53_ia1"

namespace OHOS {
namespace ACELite {
const char * const AppModule::FILE_MANIFEST = "manifest.json";
const char * const AppModule::KEY_APP_NAME = "appName";
const char * const AppModule::KEY_VERSION_NAME = "versionName";
const char * const AppModule::KEY_VERSION_CODE = "versionCode";

#ifdef FEATURE_SCREEN_ON_VISIBLE
const char * const AppModule::SCREEN_ON_VISIBLE_KEY = "visible";
const char * const AppModule::SCREEN_ON_VISIBLE_DATA = "data";
const char * const AppModule::SCREEN_ON_VISIBLE_CODE = "code";
const char * const AppModule::SCREEN_ON_VISIBLE_INVALID_PARAMETER = "Incorrect parameter";
const uint8_t AppModule::SCREEN_ON_VISIBLE_ERR = 202;

struct AsyncParams : public MemoryHeap {
    ACE_DISALLOW_COPY_AND_MOVE(AsyncParams);
    AsyncParams() : result(nullptr), callback(nullptr), context(nullptr) {}

    JSIValue result;
    JSIValue callback;
    JSIValue context;
};
#endif

static int E53IS1Control(struct HdfIoService *serv, int32_t cmd, const char* buf, char **val)
{
    int ret = HDF_FAILURE;
    struct HdfSBuf *data = HdfSBufObtainDefaultSize();
    struct HdfSBuf *reply = HdfSBufObtainDefaultSize();

    if (data == NULL || reply == NULL) {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to obtain sbuf data\n");
        return ret;
    }

    if (!HdfSbufWriteString(data, buf))
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to write sbuf\n");
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;
    }

    ret = serv->dispatcher->Dispatch(&serv->object, cmd, data, reply);
    if (ret != HDF_SUCCESS)
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to send service call\n");
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;
    }
 
    *val = (char *)(HdfSbufReadString(reply));
    if(val ==NULL){
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to get service call reply\n");
        ret = HDF_ERR_INVALID_OBJECT;
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;

    }

    HILOG_ERROR(HILOG_MODULE_ACE,"Get reply is: %s\n", *val);

    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
    return ret;
}

JSIValue AppModule::E53IS1Service(const JSIValue thisVal, const JSIValue *args, uint8_t argsNum)
{
    struct HdfIoService *serv = HdfIoServiceBind(E53_IS1_SERVICE);
    if (serv == NULL)
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to get service %s\n", E53_IS1_SERVICE);
        return JSI::CreateUndefined();
    }

    if ((args == nullptr) || (argsNum == 0) || (JSI::ValueIsUndefined(args[0]))) {
        return JSI::CreateUndefined();
    }

    JSIValue success = JSI::GetNamedProperty(args[0], CB_SUCCESS);
    JSIValue fail = JSI::GetNamedProperty(args[0], CB_FAIL);
    JSIValue complete = JSI::GetNamedProperty(args[0], CB_COMPLETE);

    int32_t cmd = (int32_t)JSI::GetNumberProperty(args[0], "cmd");  
    char *data = (char *)JSI::GetStringProperty(args[0], "data");
    HILOG_ERROR(HILOG_MODULE_ACE, "cmd is: %d\n", cmd);
    HILOG_ERROR(HILOG_MODULE_ACE,"data is: %s\n", data);
    char *replyData;

    if (E53IS1Control(serv, cmd, data, &replyData))
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to send event\n");
        JSI::CallFunction(fail, thisVal, nullptr, 0);
        JSI::CallFunction(complete, thisVal, nullptr, 0);
        JSI::ReleaseValueList(success, fail, complete);
        return JSI::CreateUndefined();
    }

    JSIValue result = JSI::CreateObject();

    JSI::SetStringProperty(result, "e53_is1", replyData);
    JSIValue argv[ARGC_ONE] = {result};
    JSI::CallFunction(success, thisVal, argv, ARGC_ONE);
    JSI::CallFunction(complete, thisVal, nullptr, 0);
    JSI::ReleaseValueList(success, fail, complete, result);

    HdfIoServiceRecycle(serv);

    return JSI::CreateUndefined();
}

static int E53SF1Control(struct HdfIoService *serv, int32_t cmd, const char* buf, char **val)
{
    int ret = HDF_FAILURE;
    struct HdfSBuf *data = HdfSBufObtainDefaultSize();
    struct HdfSBuf *reply = HdfSBufObtainDefaultSize();

    if (data == NULL || reply == NULL) {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to obtain sbuf data\n");
        return ret;
    }

    if (!HdfSbufWriteString(data, buf))
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to write sbuf\n");
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;
    }

    ret = serv->dispatcher->Dispatch(&serv->object, cmd, data, reply);
    if (ret != HDF_SUCCESS)
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to send service call\n");
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;
    }
 
    *val = (char *)(HdfSbufReadString(reply));
    if(val ==NULL){
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to get service call reply\n");
        ret = HDF_ERR_INVALID_OBJECT;
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;

    }

    HILOG_ERROR(HILOG_MODULE_ACE,"Get reply is: %s\n", *val);

    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
    return ret;
}

JSIValue AppModule::E53SF1Service(const JSIValue thisVal, const JSIValue *args, uint8_t argsNum)
{
    struct HdfIoService *serv = HdfIoServiceBind(E53_SF1_SERVICE);
    if (serv == NULL)
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to get service %s\n", E53_SF1_SERVICE);
        return JSI::CreateUndefined();
    }

    if ((args == nullptr) || (argsNum == 0) || (JSI::ValueIsUndefined(args[0]))) {
        return JSI::CreateUndefined();
    }

    JSIValue success = JSI::GetNamedProperty(args[0], CB_SUCCESS);
    JSIValue fail = JSI::GetNamedProperty(args[0], CB_FAIL);
    JSIValue complete = JSI::GetNamedProperty(args[0], CB_COMPLETE);

    int32_t cmd = (int32_t)JSI::GetNumberProperty(args[0], "cmd");  
    char *data = (char *)JSI::GetStringProperty(args[0], "data");
    HILOG_ERROR(HILOG_MODULE_ACE, "cmd is: %d\n", cmd);
    HILOG_ERROR(HILOG_MODULE_ACE,"data is: %s\n", data);
    char *replyData;

    if (E53SF1Control(serv, cmd, data, &replyData))
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to send event\n");
        JSI::CallFunction(fail, thisVal, nullptr, 0);
        JSI::CallFunction(complete, thisVal, nullptr, 0);
        JSI::ReleaseValueList(success, fail, complete);
        return JSI::CreateUndefined();
    }

    JSIValue result = JSI::CreateObject();

    JSI::SetStringProperty(result, "e53_sf1", replyData);
    JSIValue argv[ARGC_ONE] = {result};
    JSI::CallFunction(success, thisVal, argv, ARGC_ONE);
    JSI::CallFunction(complete, thisVal, nullptr, 0);
    JSI::ReleaseValueList(success, fail, complete, result);

    HdfIoServiceRecycle(serv);

    return JSI::CreateUndefined();
}

static int E53SC2Control(struct HdfIoService *serv, int32_t cmd, const char* buf, char **val)
{
    int ret = HDF_FAILURE;
    struct HdfSBuf *data = HdfSBufObtainDefaultSize();
    struct HdfSBuf *reply = HdfSBufObtainDefaultSize();

    if (data == NULL || reply == NULL) {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to obtain sbuf data\n");
        return ret;
    }

    if (!HdfSbufWriteString(data, buf))
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to write sbuf\n");
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;
    }

    ret = serv->dispatcher->Dispatch(&serv->object, cmd, data, reply);
    if (ret != HDF_SUCCESS)
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to send service call\n");
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;
    }
 
    *val = (char *)(HdfSbufReadString(reply));
    if(val ==NULL){
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to get service call reply\n");
        ret = HDF_ERR_INVALID_OBJECT;
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;

    }

    HILOG_ERROR(HILOG_MODULE_ACE,"Get reply is: %s\n", *val);

    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
    return ret;
}

JSIValue AppModule::E53SC2Service(const JSIValue thisVal, const JSIValue *args, uint8_t argsNum)
{
    struct HdfIoService *serv = HdfIoServiceBind(E53_SC2_SERVICE);
    if (serv == NULL)
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to get service %s\n", E53_SC2_SERVICE);
        return JSI::CreateUndefined();
    }

    if ((args == nullptr) || (argsNum == 0) || (JSI::ValueIsUndefined(args[0]))) {
        return JSI::CreateUndefined();
    }

    JSIValue success = JSI::GetNamedProperty(args[0], CB_SUCCESS);
    JSIValue fail = JSI::GetNamedProperty(args[0], CB_FAIL);
    JSIValue complete = JSI::GetNamedProperty(args[0], CB_COMPLETE);

    int32_t cmd = (int32_t)JSI::GetNumberProperty(args[0], "cmd");  
    char *data = (char *)JSI::GetStringProperty(args[0], "data");
    HILOG_ERROR(HILOG_MODULE_ACE, "cmd is: %d\n", cmd);
    HILOG_ERROR(HILOG_MODULE_ACE,"data is: %s\n", data);
    char *replyData;

    if (E53SC2Control(serv, cmd, data, &replyData))
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to send event\n");
        JSI::CallFunction(fail, thisVal, nullptr, 0);
        JSI::CallFunction(complete, thisVal, nullptr, 0);
        JSI::ReleaseValueList(success, fail, complete);
        return JSI::CreateUndefined();
    }

    JSIValue result = JSI::CreateObject();

    JSI::SetStringProperty(result, "e53_sc2", replyData);
    JSIValue argv[ARGC_ONE] = {result};
    JSI::CallFunction(success, thisVal, argv, ARGC_ONE);
    JSI::CallFunction(complete, thisVal, nullptr, 0);
    JSI::ReleaseValueList(success, fail, complete, result);

    HdfIoServiceRecycle(serv);

    return JSI::CreateUndefined();
}


static int E53SC1Control(struct HdfIoService *serv, int32_t cmd, const char* buf, char **val)
{
    int ret = HDF_FAILURE;
    struct HdfSBuf *data = HdfSBufObtainDefaultSize();
    struct HdfSBuf *reply = HdfSBufObtainDefaultSize();

    if (data == NULL || reply == NULL) {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to obtain sbuf data\n");
        return ret;
    }

    if (!HdfSbufWriteString(data, buf))
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to write sbuf\n");
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;
    }

    ret = serv->dispatcher->Dispatch(&serv->object, cmd, data, reply);
    if (ret != HDF_SUCCESS)
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to send service call\n");
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;
    }
 
    *val = (char *)(HdfSbufReadString(reply));
    if(val ==NULL){
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to get service call reply\n");
        ret = HDF_ERR_INVALID_OBJECT;
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;

    }

    HILOG_ERROR(HILOG_MODULE_ACE,"Get reply is: %s\n", *val);

    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
    return ret;
}

JSIValue AppModule::E53SC1Service(const JSIValue thisVal, const JSIValue *args, uint8_t argsNum)
{
    struct HdfIoService *serv = HdfIoServiceBind(E53_SC1_SERVICE);
    if (serv == NULL)
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to get service %s\n", E53_SC1_SERVICE);
        return JSI::CreateUndefined();
    }

    if ((args == nullptr) || (argsNum == 0) || (JSI::ValueIsUndefined(args[0]))) {
        return JSI::CreateUndefined();
    }

    JSIValue success = JSI::GetNamedProperty(args[0], CB_SUCCESS);
    JSIValue fail = JSI::GetNamedProperty(args[0], CB_FAIL);
    JSIValue complete = JSI::GetNamedProperty(args[0], CB_COMPLETE);

    int32_t cmd = (int32_t)JSI::GetNumberProperty(args[0], "cmd");  
    char *data = (char *)JSI::GetStringProperty(args[0], "data");
    HILOG_ERROR(HILOG_MODULE_ACE, "cmd is: %d\n", cmd);
    HILOG_ERROR(HILOG_MODULE_ACE,"data is: %s\n", data);
    char *replyData;

    if (E53SC1Control(serv, cmd, data, &replyData))
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to send event\n");
        JSI::CallFunction(fail, thisVal, nullptr, 0);
        JSI::CallFunction(complete, thisVal, nullptr, 0);
        JSI::ReleaseValueList(success, fail, complete);
        return JSI::CreateUndefined();
    }

    JSIValue result = JSI::CreateObject();

    JSI::SetStringProperty(result, "e53_sc1", replyData);
    JSIValue argv[ARGC_ONE] = {result};
    JSI::CallFunction(success, thisVal, argv, ARGC_ONE);
    JSI::CallFunction(complete, thisVal, nullptr, 0);
    JSI::ReleaseValueList(success, fail, complete, result);

    HdfIoServiceRecycle(serv);

    return JSI::CreateUndefined();
}


static int E53IA1Control(struct HdfIoService *serv, int32_t cmd, const char* buf, char **val)
{
    int ret = HDF_FAILURE;
    struct HdfSBuf *data = HdfSBufObtainDefaultSize();
    struct HdfSBuf *reply = HdfSBufObtainDefaultSize();

    if (data == NULL || reply == NULL) {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to obtain sbuf data\n");
        return ret;
    }

    if (!HdfSbufWriteString(data, buf))
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to write sbuf\n");
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;
    }

    ret = serv->dispatcher->Dispatch(&serv->object, cmd, data, reply);
    if (ret != HDF_SUCCESS)
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to send service call\n");
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;
    }
 
    *val = (char *)(HdfSbufReadString(reply));
    if(val ==NULL){
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to get service call reply\n");
        ret = HDF_ERR_INVALID_OBJECT;
        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;

    }

    HILOG_ERROR(HILOG_MODULE_ACE,"Get reply is: %s\n", *val);

    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
    return ret;
}

JSIValue AppModule::E53IA1Service(const JSIValue thisVal, const JSIValue *args, uint8_t argsNum)
{
    struct HdfIoService *serv = HdfIoServiceBind(E53_IA1_SERVICE);
    if (serv == NULL)
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to get service %s\n", E53_IA1_SERVICE);
        return JSI::CreateUndefined();
    }

    if ((args == nullptr) || (argsNum == 0) || (JSI::ValueIsUndefined(args[0]))) {
        return JSI::CreateUndefined();
    }

    JSIValue success = JSI::GetNamedProperty(args[0], CB_SUCCESS);
    JSIValue fail = JSI::GetNamedProperty(args[0], CB_FAIL);
    JSIValue complete = JSI::GetNamedProperty(args[0], CB_COMPLETE);

    int32_t cmd = (int32_t)JSI::GetNumberProperty(args[0], "cmd");  
    char *data = (char *)JSI::GetStringProperty(args[0], "data");
    HILOG_ERROR(HILOG_MODULE_ACE, "cmd is: %d\n", cmd);
    HILOG_ERROR(HILOG_MODULE_ACE,"data is: %s\n", data);
    char *replyData;

    if (E53IA1Control(serv, cmd, data, &replyData))
    {
        HILOG_ERROR(HILOG_MODULE_ACE,"fail to send event\n");
        JSI::CallFunction(fail, thisVal, nullptr, 0);
        JSI::CallFunction(complete, thisVal, nullptr, 0);
        JSI::ReleaseValueList(success, fail, complete);
        return JSI::CreateUndefined();
    }

    JSIValue result = JSI::CreateObject();

    JSI::SetStringProperty(result, "e53_ia1", replyData);
    JSIValue argv[ARGC_ONE] = {result};
    JSI::CallFunction(success, thisVal, argv, ARGC_ONE);
    JSI::CallFunction(complete, thisVal, nullptr, 0);
    JSI::ReleaseValueList(success, fail, complete, result);

    HdfIoServiceRecycle(serv);

    return JSI::CreateUndefined();
}

JSIValue AppModule::GetInfo(const JSIValue thisVal, const JSIValue *args, uint8_t argsNum)
{
    JSIValue result = JSI::CreateUndefined();

    cJSON *manifest = ReadManifest();
    if (manifest == nullptr) {
        HILOG_ERROR(HILOG_MODULE_ACE, "Fail to get the content of manifest.");
        return result;
    }

    cJSON *appName = cJSON_GetObjectItem(manifest, KEY_APP_NAME);
    cJSON *versionName = cJSON_GetObjectItem(manifest, KEY_VERSION_NAME);
    cJSON *versionCode = cJSON_GetObjectItem(manifest, KEY_VERSION_CODE);

    result = JSI::CreateObject();
    if (appName != nullptr && appName->type == cJSON_String) {
        JSI::SetStringProperty(result, KEY_APP_NAME, appName->valuestring);
    }
    if (versionName != nullptr && versionName->type == cJSON_String) {
        JSI::SetStringProperty(result, KEY_VERSION_NAME, versionName->valuestring);
    }
    if (versionCode != nullptr && versionCode->type == cJSON_Number) {
        JSI::SetNumberProperty(result, KEY_VERSION_CODE, versionCode->valuedouble);
    }
    cJSON_Delete(manifest);
    manifest = nullptr;
    return result;
}

cJSON *AppModule::ReadManifest()
{
    char *appPath = const_cast<char *>(JsAppContext::GetInstance()->GetCurrentAbilityPath());
    if ((appPath == nullptr) || !strlen(appPath)) {
        HILOG_ERROR(HILOG_MODULE_ACE, "Fail to get app information because the app path is null or empty.");
        return nullptr;
    }

    char *manifestPath = RelocateJSSourceFilePath(appPath, FILE_MANIFEST);
    if (manifestPath == nullptr) {
        HILOG_ERROR(HILOG_MODULE_ACE, "Fail to get app information because the manifest.json path is null.");
        return nullptr;
    }
    if (!strlen(manifestPath)) {
        HILOG_ERROR(HILOG_MODULE_ACE, "Fail to get app information because the manifest.json path is empty.");
        ace_free(manifestPath);
        manifestPath = nullptr;
        return nullptr;
    }

    uint32_t fileSize = 0;
    char *manifestContent = ReadFile(manifestPath, fileSize, false);

    ace_free(manifestPath);
    manifestPath = nullptr;

    if (manifestContent == nullptr) {
        HILOG_ERROR(HILOG_MODULE_ACE, "Fail to get app information because the manifest.json context is null.");
        return nullptr;
    }
    if (fileSize == 0) {
        HILOG_ERROR(HILOG_MODULE_ACE, "Fail to get app information because the manifest.json context is empty.");
        ace_free(manifestContent);
        manifestContent = nullptr;
        return nullptr;
    }

    cJSON *manifest = cJSON_Parse(manifestContent);

    ace_free(manifestContent);
    manifestContent = nullptr;

    return manifest;
}

JSIValue AppModule::Terminate(const JSIValue thisVal, const JSIValue *args, uint8_t argsNum)
{
    UNUSED(thisVal);
    UNUSED(args);
    UNUSED(argsNum);
    JsAppContext::GetInstance()->TerminateAbility();
    return JSI::CreateUndefined();
}

#ifdef FEATURE_SCREEN_ON_VISIBLE
JSIValue AppModule::ScreenOnVisible(const JSIValue thisVal, const JSIValue *args, uint8_t argsNum)
{
    JSIValue undefValue = JSI::CreateUndefined();
    if ((args == nullptr) || (argsNum == 0) || (JSI::ValueIsUndefined(args[0]))) {
        ProductAdapter::SetScreenOnVisible(false);
        return undefValue;
    }

    JSIValue visibleInput = JSI::GetNamedProperty(args[0], SCREEN_ON_VISIBLE_KEY);
    bool visible = false;
    if (!JSI::ValueIsUndefined(visibleInput)) {
        visible = JSI::ValueIsBoolean(visibleInput) ? JSI::ValueToBoolean(visibleInput) : false;
    }

    bool ret = ProductAdapter::SetScreenOnVisible(visible);
    if (ret) {
        OnSetActionSuccess(thisVal, args);
    } else {
        HILOG_ERROR(HILOG_MODULE_ACE, "Fail to set screen visible property");
        OnSetActionFail(thisVal, args);
    }
    OnSetActionComplete(thisVal, args);
    JSI::ReleaseValueList(visibleInput);
    return undefValue;
}

void AppModule::OnSetActionSuccess(const JSIValue thisVal, const JSIValue *args)
{
    JSIValue callback = JSI::GetNamedProperty(args[0], CB_SUCCESS);
    if ((!JSI::ValueIsUndefined(callback)) && JSI::ValueIsFunction(callback)) {
        AsyncCallFunction(thisVal, callback, nullptr);
    } else {
        JSI::ReleaseValue(callback);
    }
}

void AppModule::OnSetActionFail(const JSIValue thisVal, const JSIValue *args)
{
    JSIValue fail = JSI::GetNamedProperty(args[0], CB_FAIL);
    if ((!JSI::ValueIsUndefined(fail)) && JSI::ValueIsFunction(fail)) {
        JSIValue result = JSI::CreateObject();
        JSI::SetStringProperty(result, SCREEN_ON_VISIBLE_DATA, SCREEN_ON_VISIBLE_INVALID_PARAMETER);
        JSI::SetNumberProperty(result, SCREEN_ON_VISIBLE_CODE, SCREEN_ON_VISIBLE_ERR);
        AsyncCallFunction(thisVal, fail, result);
    } else {
        JSI::ReleaseValue(fail);
    }
}

void AppModule::OnSetActionComplete(const JSIValue thisVal, const JSIValue *args)
{
    JSIValue callback = JSI::GetNamedProperty(args[0], CB_COMPLETE);
    if ((!JSI::ValueIsUndefined(callback)) && JSI::ValueIsFunction(callback)) {
        AsyncCallFunction(thisVal, callback, nullptr);
    } else {
        JSI::ReleaseValue(callback);
    }
}

void AppModule::AsyncCallFunction(const JSIValue thisVal, const JSIValue callback, const JSIValue result)
{
    AsyncParams *params = new AsyncParams();
    if (params == nullptr) {
        JSI::ReleaseValueList(result, callback);
        return;
    }

    params->result = result;
    params->callback = callback;
    params->context = JSI::AcquireValue(thisVal);
    if (!JsAsyncWork::DispatchAsyncWork(Execute, static_cast<void *>(params))) {
        JSI::ReleaseValueList(params->result, params->callback, params->context);
        delete params;
        params = nullptr;
    }
}

void AppModule::Execute(void *data)
{
    AsyncParams *params = static_cast<AsyncParams *>(data);
    if (params == nullptr) {
        return;
    }

    JSIValue callback = params->callback;
    JSIValue result = params->result;
    JSIValue context = params->context;
    if (result == nullptr) {
        // complete callback and success callback do not need parameter
        JSI::CallFunction(callback, context, nullptr, 0);
        JSI::ReleaseValueList(callback, context);
    } else {
        // fail callback need error reason and error code
        JSIValue argv[ARGC_TWO] = {
            JSI::GetNamedProperty(result, SCREEN_ON_VISIBLE_DATA),
            JSI::GetNamedProperty(result, SCREEN_ON_VISIBLE_CODE)
        };
        JSI::CallFunction(callback, context, argv, ARGC_TWO);
        JSI::ReleaseValueList(callback, result, context);
    }

    delete params;
    params = nullptr;
}
#endif // FEATURE_SCREEN_ON_VISIBLE
} // namespace ACELite
} // namespace OHOS
