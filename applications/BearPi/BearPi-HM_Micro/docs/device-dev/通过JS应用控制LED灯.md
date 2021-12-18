# 编写通过JS应用控制LED灯


## 一、JS API接口开发

1. 添加控制LED灯的JS API接口

    修改`foundation\ace\ace_engine_lite\frameworks\src\core\modules\app_module.h`，加入toggleLed JS API,（"##start##"和"##end##"仅用来标识位置，添加完配置后删除这两行）

    ```
    public:
    ACE_DISALLOW_COPY_AND_MOVE(AppModule);
    AppModule() = default;
    ~AppModule() = default;
    static JSIValue GetInfo(const JSIValue thisVal, const JSIValue *args, uint8_t argsNum);
    static JSIValue Terminate(const JSIValue thisVal, const JSIValue *args, uint8_t argsNum);
    ##start##
    static JSIValue ToggleLed(const JSIValue thisVal, const JSIValue* args, uint8_t argsNum);
    ##end##
    #ifdef FEATURE_SCREEN_ON_VISIBLE
        static JSIValue ScreenOnVisible(const JSIValue thisVal, const JSIValue *args, uint8_t argsNum);
    #endif
    ```

    ```
    void InitAppModule(JSIValue exports)
    {
        JSI::SetModuleAPI(exports, "getInfo", AppModule::GetInfo);
        JSI::SetModuleAPI(exports, "terminate", AppModule::Terminate);
    ##start##
        JSI::SetModuleAPI(exports, "ledcontrol", AppModule::ToggleLed);
    ##end##
    #ifdef FEATURE_SCREEN_ON_VISIBLE
        JSI::SetModuleAPI(exports, "screenOnVisible", AppModule::ScreenOnVisible);
    #endif
    }
    ```

