#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hdf_sbuf.h"
#include "hdf_io_service_if.h"

#define PWM_SERVICE      "hdf_pwm_user"
#define PWM_CMD_SET_DUTY  1
#define PWM_CMD_SET_FREQ  2

static int SendCmd(int cmd, unsigned int value)
{
    struct HdfIoService *serv = HdfIoServiceBind(PWM_SERVICE);
    if (serv == NULL) {
        printf("fail to bind service %s\r\n", PWM_SERVICE);
        return -1;
    }

    struct HdfSBuf *data  = HdfSBufObtainDefaultSize();
    struct HdfSBuf *reply = HdfSBufObtainDefaultSize();
    if (data == NULL || reply == NULL) {
        printf("fail to obtain sbuf\r\n");
        HdfIoServiceRecycle(serv);
        return -1;
    }

    int ret = -1;
    if (!HdfSbufWriteUint32(data, value)) {
        printf("fail to write sbuf\r\n");
        goto out;
    }

    ret = serv->dispatcher->Dispatch(&serv->object, cmd, data, reply);
    if (ret != 0) {
        printf("dispatch failed: %d\r\n", ret);
        goto out;
    }

    unsigned int result = 0;
    if (HdfSbufReadUint32(reply, &result)) {
        if (cmd == PWM_CMD_SET_DUTY) {
            printf("PWM duty set to %u%%\r\n", result);
        } else {
            printf("PWM freq set to %u Hz\r\n", result);
        }
    }

out:
    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
    HdfIoServiceRecycle(serv);
    return ret;
}

int main(int argc, char **argv)
{
    if (argc != 3) {
        printf("Usage: pwm duty <0-100>\r\n");
        printf("       pwm freq <hz>\r\n");
        return -1;
    }

    int cmd;
    if (strcmp(argv[1], "duty") == 0) {
        cmd = PWM_CMD_SET_DUTY;
    } else if (strcmp(argv[1], "freq") == 0) {
        cmd = PWM_CMD_SET_FREQ;
    } else {
        printf("unknown subcommand: %s\r\n", argv[1]);
        return -1;
    }

    unsigned int value = (unsigned int)atoi(argv[2]);
    return SendCmd(cmd, value);
}
