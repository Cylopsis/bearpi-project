/*
 * Copyright (c) 2022 Nanjing Xiaoxiongpai Intelligent Technology Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <stdint.h>
#include <string.h>

#include "E53_SC1.h"
#include "E53_Common.h"

#include "hdf_device_desc.h" 
#include "hdf_log.h"         
#include "device_resource_if.h"
#include "osal_io.h"
#include "osal_mem.h"
#include "gpio_if.h"
#include "osal_time.h"


static uint8_t LightStatus;
static float lux_data;

typedef enum {
    E53_SC1_Start = 0,
    E53_SC1_Stop,
    E53_SC1_Read,
    E53_SC1_SetLight,
}E53_SC1Ctrl;

int32_t E53_SC1_DriverDispatch(struct HdfDeviceIoClient *client, int cmdCode, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int ret = -1;
    char *replay_buf;

    HDF_LOGE("E53 driver dispatch");
    if (client == NULL || client->device == NULL) {
        HDF_LOGE("E53 driver device is NULL");
        return HDF_ERR_INVALID_OBJECT;
    }
    switch (cmdCode) {
        case E53_SC1_Start:
            ret = E53_SC1Init();
            if (ret != 0) {
                HDF_LOGE("E53 SC1 Init err");
                return HDF_FAILURE;
            }
            ret = HdfSbufWriteString(reply, "E53 SC1 Init successful");
            if (ret == 0) {
                HDF_LOGE("reply failed");
                return HDF_FAILURE;
            }
            break;
        case E53_SC1_Stop:
            ret = E53_SC1DeInit();
            if (ret != 0) {
                HDF_LOGE("E53 SC1 DeInit err");
                return HDF_FAILURE;
            }
            ret = HdfSbufWriteString(reply, "E53 SC1 DeInit successful");
            if (ret == 0) {
                HDF_LOGE("reply failed");
                return HDF_FAILURE;
            }
            break;
        /* 接收到用户态发来的LED_WRITE_READ命令 */
        case E53_SC1_Read:
            ret = E53_SC1ReadData(&lux_data);
            if (ret != 0) {
                HDF_LOGE("E53 SC1 Read Data err");
                return HDF_FAILURE;
            }
            replay_buf = OsalMemAlloc(100);
            (void)memset_s(replay_buf, 100, 0, 100);
            sprintf(replay_buf, "{\"Lux\":%.2f,\"LED\":\"%s\"}", lux_data, LightStatus ? "ON" : "OFF");
            /* 把传感器数据写入reply, 可被带至用户程序 */
            if (!HdfSbufWriteString(reply, replay_buf)) {
                HDF_LOGE("replay is fail");
                return HDF_FAILURE;
            }
            OsalMemFree(replay_buf);
            break;
        case E53_SC1_SetLight:
            const char* readdata = HdfSbufReadString(data);
            if (strcmp(readdata, "ON") == 0) {
                E53_SC1LightStatusSet(ON);
                LightStatus = 1;
            } else if (strcmp(readdata, "OFF") == 0) {
                E53_SC1LightStatusSet(OFF);
                LightStatus = 0;
            } else {
                HDF_LOGE("Command wrong!");
                return HDF_FAILURE;
            }
            replay_buf = OsalMemAlloc(100);
            (void)memset_s(replay_buf, 100, 0, 100);
            sprintf(replay_buf, "{\"Lux\":%.2f,\"LED\":\"%s\"}", lux_data, LightStatus ? "ON" : "OFF");
            /* 把传感器数据写入reply, 可被带至用户程序 */
            if (!HdfSbufWriteString(reply, replay_buf)) {
                HDF_LOGE("replay is fail");
                return HDF_FAILURE;
            }
            OsalMemFree(replay_buf);
            break;

        default:
            return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}


//驱动对外提供的服务能力，将相关的服务接口绑定到HDF框架
static int32_t Hdf_E53_SC1_DriverBind(struct HdfDeviceObject *deviceObject)
{
    if (deviceObject == NULL) {
        HDF_LOGE("e53 driver bind failed!");
        return HDF_ERR_INVALID_OBJECT;
    }
    static struct IDeviceIoService e53Driver = {
        .Dispatch = E53_SC1_DriverDispatch,
    };
    deviceObject->service = (struct IDeviceIoService *)(&e53Driver);
    HDF_LOGD("E53 driver bind success");
    return HDF_SUCCESS;
}

// static struct E53_SC1Hooks hooks = {
//     .E53_SC1Init = sc1Init,
//     .E53_SC1DeInit = sc1DeInit,
//     .E53_SC1LightGpioSet = sc1GpioSet,
//     .E53_SC1_IICWriteFunc = sc1E53_IICWrite,
//     .E53_SC1_IICReadFunc = sc1E53_IICRead,
//     .E53_SC1_IICWriteReadFunc = sc1E53_IICWriteRead,
// };

static int32_t Hdf_E53_SC1_DriverInit(struct HdfDeviceObject *device)
{
    return HDF_SUCCESS;
}

// 驱动资源释放的接口
void Hdf_E53_SC1_DriverRelease(struct HdfDeviceObject *deviceObject)
{
    if (deviceObject == NULL) {
        HDF_LOGE("Led driver release failed!");
        return;
    }
    HDF_LOGD("Led driver release success");
    return;
}


// 定义驱动入口的对象，必须为HdfDriverEntry（在hdf_device_desc.h中定义）类型的全局变量
static struct HdfDriverEntry g_E53DriverEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_E53_SC1",
    .Bind = Hdf_E53_SC1_DriverBind,
    .Init = Hdf_E53_SC1_DriverInit,
    .Release = Hdf_E53_SC1_DriverRelease,
};

// 调用HDF_INIT将驱动入口注册到HDF框架中
HDF_INIT(g_E53DriverEntry);

