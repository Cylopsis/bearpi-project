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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <securec.h>

#include "socket_service.h"
#include "cenctrl_log.h"

int g_connAmount = 0; // current connection count

int g_onClick_status;
int tmp;
#define DEFAULT_PORT  8989
#define MAX_LIGHT_NUM 6

stContrlParam g_contrl[MAX_CONNECT] = { 0 };
static stCallBackParam g_callParam_ = { 0 };
static pthread_t g_threadServerId = (pthread_t)0;
static int g_threadFlag = 1;
const char* g_lightName[MAX_LIGHT_NUM] = { "LRLIGHT", "DRLIGHT", "LANTERN",
                                          "LRLIGHT1", "DRLIGHT1", "AILIGHT" };

static void HeartTick(fd_set* fdset)
{
    LOG(CENCTRL_DEBUG, "timeout to HeartTick \n");
    for (int i = 0; i < MAX_CONNECT; i++) {
        if (g_contrl[i].service_id != 0) {
            char bufr[SOCKET_BUF_SIZE] = { 0 };
            write(g_contrl[i].service_id, "LIVE", strlen("LIVE"));
            int ret = read(g_contrl[i].service_id, bufr, sizeof(bufr));
            LOG(CENCTRL_DEBUG, "heart tick recv: %s, size = %d\n ", bufr, ret);
            if (ret <= 0) {
                close(g_contrl[i].service_id);
                FD_CLR(g_contrl[i].service_id, fdset);
                g_contrl[i].enable = SOCKET_DISABLE;
                g_contrl[i].service_id = 0;
                g_contrl[i].countTick = 0;
                g_connAmount--;
                LOG(CENCTRL_DEBUG, "jump client[%d] close\n", i);
            }
        }
    }
}

static void LightOnOff(stContrlParam* ctrl, const char* recvbuf)
{
    if (ctrl == NULL) {
        return;
    }
    if (strncmp(recvbuf, "1", strlen("1")) == 0) {
        // light on
        ctrl->led_state = LIGHT_ON;
        g_callParam_.On("LRLIGHT", ctrl->led_state);
    } else if (strncmp(recvbuf, "0", strlen("0")) == 0) {
        // light off
        ctrl->led_state = LIGHT_OFF;
        g_callParam_.Off("LRLIGHT", ctrl->led_state);
    }

}

static void MotorOnOff(stContrlParam* ctrl, const char* recvbuf)
{
    if (ctrl == NULL) {
        return;
    }
    if (strncmp(recvbuf, "1", strlen("1")) == 0) {
        // light on
        ctrl->motor_state = LIGHT_ON;
        g_callParam_.On("DRLIGHT", ctrl->motor_state);
    } else if (strncmp(recvbuf, "0", strlen("0")) == 0) {
        // light off
        ctrl->motor_state = LIGHT_OFF;
        g_callParam_.Off("DRLIGHT", ctrl->motor_state);
    }

}

void InterceptString(char *descr, char *str, char *left_str, char *right_str)
{
    char *l_str = strstr(str, left_str);
    int r_str = strcspn(l_str, right_str);
    int l_len = strlen(left_str);
    int str_len = r_str - l_len;

    strncpy(descr, l_str + l_len, str_len);
    descr[str_len + 1] = '\0';

    // return descr;
}

