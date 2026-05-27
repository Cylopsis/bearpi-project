#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "pwm_if.h"

#define HDF_LOG_TAG pwm_user_driver

/* Command codes shared with the user-space app */
#define PWM_CMD_SET_DUTY  1   /* payload: uint32 duty percent (0-100) */
#define PWM_CMD_SET_FREQ  2   /* payload: uint32 frequency in Hz      */

/* PA6 maps to PWM port 3 in pwm_config.hcs */
#define PWM_PA6_NUM   3
/* Effective timer clock after prescaler, from pwm_config.hcs tim_clk_hz */
#define TIM_CLK_HZ    10000000U

static DevHandle g_pwmHandle = NULL;

int32_t PwmUserDispatch(struct HdfDeviceIoClient *client, int cmdCode,
                        struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)client;
    if (g_pwmHandle == NULL) {
        HDF_LOGE("%s: pwm handle not open", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    uint32_t value;
    struct PwmConfig cfg;

    if (!HdfSbufReadUint32(data, &value)) {
        return HDF_ERR_INVALID_PARAM;
    }

    if (PwmGetConfig(g_pwmHandle, &cfg) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }

    switch (cmdCode) {
    case PWM_CMD_SET_DUTY: {
        if (value > 100) value = 100;
        uint32_t duty = (value == 0) ? 1 : ((cfg.period + 1) * value / 100);
        if (duty < 1) duty = 1;
        if (duty > cfg.period) duty = cfg.period;
        cfg.duty = duty;
        cfg.status = PWM_ENABLE_STATUS;
        if (PwmSetConfig(g_pwmHandle, &cfg) != HDF_SUCCESS) {
            return HDF_FAILURE;
        }
        HdfSbufWriteUint32(reply, value);
        HDF_LOGD("%s: duty -> %u%% (ticks=%u)", __func__, value, duty);
        break;
    }
    case PWM_CMD_SET_FREQ: {
        if (value == 0 || value > TIM_CLK_HZ) {
            return HDF_ERR_INVALID_PARAM;
        }
        uint32_t new_period = TIM_CLK_HZ / value - 1;
        /* preserve current duty ratio when changing frequency */
        uint32_t new_duty = (cfg.period > 0) ?
            ((new_period + 1) * cfg.duty / (cfg.period + 1)) : 1;
        if (new_duty < 1) new_duty = 1;
        if (new_duty > new_period) new_duty = new_period;
        cfg.period = new_period;
        cfg.duty   = new_duty;
        cfg.status = PWM_ENABLE_STATUS;
        if (PwmSetConfig(g_pwmHandle, &cfg) != HDF_SUCCESS) {
            return HDF_FAILURE;
        }
        HdfSbufWriteUint32(reply, value);
        HDF_LOGD("%s: freq -> %u Hz (period=%u duty=%u)", __func__, value, new_period, new_duty);
        break;
    }
    default:
        return HDF_ERR_NOT_SUPPORT;
    }
    return HDF_SUCCESS;
}

int32_t HdfPwmUserBind(struct HdfDeviceObject *deviceObject)
{
    if (deviceObject == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
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
    /* start output with default config */
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
