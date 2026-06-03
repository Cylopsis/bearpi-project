#include <string.h>
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "osal_mem.h"
#include "osal_thread.h"
#include "osal_time.h"
#include "pwm_if.h"

#define HDF_LOG_TAG pwm_user_driver

/* Command codes shared with user-space app */
#define PWM_CMD_SET_DUTY      1   /* uint32 duty percent (0-100) */
#define PWM_CMD_SET_FREQ      2   /* uint32 frequency in Hz      */
#define PWM_CMD_LOAD_SAMPLES  3   /* byte buffer: raw 8-bit PCM  */
#define PWM_CMD_START_PLAY    4   /* no payload                  */
#define PWM_CMD_STOP_PLAY     5   /* no payload                  */

/* PA6 → PWM port 3 (pwm_config.hcs) */
#define PWM_PA6_NUM         3
/* Timer clock after prescaler */
#define TIM_CLK_HZ          10000000U
/* 8-bit PWM depth for audio; carrier ≈ 39 kHz > buzzer resonance */
#define AUDIO_PERIOD_TICKS  255U
/* 8 kHz audio sample rate → 125 µs per sample */
#define AUDIO_SAMPLE_US     125U

extern void Stm32PwmAudioSetDuty(uint32_t dutyTicks);

static DevHandle          g_pwmHandle   = NULL;
static uint8_t           *g_sampleBuf   = NULL;
static uint32_t           g_sampleCount = 0;
static volatile bool      g_playing     = false;
static volatile bool      g_taskDone    = true;
static struct OsalThread  g_playThread;

/* ------------------------------------------------------------------ */
/* Playback task: runs at high priority, busy-waits 125 µs per sample */
/* ------------------------------------------------------------------ */
static int32_t PlaybackTask(void *arg)
{
    (void)arg;
    uint32_t i;
    for (i = 0; i < g_sampleCount && g_playing; i++) {
        Stm32PwmAudioSetDuty(g_sampleBuf[i]);
        OsalUDelay(AUDIO_SAMPLE_US);
    }
    /* Return to mid-point (silence) when done */
    Stm32PwmAudioSetDuty(AUDIO_PERIOD_TICKS / 2);
    g_playing  = false;
    g_taskDone = true;
    return 0;
}