static void ReadAnalysis(stContrlParam* ctrl, const char* recvbuf, int recvbytes)
{
    const int index = 2;
    const int index1 = 4;
    char data[TMP_BUF_MAX] = { 0 };
    char *Tem, *Lux, *Hum, *Led, *Motor;
    if (ctrl == NULL || recvbuf == NULL) {
        return;
    }
    if (memcpy_s(data, sizeof(data), &recvbuf[index1], recvbytes - index1) == 0) {
        if (strncmp(&recvbuf[index], "TD", strlen("TD")) == 0) {
            // TD
            if (memcpy_s(ctrl->devName, sizeof(ctrl->devName), &recvbuf[index1], recvbytes - index1) == 0) {
                ctrl->devName[recvbytes - index1] = 0;
                LOG(CENCTRL_DEBUG, "On connect devName=%s\n", ctrl->devName);
                g_callParam_.OnConnect(ctrl->devName, ctrl->enable);
            }
        } else if (strncmp(&recvbuf[index], "TA", strlen("TA")) == 0) {
            // TA
            // LightOnOff(ctrl, recvbuf, recvbytes);
        } else if (strncmp(&recvbuf[index], "TP", strlen("TP")) == 0) {
            // TP
            Tem = (char *)malloc(10);
            Lux = (char *)malloc(10);
            Hum = (char *)malloc(10);
            Led = (char *)malloc(10);
            Motor = (char *)malloc(10);
            InterceptString(Tem, data, "Tem:", "\n");
            InterceptString(Lux, data, "Lux:", "\n");
            InterceptString(Hum, data, "Hum:", "\n");
            InterceptString(Led, data, "LED:", "\n");
            InterceptString(Motor, data, "MOTOR:", "\n");
            // int tp = atoi(data);
            g_callParam_.DisplayeSensor(atoi(Tem), atoi(Lux), atoi(Hum));
            if (g_onClick_status == 1) {
                if (tmp < 4) {
                    tmp++;
                } else {
                    tmp = 0;
                    g_onClick_status == 0;
                    LightOnOff(ctrl, Led);
                    MotorOnOff(ctrl, Motor);
                }
            } else {
                LightOnOff(ctrl, Led);
                MotorOnOff(ctrl, Motor);
            }
            free(Tem);
            free(Lux);
            free(Hum);
            free(Led);
            free(Motor);
        }
    }
}

/*
Device communication protocol

ProtHead    Type    Data            Explanation

HM          TD      LRLIGHT         Dining Room Downlight (Top)         (device name)
                    DRLIGHT         Dining Room Downlight (Top)         (device name)
                    LANTERN         Lantern                             (device name)
                    LRLIGHT1        Dining Room Downlight (Lower)       (device name)
                    DRLIGHT1        Dining Room Downlight (Lower)       (device name)
                    AILIGHT         Aisle downlight                     (device name)
            TA      ON              On                                  (CMD value)
                    OFF             Off                                 (CMD value)
            TP      string          Temperature                         (data value)
            TH      string          Air humidity                        (data value)
            TQ      string          Smog concentration                  (data value)
*/

static int ReadHandle(stContrlParam* ctrl, fd_set* fdSet)
{
    // Processing the UI layer
    /*
    1.Listen to the on-off status and upload.
    2.Disconnect request
    3.Connecting a Device
    4.Upload the temperature and humidity results.
    */
    const int HeadSize = 4;
    char recvbuf[SOCKET_BUF_SIZE] = { 0 };
    int recvbytes;
    recvbytes = read(ctrl->service_id, recvbuf, sizeof(recvbuf) - 1);
    if (recvbytes <= 0) {
        LOG(CENCTRL_DEBUG, "service_id = %d closed\n", ctrl->service_id);
        close(ctrl->service_id);
        FD_CLR(ctrl->service_id, fdSet);
        ctrl->service_id = 0;
        ctrl->enable = SOCKET_DISABLE;
        ctrl->led_state = LIGHT_OFF;
        g_connAmount--;
        g_callParam_.OnDisconnect(ctrl->devName, ctrl->enable);
    } else if (recvbytes > HeadSize) {
        LOG(CENCTRL_DEBUG, "server recv recvbytes = %d: %s \n", recvbytes, recvbuf);
        if (strncmp(&recvbuf[0], "HM", strlen("HM")) == 0) {
            ReadAnalysis(ctrl, recvbuf, recvbytes);
        } else {
            LOG(CENCTRL_ERROR, "contrl socket fd:%d state err", ctrl->service_id);
        }
    } else {
        LOG(CENCTRL_ERROR, "ReadHandle: error!\n");
    }
    return 0;
}

// send data to L0 device
static int WriteHandle(int sockfd, const char buf[], int size)
{
    int sendbytes;
    sendbytes = write(sockfd, buf, size);
    if (sendbytes <= 0) {
        LOG(CENCTRL_ERROR, "write socket err. errno = %d\n", errno);
        return SOCKET_ERR;
    }
    LOG(CENCTRL_DEBUG, "### WriteHandle: %s , sendbytes = %d success ####\n", buf, sendbytes);
    return SOCKET_OK;
}

