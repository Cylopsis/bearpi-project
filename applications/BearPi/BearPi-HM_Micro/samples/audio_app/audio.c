#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "hdf_sbuf.h"
#include "hdf_io_service_if.h"

#define AUDIO_SERVICE   "hdf_audio"
#define AUDIO_CMD_BEEP  1
#define AUDIO_CMD_VOL   2
#define AUDIO_CMD_STAT  3

static struct HdfIoService *OpenService(void)
{
    struct HdfIoService *serv = HdfIoServiceBind(AUDIO_SERVICE);
    if (serv == NULL)
        printf("fail to bind service %s\r\n", AUDIO_SERVICE);
    return serv;
}

static int SendBeep(unsigned int freq, unsigned int dur_ms)
{
    struct HdfIoService *serv = OpenService();
    if (serv == NULL) return -1;

    struct HdfSBuf *data  = HdfSBufObtainDefaultSize();
    struct HdfSBuf *reply = HdfSBufObtainDefaultSize();
    if (data == NULL || reply == NULL) {
        printf("fail to obtain sbuf\r\n");
        HdfIoServiceRecycle(serv);
        return -1;
    }

    int ret = -1;
    if (!HdfSbufWriteUint32(data, freq) || !HdfSbufWriteUint32(data, dur_ms)) {
        printf("fail to write sbuf\r\n");
        goto out;
    }

    ret = serv->dispatcher->Dispatch(&serv->object, AUDIO_CMD_BEEP, data, reply);
    if (ret != 0) {
        printf("dispatch failed: %d\r\n", ret);
        goto out;
    }

    /* Read back status from kernel driver */
    unsigned int stuck = 0, sr = 0;
    if (HdfSbufReadUint32(reply, &stuck) && HdfSbufReadUint32(reply, &sr)) {
        if (stuck) {
            printf("beep %u Hz for %u ms — FIFO STUCK (SAI clock not running?) SR=0x%08x\r\n",
                   freq, dur_ms, sr);
        } else {
            printf("beep %u Hz for %u ms OK  SR=0x%08x\r\n", freq, dur_ms, sr);
        }
    } else {
        printf("beep %u Hz for %u ms\r\n", freq, dur_ms);
    }

out:
    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
    HdfIoServiceRecycle(serv);
    return ret;
}

static int SendVol(unsigned int vol)
{
    struct HdfIoService *serv = OpenService();
    if (serv == NULL) return -1;

    struct HdfSBuf *data  = HdfSBufObtainDefaultSize();
    struct HdfSBuf *reply = HdfSBufObtainDefaultSize();
    if (data == NULL || reply == NULL) {
        printf("fail to obtain sbuf\r\n");
        HdfIoServiceRecycle(serv);
        return -1;
    }

    int ret = -1;
    if (!HdfSbufWriteUint32(data, vol)) {
        printf("fail to write sbuf\r\n");
        goto out;
    }

    ret = serv->dispatcher->Dispatch(&serv->object, AUDIO_CMD_VOL, data, reply);
    if (ret != 0)
        printf("dispatch failed: %d\r\n", ret);
    else
        printf("volume set to %u\r\n", vol);

out:
    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
    HdfIoServiceRecycle(serv);
    return ret;
}

static int SendStat(void)
{
    struct HdfIoService *serv = OpenService();
    if (serv == NULL) return -1;

    struct HdfSBuf *data  = HdfSBufObtainDefaultSize();
    struct HdfSBuf *reply = HdfSBufObtainDefaultSize();
    if (data == NULL || reply == NULL) {
        printf("fail to obtain sbuf\r\n");
        HdfIoServiceRecycle(serv);
        return -1;
    }

    int ret = serv->dispatcher->Dispatch(&serv->object, AUDIO_CMD_STAT, data, reply);
    if (ret != 0) {
        printf("dispatch failed: %d\r\n", ret);
    } else {
        unsigned int cr1b = 0, srb = 0, pll4cr = 0, sai2sel = 0, sra = 0;
        HdfSbufReadUint32(reply, &cr1b);
        HdfSbufReadUint32(reply, &srb);
        HdfSbufReadUint32(reply, &pll4cr);
        HdfSbufReadUint32(reply, &sai2sel);
        HdfSbufReadUint32(reply, &sra);
        unsigned int flvlb = (srb >> 16) & 7u;
        unsigned int flvla = (sra >> 16) & 7u;
        printf("SAI2A SR=0x%08x  FLVL=%u(%s)\r\n",
               sra, flvla, flvla == 0 ? "empty" : flvla == 5 ? "FULL" : "ok");
        printf("SAI2B CR1=0x%08x  SR=0x%08x  FLVL=%u(%s)\r\n",
               cr1b, srb, flvlb,
               flvlb == 0 ? "empty" : flvlb == 5 ? "FULL-stuck" : "ok");
        printf("PLL4CR=0x%08x  SAI2CKSELR=0x%08x (src=%u=%s)\r\n",
               pll4cr, sai2sel, sai2sel & 7u,
               (sai2sel & 7u) == 0 ? "PLL4_Q" :
               (sai2sel & 7u) == 1 ? "PLL3_Q" :
               (sai2sel & 7u) == 5 ? "PLL3_R" : "other");
    }

    HdfSBufRecycle(data);
    HdfSBufRecycle(reply);
    HdfIoServiceRecycle(serv);
    return ret;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        printf("Usage: audio beep <freq_hz> [duration_ms]\r\n");
        printf("       audio vol  <0-127>\r\n");
        printf("       audio stat\r\n");
        return -1;
    }

    if (strcmp(argv[1], "beep") == 0) {
        if (argc < 3) { printf("Usage: audio beep <freq_hz> [duration_ms]\r\n"); return -1; }
        unsigned int freq = (unsigned int)atoi(argv[2]);
        unsigned int dur  = (argc >= 4) ? (unsigned int)atoi(argv[3]) : 500;
        return SendBeep(freq, dur);
    } else if (strcmp(argv[1], "vol") == 0) {
        if (argc < 3) { printf("Usage: audio vol <0-127>\r\n"); return -1; }
        unsigned int vol = (unsigned int)atoi(argv[2]);
        return SendVol(vol);
    } else if (strcmp(argv[1], "stat") == 0) {
        return SendStat();
    } else {
        printf("unknown subcommand: %s\r\n", argv[1]);
        return -1;
    }
}
