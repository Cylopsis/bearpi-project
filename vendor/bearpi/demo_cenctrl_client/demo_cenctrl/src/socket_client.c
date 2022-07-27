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
#include <securec.h>
#include "socket_udp.h"
#include "socket_client_1.h"
#include "cenctrl_log.h"


static pthread_t g_discThreadID = 0;


#define    UDP_DEF_PORT        9090
#define    TCP_DEF_PORT        8989

#define    MSG_DATE_BUF_LEN    24
#define    PROTOCOL_HEADER        "HM"
#define    DATA_DEVICE_NAME    "TD"
#define    DATA_SWITCH         "TA"
#define    DATA_MSG_ON         "on"
#define    DATA_MSG_OFF        "off"

#ifndef    ARRAYSIZE
#define    ARRAYSIZE(a)    (sizeof((a)) / sizeof((a[0])))
#endif
#ifndef    bool
#define    bool    unsigned char
#endif
#ifndef    true
#define    true    1
#endif
#ifndef    false
#define    false    0
#endif

static void ResolveDevName(SocketEventCallback callback, char *value);
static void ResolveSwitch(SocketEventCallback callback, char *value);

typedef union {
    char msg[MSG_DATE_BUF_LEN];
    struct {
        char head[2];
        char cmd[2];
        char buff[20];
    } msg_info;
} MsgInfo;

typedef struct {
    char cmd[MSG_DATE_BUF_LEN];
    void (*func)(SocketEventCallback callback, char *value);
} MsgData;

static MsgData g_msgData[] = {
    {DATA_DEVICE_NAME, ResolveDevName},
    {DATA_SWITCH, ResolveSwitch}
};

static bool g_threadRunning = 0;

// ********************************************************************************************************************************** //
static bool IsEqualTo(const char *msg1, const char *msg2, int length)
{
    if (msg1 == NULL || msg2 == NULL || length <= 0) {
        LOG(CENCTRL_ERROR, "NULL POINT! \n");
        return 0;
    }

    return (strncasecmp(msg1, msg2, length) == 0);
}

static void ResolveDevName(SocketEventCallback callback, char *value)
{
    LOG(CENCTRL_INFO, " ########### value : %s ################ \n", value);
}

static void ResolveSwitch(SocketEventCallback callback, char *value)
{
    LOG(CENCTRL_INFO, " ########### value : %s ################ \n", value);
    if (callback != NULL) {
        callback(SOCKET_SET_CMD, value);
    }
}

static int SocketClientResolveData(const char *data, int len, SocketEventCallback callback)
{
    MsgInfo msgInfo = { 0 };
    if (data == NULL || len <= 0) {
        LOG(CENCTRL_ERROR, "NULL POINT!\n");
        return -1;
    }
    if (len > MSG_DATE_BUF_LEN) {
        len = MSG_DATE_BUF_LEN;
    }
    memcpy(msgInfo.msg, data, len);
    LOG(CENCTRL_DEBUG, "head:%s\n", msgInfo.msg_info.head);
    for (int i = 0; i < ARRAYSIZE(g_msgData); i++) {
        if (IsEqualTo(msgInfo.msg_info.cmd, g_msgData[i].cmd, strlen(g_msgData[i].cmd))) {
            g_msgData[i].func(callback, msgInfo.msg_info.buff);
            LOG(CENCTRL_INFO, " cmd %s is match! \n", g_msgData[i].cmd);
            break;
        }
    }

    return 0;
}

static int SocketOpen(const char *ip, int port)
{
    int sockfd = 0;
    int yes = 1;
    struct sockaddr_in recvAddr;
    if (ip == NULL || port <= 0) {
        return -1;
    }
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        LOG(CENCTRL_ERROR, "socket failed! errno=%d\n", errno);
        return -1;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVBUF, &yes, sizeof(int)) == -1) {
        LOG(CENCTRL_ERROR, "setsockopt! \n");
        close(sockfd);
        return -1;
    }
    // memset(&recvAddr, 0x00, sizeof(recvAddr));
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_port = htons(port);
    recvAddr.sin_addr.s_addr = inet_addr(ip);
    if (connect(sockfd, (struct sockaddr *)&recvAddr, sizeof(recvAddr)) < 0) {
        LOG(CENCTRL_ERROR, "connect failed! errno=%d\n", errno);
        close(sockfd);
        return -1;
    }
    return sockfd;
}

