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


#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "cenctrl_log.h"
#include "centrl_lamp.h"
#include "sensor.h"

#define     ON            "ON"
#define     OFF           "OFF"


static lampCallBackParam g_callParam_ = { 0 };

void RegisterLampCallback(lampCallBackParam* callParam)
{
    g_callParam_.On = callParam->On;
    g_callParam_.Off = callParam->Off;
}
void UnRegisterLampCallback()
{
    g_callParam_.On = NULL;
    g_callParam_.Off = NULL;
}


static int LampGetNameHandle(char *buff, int size)
{
    const char *g_DeviceName[] = {
        "LRLIGHT",        // living room downlight
        "DRLIGHT",        // dining room downligth
        "LANTERN",        // lantern
        "LRLIGHT1",        // living room downlight
        "DRLIGHT1",        // dining room downligth
        "AILIGHT"        // aisle downlight
    };

    if (buff == NULL || size <= 0) {
        return -1;
    }

    strcpy(buff, g_DeviceName[0]);

    return 0;
}


static int LampSocketEventHandle(SOCKET_EVENT event, void *value)
{
    struct HdfIoService *serv = NULL;
    switch (event) {
        case SOCKET_CONNECTTED:
            LOG(CENCTRL_DEBUG, "### socket connectted ####\n");
            break;
        case SOCKET_DISCONNECT:
            LOG(CENCTRL_DEBUG, "### socket disconnect ####\n");
            break;
        case SOCKET_SET_CMD:

            if (strcmp(value, "LED:ON") == 0) {
                LOG(CENCTRL_DEBUG, "### cmd on ####\n");
                g_callParam_.On("Led", 1);
                DeiceCtrl("Led", "ON");                
            } else if (strcmp(value, "LED:OFF") == 0) {
                LOG(CENCTRL_DEBUG, "### cmd off ####\n");
                 g_callParam_.Off("Led", 0);
                DeiceCtrl("Led", "OFF");
            }
            if (strcmp(value, "MOTOR:ON") == 0) {
                g_callParam_.On("Motor", 1);
                LOG(CENCTRL_DEBUG, "### cmd on ####\n");
                DeiceCtrl("Motor", "ON");
            } else if (strcmp(value, "MOTOR:OFF") == 0) {
                g_callParam_.Off("Motor", 0);
                LOG(CENCTRL_DEBUG, "### cmd off ####\n");
                DeiceCtrl("Motor", "OFF");
            }
            break;
        default:
            LOG(CENCTRL_ERROR, "no such socket event(%d)! \n", event);
            break;
    }

    return 0;
}

void StartLam(void)
{
    SocketCallback mCallback;
    mCallback.socketGetDeviceName = LampGetNameHandle;
    mCallback.socketEvent = LampSocketEventHandle;
    SocketClientStart(&mCallback);
}
