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
#include "js_async_work.h"
#include "js_app_context.h"
#ifdef FEATURE_SCREEN_ON_VISIBLE
#include "product_adapter.h"
#endif

#include "hdf_sbuf.h"
#include "hdf_io_service_if.h"

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <unistd.h>

#define E53_IS1_SERVICE "hdf_e53_is1"
#define E53_SF1_SERVICE "hdf_e53_sf1"
#define E53_SC2_SERVICE "hdf_e53_sc2"
#define E53_SC1_SERVICE "hdf_e53_sc1"
#define E53_IA1_SERVICE "hdf_e53_ia1"

#define TEMP_CONTROL_HOST "127.0.0.1"
#define TEMP_CONTROL_PORT 5000
#define TEMP_CONTROL_TIMEOUT_MS 500
#define TEMP_CONTROL_CMD_MAX 128
#define TEMP_CONTROL_RESPONSE_MAX 1024

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

struct TempControlParams : public MemoryHeap {
    ACE_DISALLOW_COPY_AND_MOVE(TempControlParams);
    TempControlParams() : context(nullptr), success(nullptr), fail(nullptr), complete(nullptr), ok(false)
    {
        command[0] = '\0';
        response[0] = '\0';
        error[0] = '\0';
    }

    JSIValue context;
    JSIValue success;
    JSIValue fail;
    JSIValue complete;
    char command[TEMP_CONTROL_CMD_MAX];
    char response[TEMP_CONTROL_RESPONSE_MAX];
    char error[128];
    bool ok;
};

static bool IsNumericToken(const char *value)
{
    if (value == nullptr || value[0] == '\0') {
        return false;
    }
    char *end = nullptr;
    errno = 0;
    (void)strtod(value, &end);
    return (errno == 0 && end != value && end != nullptr && *end == '\0');
}

static bool IsAllowedTuneCommand(int argc, char **argv)
{
    if (argc == 3) {
        if (!IsNumericToken(argv[2])) {
            return false;
        }
        return strcmp(argv[1], "target") == 0 || strcmp(argv[1], "hys") == 0 ||
               strcmp(argv[1], "warmbias") == 0 || strcmp(argv[1], "heatbias") == 0;
    }
    if (argc != 4 || !IsNumericToken(argv[3])) {
        return false;
    }
    if (strcmp(argv[1], "box") == 0 || strcmp(argv[1], "heat") == 0) {
        return strcmp(argv[2], "kp") == 0 || strcmp(argv[2], "ki") == 0 || strcmp(argv[2], "kd") == 0;
    }
    if (strcmp(argv[1], "cool") == 0) {
        return strcmp(argv[2], "kp") == 0 || strcmp(argv[2], "ki") == 0;
    }
    return false;
}

static bool IsAllowedTempControlCommand(const char *command)
{
    if (command == nullptr || command[0] == '\0' || strlen(command) >= TEMP_CONTROL_CMD_MAX - 3) {
        return false;
    }
    char copy[TEMP_CONTROL_CMD_MAX];
    if (snprintf(copy, sizeof(copy), "%s", command) < 0) {
        return false;
    }

    char *argv[5] = { nullptr };
    int argc = 0;
    char *saveptr = nullptr;
    char *token = strtok_r(copy, " ", &saveptr);
    while (token != nullptr && argc < 5) {
        argv[argc++] = token;
        token = strtok_r(nullptr, " ", &saveptr);
    }
    if (token != nullptr || argc == 0) {
        return false;
    }
    if (argc == 1) {
        return strcmp(argv[0], "get_status") == 0;
    }
    if (strcmp(argv[0], "tune") == 0) {
        return IsAllowedTuneCommand(argc, argv);
    }
    return false;
}

// Persistent connection to the temp control backend. The socket is opened once and
// reused across commands; it is closed (and lazily reopened) only when an exchange
// fails. Access is serialized by g_tempControlLock so concurrent JS calls cannot
// interleave their request/response on the shared socket.
static int g_tempControlSock = -1;
static pthread_mutex_t g_tempControlLock = PTHREAD_MUTEX_INITIALIZER;