static int InitServer(int *sockfd)
{
    memset_s(g_contrl, sizeof(stContrlParam), 0, sizeof(stContrlParam) * MAX_CONNECT);
    g_connAmount = 0;
    int sock_fd = 0;
    int yes = 1;
    struct sockaddr_in server_addr;
    if ((sock_fd = socket(AF_INET, SOCK_STREAM, 0)) == SOCKET_ERR) {
        LOG(CENCTRL_ERROR, "socket! \n");
        return -1;
    }

    if (setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == SOCKET_ERR) {
        LOG(CENCTRL_ERROR, "setsockopt! \n");
        close(sock_fd);
        return -1;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(DEFAULT_PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;
    memset_s(server_addr.sin_zero, sizeof(server_addr.sin_zero), '\0', sizeof(server_addr.sin_zero));

    if (bind(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == SOCKET_ERR) {
        LOG(CENCTRL_ERROR, "bind! \n");
        close(sock_fd);
        return -1;
    }

    if (listen(sock_fd, MAX_CONNECT) == SOCKET_ERR) {
        LOG(CENCTRL_ERROR, "listen! \n");
        close(sock_fd);
        return -1;
    }

    LOG(CENCTRL_DEBUG, "listen port %d\n", DEFAULT_PORT);

    *sockfd = sock_fd;

    return 0;
}

static int SetConnectParam(int new_fd, int* maxsock)
{
    if (++g_connAmount == MAX_CONNECT) {
        g_connAmount--;
        LOG(CENCTRL_ERROR, "max connections arrived! \n");
        return -1;
    }
    for (int i = 0; i < MAX_CONNECT; i++) {
        if (g_contrl[i].enable == SOCKET_DISABLE) {
            g_contrl[i].enable = SOCKET_ENABLE;
            g_contrl[i].service_id = new_fd;
            g_contrl[i].led_state = 0;
            break;
        }
    }

    if (new_fd > *maxsock) {
        *maxsock = new_fd;
    }

    return 0;
}

static void SocketRelease(int sock_fd)
{
    for (int i = 0; i < MAX_CONNECT; i++) {
        if (g_contrl[i].service_id != 0) {
            close(g_contrl[i].service_id);
            g_contrl[i].service_id = 0;
            g_contrl[i].enable = SOCKET_DISABLE;
            g_contrl[i].led_state = 0;
        }
    }
    if (sock_fd) {
        close(sock_fd);
    }
}

static void *CenctrlServer(void *params)
{
    pthread_detach(pthread_self());
    int maxsock;
    int sock_fd = 0;
    struct sockaddr_in client_addr;
    socklen_t sin_size;
    fd_set fdset;
    struct timeval tv;

    if (InitServer(&sock_fd) < 0) {
        LOG(CENCTRL_ERROR, "InitServer failed! \n");
        return NULL;
    }

    sin_size = sizeof(client_addr);
    maxsock = sock_fd;
    while (g_threadFlag) {
        int ret, new_fd, i;
        FD_ZERO(&fdset);
        FD_SET(sock_fd, &fdset);
        tv.tv_sec = 10; // 10 second
        tv.tv_usec = 0;
        for (i = 0; i < MAX_CONNECT; i++) {
            if (g_contrl[i].service_id != 0) {
                FD_SET(g_contrl[i].service_id, &fdset);
            }
        }

        ret = select(maxsock + 1, &fdset, NULL, NULL, &tv);
        if (ret < SOCKET_OK) {
            LOG(CENCTRL_ERROR, "select! \n");
            break;
        } else if (ret == SOCKET_OK) {
            HeartTick(&fdset);
            continue;
        }

        for (i = 0; i < MAX_CONNECT; i++) {
            if (FD_ISSET(g_contrl[i].service_id, &fdset)) {
                ReadHandle(&g_contrl[i], &fdset);
            }
        }

        if (FD_ISSET(sock_fd, &fdset)) {
            new_fd = accept(sock_fd, (struct sockaddr *)&client_addr, &sin_size);
            if (new_fd <= 0) {
                continue;
            }
            if (SetConnectParam(new_fd, &maxsock) < 0) {
                close(new_fd);
            }
        }
    }
    SocketRelease(sock_fd);
    return NULL;
}

void SocketServiceCreate()
{
    LOG(CENCTRL_DEBUG, "SocketServiceCreate \n");
    int ret = pthread_create(&g_threadServerId, NULL, CenctrlServer, NULL);
    if (ret != SOCKET_OK) {
        LOG(CENCTRL_ERROR, "SocketServiceCreate thread error %s \n", strerror(errno));
        return;
    }
}

void RegisterSocketCallback(stCallBackParam* callParam)
{
    g_callParam_.On = callParam->On;
    g_callParam_.Off = callParam->Off;
    g_callParam_.DisplayeSensor = callParam->DisplayeSensor;
    g_callParam_.OnConnect = callParam->OnConnect;
    g_callParam_.OnDisconnect = callParam->OnDisconnect;
}

int WriteSocket(const char* devName, const char* buf, int size)
{
    if (!devName || !buf || size <= 0) {
        LOG(CENCTRL_ERROR, "param err\n");
        return SOCKET_ERR;
    }
    const int index = 4;
    LOG(CENCTRL_DEBUG, "devName=%s, buf=%s, g_connAmount = %d \n", devName, buf, g_connAmount);
    for (int i = 0; i < g_connAmount; i++) {
        if (strcmp(devName, "LRLIGHT") == 0) {
            if (g_contrl[i].enable) {
                if (strcmp(&buf[index], "ON") == 0) {
                    if (WriteHandle(g_contrl[i].service_id, "HMTALED:ON", strlen("HMTALED:ON")) == SOCKET_OK) {
                        g_contrl[i].led_state = LIGHT_ON;
                        return SOCKET_OK;
                    }
                }
                if (strcmp(&buf[index], "OFF") == 0) {
                    if (WriteHandle(g_contrl[i].service_id, "HMTALED:OFF", strlen("HMTALED:OFF")) == SOCKET_OK) {
                        g_contrl[i].led_state = LIGHT_OFF;
                        return SOCKET_OK;
                    }
                }
            }

        }
        if (strcmp(devName, "DRLIGHT") == 0) {
            if (g_contrl[i].enable) {
                if (strcmp(&buf[index], "ON") == 0) {
                    if (WriteHandle(g_contrl[i].service_id, "HMTAMOTOR:ON", strlen("HMTAMOTOR:ON")) == SOCKET_OK) {
                        g_contrl[i].motor_state = LIGHT_ON;
                        return SOCKET_OK;
                    }
                }
                if (strcmp(&buf[index], "OFF") == 0) {
                    if (WriteHandle(g_contrl[i].service_id, "HMTAMOTOR:OFF", strlen("HMTAMOTOR:OFF")) == SOCKET_OK) {
                        g_contrl[i].motor_state = LIGHT_OFF;
                        return SOCKET_OK;
                    }
                }
            }

        }

    }
    LOG(CENCTRL_ERROR, "ui send err\n");
    return SOCKET_ERR;
}

void UnRegisterSocketCallback()
{
    g_callParam_.On = NULL;
    g_callParam_.Off = NULL;
    g_callParam_.DisplayeSensor = NULL;
    g_callParam_.OnConnect = NULL;
    g_callParam_.OnDisconnect = NULL;
}

void SocketServiceDelete()
{
    g_threadFlag = SOCKET_OK;
    UnRegisterSocketCallback();
    int ret;
    if (g_threadServerId != SOCKET_OK) {
        ret = pthread_cancel(g_threadServerId);
        pthread_join(g_threadServerId, NULL);
        if (ret != SOCKET_OK) {
            LOG(CENCTRL_ERROR, "pthread_cancel(g_threadServerId) ret -> %d \n", ret);
            return;
        }
        g_threadServerId = SOCKET_OK;
        LOG(CENCTRL_DEBUG, "SocketServerDelete\n");
    }
}