2. 编写控制LED灯c++ 业务代码

    在`foundation\ace\ace_engine_lite\frameworks\src\core\modules\app_module.cpp`中加入控制LED灯c++ 业务代码（"##start##"和"##end##"仅用来标识位置，添加完配置后删除这两行）。

    ```
    #include "app_module.h"
    #include "ace_log.h"
    #include "js_app_context.h"
    #ifdef FEATURE_SCREEN_ON_VISIBLE
    #include "js_async_work.h"
    #include "product_adapter.h"
    #endif

    ##start##
    #include "hdf_sbuf.h"
    #include "hdf_io_service_if.h"

    #define LED_WRITE_READ 1
    #define LED_SERVICE "hdf_led"
    ##end##

    namespace OHOS {
    namespace ACELite {
    const char * const AppModule::FILE_MANIFEST = "manifest.json";
    const char * const AppModule::KEY_APP_NAME = "appName";
    const char * const AppModule::KEY_VERSION_NAME = "versionName";
    const char * const AppModule::KEY_VERSION_CODE = "versionCode";

    ```

    ```
    struct AsyncParams : public MemoryHeap {
        ACE_DISALLOW_COPY_AND_MOVE(AsyncParams);
        AsyncParams() : result(nullptr), callback(nullptr), context(nullptr) {}

        JSIValue result;
        JSIValue callback;
        JSIValue context;
    };
    #endif

    ##start##

    static int OnDevEventReceived(void *priv, uint32_t id, struct HdfSBuf *data)
    {
        uint32_t value;
        HdfSbufReadUint32(data, &value);
        HILOG_ERROR(HILOG_MODULE_ACE,"%s: dev event received: %u %u\n", (char *)priv, id, value);

        return HDF_SUCCESS;
    }

    static int GpioWriteRead(struct HdfIoService *serv, int32_t eventData, int32_t *val)
    {
        int ret = HDF_FAILURE;
        struct HdfSBuf *data = HdfSBufObtainDefaultSize();
        struct HdfSBuf *reply = HdfSBufObtainDefaultSize();

        if (data == NULL || reply == NULL) {
            HILOG_ERROR(HILOG_MODULE_ACE,"fail to obtain sbuf data\n");
            return ret;
        }

        if (!HdfSbufWriteUint8(data, (uint8_t)eventData))
        {
            HILOG_ERROR(HILOG_MODULE_ACE,"fail to write sbuf\n");
            HdfSBufRecycle(data);
            HdfSBufRecycle(reply);
            return ret;
        }

        ret = serv->dispatcher->Dispatch(&serv->object, LED_WRITE_READ, data, reply);
        if (ret != HDF_SUCCESS)
        {
            HILOG_ERROR(HILOG_MODULE_ACE,"fail to send service call\n");
            HdfSBufRecycle(data);
            HdfSBufRecycle(reply);
            return ret;
        }
        if (!HdfSbufReadInt32(reply, val))
        {
            HILOG_ERROR(HILOG_MODULE_ACE,"fail to get service call reply\n");
            ret = HDF_ERR_INVALID_OBJECT;
            HdfSBufRecycle(data);
            HdfSBufRecycle(reply);
            return ret;
        }
        HILOG_ERROR(HILOG_MODULE_ACE,"Get reply is: %d\n", val);

        HdfSBufRecycle(data);
        HdfSBufRecycle(reply);
        return ret;
    }

    JSIValue AppModule::ToggleLed(const JSIValue thisVal, const JSIValue *args, uint8_t argsNum)
    {
        HILOG_ERROR(HILOG_MODULE_ACE, "led button pressed.");

        struct HdfIoService *serv = HdfIoServiceBind(LED_SERVICE);
        if (serv == NULL)
        {
            HILOG_ERROR(HILOG_MODULE_ACE,"fail to get service2 %s\n", LED_SERVICE);
            return JSI::CreateUndefined();
        }
    
        if ((args == nullptr) || (argsNum == 0) || (JSI::ValueIsUndefined(args[0]))) {
            return JSI::CreateUndefined();
        }

        JSIValue success = JSI::GetNamedProperty(args[0], CB_SUCCESS);
        JSIValue fail = JSI::GetNamedProperty(args[0], CB_FAIL);
        JSIValue complete = JSI::GetNamedProperty(args[0], CB_COMPLETE);

        int32_t num = (int32_t)JSI::GetNumberProperty(args[0], "code");

        int32_t replyData = 0;

        if (GpioWriteRead(serv, num, &replyData))
        {
            HILOG_ERROR(HILOG_MODULE_ACE,"fail to send event\n");
            JSI::CallFunction(fail, thisVal, nullptr, 0);
            JSI::CallFunction(complete, thisVal, nullptr, 0);
            JSI::ReleaseValueList(success, fail, complete);
            return JSI::CreateUndefined();
        }

        JSIValue result = JSI::CreateObject();
        JSI::SetNumberProperty(result, "led_status", replyData);

        JSIValue argv[ARGC_ONE] = {result};
        JSI::CallFunction(success, thisVal, argv, ARGC_ONE);
        JSI::CallFunction(complete, thisVal, nullptr, 0);
        JSI::ReleaseValueList(success, fail, complete, result);

        HdfIoServiceRecycle(serv);

        return JSI::CreateUndefined();
    }

    ##end##

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
    ```
3. 配置HDF头文件路径

    在`foundation\ace\ace_engine_lite\ace_lite.gni`中添加HDF头文件路径。

    ```
    ace_lite_include_dirs += [
        "//drivers/framework/ability/sbuf/include",
        "//drivers/framework/include/core",
        "//drivers/framework/include/utils",
        "//drivers/adapter/uhdf/posix/include",
    ]
    ```
4. 添加编译依赖

    修改`foundation\ace\ace_engine_lite\frameworks\BUILD.gn`，在`public_deps`中添加以下代码
    ```
    "//drivers/adapter/uhdf/manager:hdf_core",
    ```

    修改`foundation\ace\ace_engine_lite\test\ace_test_config.gni`，在`extra_deps`中添加以下代码
    ```
    "//drivers/adapter/uhdf/manager:hdf_core",
    ```

## 二、运行结果

示例代码[编译](如何编译系统.md)、[烧录](如何烧录固件并启动.md)后，参考[如何在开发板上安装HAP应用](如何在开发板上安装HAP应用.md)教程安装applications/BearPi/BearPi-HM_Micro/tools/hap_tools/hap_example目录下的LED.hap应用进行测试