static void CloseTempControlSock()
{
    if (g_tempControlSock >= 0) {
        close(g_tempControlSock);
        g_tempControlSock = -1;
    }
}

static int EnsureTempControlConnected(char *error, size_t errorLen)
{
    if (g_tempControlSock >= 0) {
        return 0;
    }

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        (void)snprintf(error, errorLen, "socket failed: %d", errno);
        return -1;
    }

    struct timeval timeout = {
        TEMP_CONTROL_TIMEOUT_MS / 1000,
        (TEMP_CONTROL_TIMEOUT_MS % 1000) * 1000
    };
    (void)setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    (void)setsockopt(sock, SOL_SOCKET, SO_SNDTIMEO, &timeout, sizeof(timeout));

    struct sockaddr_in addr;
    (void)memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(TEMP_CONTROL_PORT);
    if (inet_pton(AF_INET, TEMP_CONTROL_HOST, &addr.sin_addr) <= 0) {
        (void)snprintf(error, errorLen, "invalid temp control host");
        close(sock);
        return -1;
    }
    if (connect(sock, reinterpret_cast<struct sockaddr *>(&addr), sizeof(addr)) < 0) {
        (void)snprintf(error, errorLen, "connect failed: %d", errno);
        close(sock);
        return -1;
    }

    g_tempControlSock = sock;
    return 0;
}

// Single attempt over the persistent socket. On any failure the socket is closed so
// the next attempt reconnects; on success the socket is kept open for reuse.
static int TempControlExchangeOnce(const char *command, char *response, size_t responseLen,
    char *error, size_t errorLen)
{
    response[0] = '\0';

    if (EnsureTempControlConnected(error, errorLen) != 0) {
        return -1;
    }

    char line[TEMP_CONTROL_CMD_MAX + 3];
    int lineLen = snprintf(line, sizeof(line), "%s\r\n", command);
    if (lineLen <= 0 || lineLen >= static_cast<int>(sizeof(line))) {
        (void)snprintf(error, errorLen, "command too long");
        CloseTempControlSock();
        return -1;
    }

    int sent = 0;
    while (sent < lineLen) {
        int ret = send(g_tempControlSock, line + sent, lineLen - sent, 0);
        if (ret <= 0) {
            (void)snprintf(error, errorLen, "send failed: %d", errno);
            CloseTempControlSock();
            return -1;
        }
        sent += ret;
    }

    size_t used = 0;
    while (used + 1 < responseLen) {
        char ch;
        int ret = recv(g_tempControlSock, &ch, 1, 0);
        if (ret <= 0) {
            (void)snprintf(error, errorLen, "recv failed: %d", errno);
            CloseTempControlSock();
            return -1;
        }
        if (ch == '\n') {
            break;
        }
        if (ch != '\r') {
            response[used++] = ch;
        }
    }
    response[used] = '\0';

    if (used == 0) {
        (void)snprintf(error, errorLen, "empty response");
        CloseTempControlSock();
        return -1;
    }
    return 0;
}

// Reuses a single persistent connection to the temp control backend. If the first
// attempt fails (e.g. the backend restarted or closed an idle connection), the socket
// is dropped and a single reconnect+retry is performed before giving up.
static int TempControlExchange(const char *command, char *response, size_t responseLen, char *error, size_t errorLen)
{
    if (response == nullptr || responseLen == 0 || error == nullptr || errorLen == 0) {
        return -1;
    }
    response[0] = '\0';
    error[0] = '\0';

    pthread_mutex_lock(&g_tempControlLock);
    int ret = TempControlExchangeOnce(command, response, responseLen, error, errorLen);
    if (ret != 0) {
        ret = TempControlExchangeOnce(command, response, responseLen, error, errorLen);
    }
    pthread_mutex_unlock(&g_tempControlLock);
    return ret;
}

static void ReleaseTempControlParams(TempControlParams *params)
{
    if (params == nullptr) {
        return;
    }
    JSI::ReleaseValueList(params->context, params->success, params->fail, params->complete);
    delete params;
}

