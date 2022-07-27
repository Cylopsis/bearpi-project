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
#include "sensor_main.h"
#include "centrl_lamp.h"
#include "socket_client_1.h"
#include "sensor.h"

static displayCallBackParam g_callParam_ = {0};


void InterceptString (char *descr, char *str, char *left_str, char *right_str)
{
	char *l_str = strstr(str, left_str);
	int r_str = strcspn(l_str, right_str);
	int l_len = strlen(left_str);
    int str_len = r_str - l_len;
    
    strncpy(descr, l_str + l_len, str_len);    
    descr[str_len + 1] = '\0';

}

void RegisterSensorDisplsyCallback(displayCallBackParam* callParam)
{
    g_callParam_.SensorDisplay = callParam->SensorDisplay;
}
void UnRegisterSensorDisplsyCallback()
{
    g_callParam_.SensorDisplay = NULL;
}


static pthread_t g_discThreadID = 0;

static unsigned char g_threadRunning = 0;
int g_motor_status;
int g_led_status;
static void SensorProcess(void *arg)
{
    pthread_detach(pthread_self());
    struct HdfIoService *serv = NULL;
    char replyDatabuf[1000];
    char sendDatabuf[1000];
    char *Tem,*Lux,*Hum;
    (void)arg;
    InitSensor();
    while (g_threadRunning) {

        ReadSensorData(replyDatabuf);

        Tem = (char *)malloc(10);
        Lux = (char *)malloc(10);
        Hum = (char *)malloc(10);
        InterceptString(Tem, replyDatabuf, "\"Temp\":", ",");
        InterceptString(Lux, replyDatabuf, "\"Lux\":", ",");
        InterceptString(Hum, replyDatabuf, "\"Hum\":", ",");


        g_callParam_.SensorDisplay(atoi(Tem),
                                    atoi(Lux),
                                    atoi(Hum));
        memset(sendDatabuf,0,sizeof(sendDatabuf));
        if (sprintf_s(sendDatabuf, 1200, "HMTPTem:%d\nLux:%d\nHum:%d\nLED:%d\nMOTOR:%d\n", atoi(Tem), atoi(Lux), atoi(Hum),g_led_status,g_motor_status) < 0) {
             LOG(CENCTRL_DEBUG, "sprintf_s fail\n");
        }
        SocketSendData(sendDatabuf);
        sleep(2);
    }
}

void StartSensor(void)
{
    LOG(CENCTRL_DEBUG, "create the StartSensor\n");
    g_threadRunning = 1;
    int ret = pthread_create(&g_discThreadID, NULL, SensorProcess, NULL);
    if (ret != 0) {
        LOG(CENCTRL_ERROR, "SensorProcess [ERROR]thread error %s\n", strerror(errno));
        return;
    }
}
