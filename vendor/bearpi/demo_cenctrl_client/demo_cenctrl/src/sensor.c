/* 
 * Copyright (c) 2022 Nanjing Xiaoxiongpai Intelligent Technology CO., LIMITED.
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


#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include "cenctrl_log.h"
#include "sensor.h"
#include "hdf_sbuf.h"
#include "hdf_io_service_if.h"

#define     ON            "ON"
#define     OFF           "OFF"
extern int g_motor_status;
extern int g_led_status;

#define E53_IA1_SERVICE "hdf_e53_ia1"

static int SendEvent(struct HdfIoService *serv, const char* cmd, const char* buf, char* replyData)
{
    int ret = 0;
    int cmdID = 0;
    struct HdfSBuf *data = HdfSBufObtainDefaultSize();
    if (data == NULL) {
        printf("fail to obtain sbuf data!\r\n");
        return 1;
    }

    struct HdfSBuf *reply = HdfSBufObtainDefaultSize();
    if (reply == NULL) {
        printf("fail to obtain sbuf reply!\r\n");
        ret = HDF_DEV_ERR_NO_MEMORY;
        goto out;
    }
    /* 写入数据 */
    if (!HdfSbufWriteString(data, buf)) {
        printf("fail to write sbuf!\r\n");
        ret = HDF_FAILURE;
        goto out;
    }

    if (strcmp(cmd, "Init") == 0) {
        cmdID = 0;
    } else if (strcmp(cmd, "Read") == 0) {
        cmdID = 2;
    } else if (strcmp(cmd, "Motor") == 0) {
        cmdID = 3;
    } else if (strcmp(cmd, "Led") == 0) {
        cmdID = 4;
    } else {
        goto out;
    }

    /* 通过Dispatch发送到驱动 */
    ret = serv->dispatcher->Dispatch(&serv->object, cmdID, data, reply);

    if (ret != HDF_SUCCESS) {
        printf("fail to send service call!\r\n");
        goto out;
    }
    if (replyData != NULL) {
        // char replyDatabuf[1000];
        sprintf(replyData, "%s", HdfSbufReadString(reply));
        /* 读取驱动的回复数据 */
        if (replyData[0] == NULL) {
            printf("fail to get service call reply!\r\n");
            ret = HDF_ERR_INVALID_OBJECT;
            goto out;
        }
    }


out:

    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
    return ret;
}
int InitSensor(void)
{
    int ret = 0;
    struct HdfIoService *serv = NULL;

    serv = HdfIoServiceBind(E53_IA1_SERVICE);
    if (serv == NULL) {
        printf("fail to get service %s!\r\n", E53_IA1_SERVICE);
        return HDF_FAILURE;
    }

    ret = SendEvent(serv, "Init", NULL, NULL);

    HdfIoServiceRecycle(serv);

    return ret;
}

int ReadSensorData(char* replyData)
{
    int ret = 0;
    struct HdfIoService *serv = NULL;

    serv = HdfIoServiceBind(E53_IA1_SERVICE);
    if (serv == NULL) {
        printf("fail to get service %s!\r\n", E53_IA1_SERVICE);
        return HDF_FAILURE;
    }

    ret = SendEvent(serv, "Read", NULL, replyData);

    HdfIoServiceRecycle(serv);

    return ret;
}

int DeiceCtrl(const char* cmd, const char* buf)
{
    int ret = 0;
    struct HdfIoService *serv = NULL;
    serv = HdfIoServiceBind(E53_IA1_SERVICE);
    if (serv == NULL) {
        printf("fail to get service %s!\r\n", E53_IA1_SERVICE);
        return HDF_FAILURE;
    }
    if (strcmp(cmd, "Led") == 0) {
        if (strcmp(buf, "ON") == 0) {
            g_led_status = 1;
        } else  if (strcmp(buf, "OFF") == 0) {
            g_led_status = 0;
        }

    } else if (strcmp(cmd, "Motor") == 0) {
        if (strcmp(buf, "ON") == 0) {
            g_motor_status = 1;
        } else  if (strcmp(buf, "OFF") == 0) {
            g_motor_status = 0;
        }
    }
    ret = SendEvent(serv, cmd, buf, NULL);

    HdfIoServiceRecycle(serv);

    return ret;
}