static void CompleteTempControl(void *data)
{
    TempControlParams *params = static_cast<TempControlParams *>(data);
    if (params == nullptr) {
        return;
    }

    JSIValue result = JSI::CreateObject();
    if (params->ok) {
        JSI::SetStringProperty(result, "response", params->response);
        if (JSI::ValueIsFunction(params->success)) {
            JSIValue argv[ARGC_ONE] = { result };
            JSI::CallFunction(params->success, params->context, argv, ARGC_ONE);
        }
    } else {
        JSI::SetStringProperty(result, "message", params->error);
        if (JSI::ValueIsFunction(params->fail)) {
            JSIValue argv[ARGC_ONE] = { result };
            JSI::CallFunction(params->fail, params->context, argv, ARGC_ONE);
        }
    }
    if (JSI::ValueIsFunction(params->complete)) {
        JSI::CallFunction(params->complete, params->context, nullptr, 0);
    }
    JSI::ReleaseValue(result);
    ReleaseTempControlParams(params);
}

static void *TempControlThreadEntry(void *data)
{
    TempControlParams *params = static_cast<TempControlParams *>(data);
    if (params == nullptr) {
        return nullptr;
    }
    params->ok = (TempControlExchange(params->command, params->response, sizeof(params->response),
        params->error, sizeof(params->error)) == 0);

    if (!JsAsyncWork::DispatchAsyncWork(CompleteTempControl, static_cast<void *>(params))) {
        HILOG_ERROR(HILOG_MODULE_ACE, "TempControl: failed to dispatch JS callback");
    }
    return nullptr;
}

static JSIValue CallTempControlFail(const JSIValue thisVal, const JSIValue *args, const char *message)
{
    if ((args == nullptr) || JSI::ValueIsUndefined(args[0])) {
        return JSI::CreateUndefined();
    }
    JSIValue fail = JSI::GetNamedProperty(args[0], CB_FAIL);
    JSIValue complete = JSI::GetNamedProperty(args[0], CB_COMPLETE);
    JSIValue result = JSI::CreateObject();
    JSI::SetStringProperty(result, "message", message);
    if (JSI::ValueIsFunction(fail)) {
        JSIValue argv[ARGC_ONE] = { result };
        JSI::CallFunction(fail, thisVal, argv, ARGC_ONE);
    }
    if (JSI::ValueIsFunction(complete)) {
        JSI::CallFunction(complete, thisVal, nullptr, 0);
    }
    JSI::ReleaseValueList(fail, complete, result);
    return JSI::CreateUndefined();
}

JSIValue AppModule::TempControl(const JSIValue thisVal, const JSIValue *args, uint8_t argsNum)
{
    if ((args == nullptr) || (argsNum == 0) || JSI::ValueIsUndefined(args[0])) {
        return JSI::CreateUndefined();
    }

    char *command = JSI::GetStringProperty(args[0], "command");
    if (!IsAllowedTempControlCommand(command)) {
        JSI::ReleaseString(command);
        return CallTempControlFail(thisVal, args, "invalid temp control command");
    }

    TempControlParams *params = new TempControlParams();
    if (params == nullptr) {
        JSI::ReleaseString(command);
        return CallTempControlFail(thisVal, args, "out of memory");
    }
    (void)snprintf(params->command, sizeof(params->command), "%s", command);
    JSI::ReleaseString(command);

    params->context = JSI::AcquireValue(thisVal);
    params->success = JSI::GetNamedProperty(args[0], CB_SUCCESS);
    params->fail = JSI::GetNamedProperty(args[0], CB_FAIL);
    params->complete = JSI::GetNamedProperty(args[0], CB_COMPLETE);

    pthread_t threadId;
    pthread_attr_t attr;
    if (pthread_attr_init(&attr) != 0) {
        ReleaseTempControlParams(params);
        return CallTempControlFail(thisVal, args, "failed to create worker");
    }
    (void)pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    int ret = pthread_create(&threadId, &attr, TempControlThreadEntry, static_cast<void *>(params));
    (void)pthread_attr_destroy(&attr);
    if (ret != 0) {
        ReleaseTempControlParams(params);
        return CallTempControlFail(thisVal, args, "failed to create worker");
    }

    return JSI::CreateUndefined();
}

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