/* ------------------------------------------------------------------ */
/* Dispatch handler                                                    */
/* ------------------------------------------------------------------ */
int32_t PwmUserDispatch(struct HdfDeviceIoClient *client, int cmdCode,
                        struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)client;
    if (g_pwmHandle == NULL) {
        HDF_LOGE("%s: pwm handle not open", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    struct PwmConfig cfg;

    switch (cmdCode) {

    /* ---- existing commands ---- */
    case PWM_CMD_SET_DUTY: {
        uint32_t value;
        if (!HdfSbufReadUint32(data, &value)) return HDF_ERR_INVALID_PARAM;
        if (value > 100) value = 100;
        if (PwmGetConfig(g_pwmHandle, &cfg) != HDF_SUCCESS) return HDF_FAILURE;
        uint32_t duty = (value == 0) ? 1 : ((cfg.period + 1) * value / 100);
        if (duty < 1) duty = 1;
        if (duty > cfg.period) duty = cfg.period;
        cfg.duty   = duty;
        cfg.status = PWM_ENABLE_STATUS;
        if (PwmSetConfig(g_pwmHandle, &cfg) != HDF_SUCCESS) return HDF_FAILURE;
        HdfSbufWriteUint32(reply, value);
        HDF_LOGD("%s: duty -> %u%% (ticks=%u)", __func__, value, duty);
        break;
    }

    case PWM_CMD_SET_FREQ: {
        uint32_t value;
        if (!HdfSbufReadUint32(data, &value)) return HDF_ERR_INVALID_PARAM;
        if (value == 0 || value > TIM_CLK_HZ) return HDF_ERR_INVALID_PARAM;
        if (PwmGetConfig(g_pwmHandle, &cfg) != HDF_SUCCESS) return HDF_FAILURE;
        uint32_t newPeriod = TIM_CLK_HZ / value - 1;
        uint32_t newDuty = (cfg.period > 0) ?
            ((newPeriod + 1) * cfg.duty / (cfg.period + 1)) : 1;
        if (newDuty < 1) newDuty = 1;
        if (newDuty > newPeriod) newDuty = newPeriod;
        cfg.period = newPeriod;
        cfg.duty   = newDuty;
        cfg.status = PWM_ENABLE_STATUS;
        if (PwmSetConfig(g_pwmHandle, &cfg) != HDF_SUCCESS) return HDF_FAILURE;
        HdfSbufWriteUint32(reply, value);
        HDF_LOGD("%s: freq -> %u Hz (period=%u duty=%u)", __func__, value, newPeriod, newDuty);
        break;
    }

    /* ---- audio commands ---- */
    case PWM_CMD_LOAD_SAMPLES: {
        if (g_playing) {
            HDF_LOGE("%s: cannot load while playing", __func__);
            return HDF_ERR_DEVICE_BUSY;
        }
        const uint8_t *buf = NULL;
        uint32_t len = 0;
        if (!HdfSbufReadBuffer(data, (const void **)&buf, &len) || len == 0) {
            HDF_LOGE("%s: read sample buffer failed", __func__);
            return HDF_ERR_INVALID_PARAM;
        }
        if (g_sampleBuf != NULL) {
            OsalMemFree(g_sampleBuf);
            g_sampleBuf   = NULL;
            g_sampleCount = 0;
        }
        g_sampleBuf = (uint8_t *)OsalMemCalloc(len);
        if (g_sampleBuf == NULL) {
            HDF_LOGE("%s: OsalMemCalloc(%u) failed", __func__, len);
            return HDF_ERR_MALLOC_FAIL;
        }
        memcpy(g_sampleBuf, buf, len);
        g_sampleCount = len;
        HdfSbufWriteUint32(reply, len);
        HDF_LOGD("%s: loaded %u samples", __func__, len);
        break;
    }

    case PWM_CMD_START_PLAY: {
        if (g_sampleBuf == NULL || g_sampleCount == 0) {
            HDF_LOGE("%s: no samples loaded", __func__);
            return HDF_ERR_INVALID_OBJECT;
        }
        if (g_playing) {
            HDF_LOGE("%s: already playing", __func__);
            return HDF_ERR_DEVICE_BUSY;
        }
        /* Wait up to 500 ms for any previous task to exit */
        for (int w = 0; w < 50 && !g_taskDone; w++) OsalMSleep(10);

        /* Set period = 255 (8-bit depth) so duty range is 0-255 */
        if (PwmGetConfig(g_pwmHandle, &cfg) != HDF_SUCCESS) return HDF_FAILURE;
        cfg.period = AUDIO_PERIOD_TICKS;
        cfg.duty   = AUDIO_PERIOD_TICKS / 2;
        cfg.status = PWM_ENABLE_STATUS;
        if (PwmSetConfig(g_pwmHandle, &cfg) != HDF_SUCCESS) return HDF_FAILURE;

        g_playing  = true;
        g_taskDone = false;

        struct OsalThreadParam param = {
            .name      = "pwm_audio",
            .stackSize = 4096,
            .priority  = OSAL_THREAD_PRI_HIGH,
        };
        if (OsalThreadCreate(&g_playThread, PlaybackTask, NULL) != HDF_SUCCESS ||
            OsalThreadStart(&g_playThread, &param) != HDF_SUCCESS) {
            g_playing  = false;
            g_taskDone = true;
            HDF_LOGE("%s: failed to start playback task", __func__);
            return HDF_FAILURE;
        }
        HdfSbufWriteUint32(reply, g_sampleCount);
        HDF_LOGD("%s: playback started (%u samples)", __func__, g_sampleCount);
        break;
    }

    case PWM_CMD_STOP_PLAY: {
        g_playing = false;
        for (int w = 0; w < 50 && !g_taskDone; w++) OsalMSleep(10);
        Stm32PwmAudioSetDuty(AUDIO_PERIOD_TICKS / 2);
        HdfSbufWriteUint32(reply, 0);
        HDF_LOGD("%s: playback stopped", __func__);
        break;
    }

    default:
        return HDF_ERR_NOT_SUPPORT;
    }

    return HDF_SUCCESS;
}

/* ------------------------------------------------------------------ */
/* Driver lifecycle                                                    */
/* ------------------------------------------------------------------ */
int32_t HdfPwmUserBind(struct HdfDeviceObject *deviceObject)
{
    if (deviceObject == NULL) return HDF_ERR_INVALID_OBJECT;
    static struct IDeviceIoService pwmUserService = {
        .Dispatch = PwmUserDispatch,
    };
    deviceObject->service = (struct IDeviceIoService *)(&pwmUserService);
    HDF_LOGD("pwm user driver bind success");
    return HDF_SUCCESS;
}

int32_t HdfPwmUserInit(struct HdfDeviceObject *device)
{
    (void)device;
    g_pwmHandle = PwmOpen(PWM_PA6_NUM);
    if (g_pwmHandle == NULL) {
        HDF_LOGE("%s: PwmOpen(%d) failed", __func__, PWM_PA6_NUM);
        return HDF_FAILURE;
    }
    struct PwmConfig cfg;
    if (PwmGetConfig(g_pwmHandle, &cfg) == HDF_SUCCESS) {
        cfg.status = PWM_ENABLE_STATUS;
        PwmSetConfig(g_pwmHandle, &cfg);
    }
    HDF_LOGD("pwm user driver init ok (PA6, PWM%d)", PWM_PA6_NUM);
    return HDF_SUCCESS;
}

void HdfPwmUserRelease(struct HdfDeviceObject *deviceObject)
{
    (void)deviceObject;
    g_playing = false;
    for (int w = 0; w < 50 && !g_taskDone; w++) OsalMSleep(10);
    if (g_sampleBuf != NULL) {
        OsalMemFree(g_sampleBuf);
        g_sampleBuf   = NULL;
        g_sampleCount = 0;
    }
    if (g_pwmHandle != NULL) {
        PwmClose(g_pwmHandle);
        g_pwmHandle = NULL;
    }
    HDF_LOGD("pwm user driver release success");
}

struct HdfDriverEntry g_pwmUserDriverEntry = {
    .moduleVersion = 1,
    .moduleName    = "HDF_PWM_USER",
    .Bind          = HdfPwmUserBind,
    .Init          = HdfPwmUserInit,
    .Release       = HdfPwmUserRelease,
};

HDF_INIT(g_pwmUserDriverEntry);
