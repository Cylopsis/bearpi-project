#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hdf_sbuf.h"
#include "hdf_io_service_if.h"
#include "music_samples.h"

#define PWM_SERVICE           "hdf_pwm_user"
#define PWM_CMD_SET_DUTY      1
#define PWM_CMD_SET_FREQ      2
#define PWM_CMD_LOAD_SAMPLES  3
#define PWM_CMD_START_PLAY    4
#define PWM_CMD_STOP_PLAY     5

/* ------------------------------------------------------------------ */
/* Helpers                                                             */
/* ------------------------------------------------------------------ */
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
        } else if (cmd == PWM_CMD_SET_FREQ) {
            printf("PWM freq set to %u Hz\r\n", result);
        }
    }

out:
    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
    HdfIoServiceRecycle(serv);
    return ret;
}

/* ------------------------------------------------------------------ */
/* Audio playback: load sample array then start                        */
/* ------------------------------------------------------------------ */
static int PlayMusic(void)
{
    unsigned int count = g_musicSampleCount;
    if (count == 0) {
        printf("music_samples.h is empty – run gen_music.py first\r\n");
        return -1;
    }

    struct HdfIoService *serv = HdfIoServiceBind(PWM_SERVICE);
    if (serv == NULL) {
        printf("fail to bind service %s\r\n", PWM_SERVICE);
        return -1;
    }

    /* Buffer must fit the 4-byte length prefix written by HdfSbufWriteBuffer */
    struct HdfSBuf *data  = HdfSBufObtain(count + sizeof(unsigned int) + 8);
    struct HdfSBuf *reply = HdfSBufObtainDefaultSize();
    if (data == NULL || reply == NULL) {
        printf("fail to obtain sbuf\r\n");
        HdfIoServiceRecycle(serv);
        return -1;
    }

    int ret = -1;

    /* Step 1: transfer sample array to kernel */
    if (!HdfSbufWriteBuffer(data, g_musicSamples, count)) {
        printf("fail to write sample buffer\r\n");
        goto out;
    }
    ret = serv->dispatcher->Dispatch(&serv->object, PWM_CMD_LOAD_SAMPLES, data, reply);
    if (ret != 0) {
        printf("LOAD_SAMPLES failed: %d\r\n", ret);
        goto out;
    }
    unsigned int loaded = 0;
    HdfSbufReadUint32(reply, &loaded);
    printf("loaded %u samples (%.1f s @ 8 kHz)\r\n", loaded, (float)loaded / 8000.0f);

    /* Step 2: start playback */
    HdfSbufFlush(data);
    HdfSbufFlush(reply);
    ret = serv->dispatcher->Dispatch(&serv->object, PWM_CMD_START_PLAY, data, reply);
    if (ret != 0) {
        printf("START_PLAY failed: %d\r\n", ret);
        goto out;
    }
    printf("playback started\r\n");

out:
    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
    HdfIoServiceRecycle(serv);
    return ret;
}

static int StopMusic(void)
{
    struct HdfIoService *serv = HdfIoServiceBind(PWM_SERVICE);
    if (serv == NULL) {
        printf("fail to bind service %s\r\n", PWM_SERVICE);
        return -1;
    }
    struct HdfSBuf *data  = HdfSBufObtainDefaultSize();
    struct HdfSBuf *reply = HdfSBufObtainDefaultSize();
    if (data == NULL || reply == NULL) {
        HdfIoServiceRecycle(serv);
        return -1;
    }
    HdfSbufWriteUint32(data, 0);
    int ret = serv->dispatcher->Dispatch(&serv->object, PWM_CMD_STOP_PLAY, data, reply);
    if (ret == 0) printf("playback stopped\r\n");
    else          printf("stop failed: %d\r\n", ret);
    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
    HdfIoServiceRecycle(serv);
    return ret;
}

/* ------------------------------------------------------------------ */
/* Entry point                                                         */
/* ------------------------------------------------------------------ */
int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: pwm duty <0-100>\r\n");
        printf("       pwm freq <hz>\r\n");
        printf("       pwm play\r\n");
        printf("       pwm stop\r\n");
        return -1;
    }

    if (strcmp(argv[1], "duty") == 0 && argc == 3) {
        return SendCmd(PWM_CMD_SET_DUTY, (unsigned int)atoi(argv[2]));
    } else if (strcmp(argv[1], "freq") == 0 && argc == 3) {
        return SendCmd(PWM_CMD_SET_FREQ, (unsigned int)atoi(argv[2]));
    } else if (strcmp(argv[1], "play") == 0) {
        return PlayMusic();
    } else if (strcmp(argv[1], "stop") == 0) {
        return StopMusic();
    } else {
        printf("unknown command: %s\r\n", argv[1]);
        return -1;
    }
}