static int GetServerIp(char *ip, int size)
{
    char recMsg[256] = { 0 };
    int sockfd = 0;
    int yes = 1;
    char *tmp = NULL;
    struct sockaddr_in localAddr, serverAddr;
    int sockaddr_len = sizeof(serverAddr);
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        LOG(CENCTRL_ERROR, "socket failed!\n");
        return -1;
    }
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
        LOG(CENCTRL_ERROR, "setsockopt! \n");
        close(sockfd);
        return -1;
    }
    memset(&localAddr, 0x00, sizeof(localAddr));
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(UDP_DEF_PORT);
    localAddr.sin_addr.s_addr = INADDR_ANY;
    memset_s(localAddr.sin_zero, sizeof(localAddr.sin_zero), '\0', sizeof(localAddr.sin_zero));
    if (bind(sockfd, (struct sockaddr *)&localAddr, sizeof(localAddr)) < 0) {
        LOG(CENCTRL_ERROR, "bind failed! errno : %d(%s)\n", errno, strerror(errno));
        close(sockfd);
        return -1;
    }
    if (recvfrom(sockfd, recMsg, sizeof(recMsg), 0, (struct sockaddr *)&serverAddr, &sockaddr_len) < 0) {
        LOG(CENCTRL_ERROR, "recvfrom failed! \n");
        close(sockfd);
        return -1;
    }
    tmp = inet_ntoa(serverAddr.sin_addr);
    LOG(CENCTRL_DEBUG, "the server ip is %s \n", tmp);
    if (ip == NULL || size < strlen(tmp)) {
        LOG(CENCTRL_ERROR, "params is invalid!! \n");
        close(sockfd);
        return -1;
    }

    strncpy(ip, tmp, strlen(tmp));

    close(sockfd);

    return 0;
}
int tcp_sockfd = -1;
int g_connected = 0;

static void SocketClientProcess(void *arg)
{
    pthread_detach(pthread_self());
    // int tcp_sockfd = -1;
    char ipBuf[256] = { 0 };
    char sendBuf[256] = { 0 };
    char mDeviceName[256] = { 0 };
    SocketCallback *mCallback = (SocketCallback *)arg;
    if (mCallback == NULL) {
        LOG(CENCTRL_ERROR, "socket callback is NULL! \n");
        return;
    }
    if (GetServerIp(ipBuf, sizeof(ipBuf)) < 0) {
        LOG(CENCTRL_ERROR, "get server ip failed! \n");
        g_threadRunning = 0;
        return;
    }
    tcp_sockfd = SocketOpen((const char *)ipBuf, TCP_DEF_PORT);
    if (tcp_sockfd < 0) {
        LOG(CENCTRL_ERROR, "socket open failed! \n");
        g_threadRunning = 0;
        return;
    }
    if (mCallback->socketEvent != NULL) {
        mCallback->socketEvent(SOCKET_CONNECTTED, NULL);
    }

    if (mCallback->socketGetDeviceName != NULL) {
        mCallback->socketGetDeviceName(mDeviceName, sizeof(mDeviceName));
        sprintf(sendBuf, "%s%s%s", PROTOCOL_HEADER, DATA_DEVICE_NAME, mDeviceName);
        if (send(tcp_sockfd, "HMTDDRLIGHT", strlen(sendBuf), 0) < 0) {
            LOG(CENCTRL_ERROR, "send %s failed! \n", sendBuf);
            goto EXIT;
        }
        if (send(tcp_sockfd, "HMTDLRLIGHT", strlen(sendBuf), 0) < 0) {
            LOG(CENCTRL_ERROR, "send %s failed! \n", sendBuf);
            goto EXIT;
        }
        g_connected = 1;
    }
    while (g_threadRunning) {
        char recvBuf[1024] = { 0 };
        int recvBytes = recv(tcp_sockfd, recvBuf, sizeof(recvBuf), 0);
        if (recvBytes <= 0) {
            break;
        }
        LOG(CENCTRL_INFO, "recvMsg[%d] : %s \n", recvBytes, recvBuf);
        if (SocketClientResolveData((const char *)recvBuf, recvBytes, mCallback->socketEvent) < 0) {
            break;
        }
    }

EXIT:
    close(tcp_sockfd);
    tcp_sockfd = -1;
    g_connected = 0;
    if (mCallback->socketEvent != NULL) {
        mCallback->socketEvent(SOCKET_DISCONNECT, NULL);
    }
    g_threadRunning = 0;
}
void SocketSendData(char *buf)
{
    if (g_connected) {
        if (send(tcp_sockfd, buf, strlen(buf), 0) < 0) {
            LOG(CENCTRL_ERROR, "send %s failed! \n", buf);
        }
    }
}
void SocketClientStart(SocketCallback *gCallback)
{
    LOG(CENCTRL_DEBUG, "create the broadcast\n");
    g_threadRunning = 1;
    SocketCallback *prog = malloc(sizeof(SocketCallback));
    if (prog == NULL) {
        return;
    } else {
        *prog = *gCallback;
    }

    int ret = pthread_create(&g_discThreadID, NULL, SocketClientProcess, (void *)prog);
    if (ret != 0) {
        LOG(CENCTRL_ERROR, "socket_udp_main [ERROR]thread error %s\n", strerror(errno));
        return;
    }
    LOG(CENCTRL_DEBUG, "create the broadcast end\n");
}

void SocketClientStop(void)
{
    int ret;
    if (g_threadRunning) {
        if (g_discThreadID != 0) {
            ret = pthread_cancel(g_discThreadID);
            pthread_join(g_discThreadID, NULL);
            if (ret != 0) {
                LOG(CENCTRL_ERROR, "pthread_cancel(g_discThreadID) ret -> %d \n", ret);
                return;
            }
            g_discThreadID = 0;
            LOG(CENCTRL_DEBUG, " SocketUdpDelete\n ");
        }
        g_threadRunning = false;
    }
}
