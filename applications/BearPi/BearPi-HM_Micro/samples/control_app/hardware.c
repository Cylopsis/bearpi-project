#include "hardware.h"
#include <stdio.h>
#include <pthread.h>
#include "hdf_sbuf.h"
#include "hdf_io_service_if.h"
#include "bme280.h"

/* Service names (must match the HDF driver serviceName fields) */
#define ADC_SERVICE     "hdf_adc_user"
#define PWM_SERVICE     "hdf_pwm_user"
#define STATE_SERVICE   "hdf_state_user"

/* Command codes (must match the driver dispatch handlers) */
#define ADC_CMD_READ        1
#define PWM_CMD_SET_DUTY    1
#define STATE_CMD_SET       1

static struct HdfIoService *g_adcServ   = NULL;
static struct HdfIoService *g_pwmServ   = NULL;
static struct HdfIoService *g_stateServ = NULL;

/* Serialize all HDF IPC: hw_* is called from main loop, PID thread, etc. */
static pthread_mutex_t g_hwLock = PTHREAD_MUTEX_INITIALIZER;

int hw_init(void)
{
    g_adcServ = HdfIoServiceBind(ADC_SERVICE);
    if (g_adcServ == NULL) {
        printf("[hw] bind %s failed\n", ADC_SERVICE);
        return -1;
    }
    g_pwmServ = HdfIoServiceBind(PWM_SERVICE);
    if (g_pwmServ == NULL) {
        printf("[hw] bind %s failed\n", PWM_SERVICE);
        return -1;
    }
    g_stateServ = HdfIoServiceBind(STATE_SERVICE);
    if (g_stateServ == NULL) {
        printf("[hw] bind %s failed\n", STATE_SERVICE);
        return -1;
    }
    if (bme280_init() != 0) {
        printf("[hw] bme280 init failed\n");
        return -1;
    }
    printf("[hw] all services bound\n");
    return 0;
}

void hw_deinit(void)
{
    if (g_adcServ != NULL)   { HdfIoServiceRecycle(g_adcServ);   g_adcServ = NULL; }
    if (g_pwmServ != NULL)   { HdfIoServiceRecycle(g_pwmServ);   g_pwmServ = NULL; }
    if (g_stateServ != NULL) { HdfIoServiceRecycle(g_stateServ); g_stateServ = NULL; }
    bme280_deinit();
}

/* Number of raw samples averaged per ADC read to suppress noise. */
#define ADC_OVERSAMPLE  16

/* Single raw conversion. Caller must hold g_hwLock. */
static int adc_read_once(uint32_t channel, uint32_t *val)
{
    int ret = -1;
    struct HdfSBuf *data  = HdfSBufObtainDefaultSize();
    struct HdfSBuf *reply = HdfSBufObtainDefaultSize();
    if (data == NULL || reply == NULL) {
        goto out;
    }
    if (!HdfSbufWriteUint32(data, channel)) {
        goto out;
    }
    if (g_adcServ->dispatcher->Dispatch(&g_adcServ->object, ADC_CMD_READ, data, reply) != 0) {
        goto out;
    }
    if (!HdfSbufReadUint32(reply, val)) {
        goto out;
    }
    ret = 0;
out:
    if (data != NULL)  { HdfSBufRecycle(data); }
    if (reply != NULL) { HdfSBufRecycle(reply); }
    return ret;
}

int hw_adc_read(uint32_t channel, uint32_t *val)
{
    if (g_adcServ == NULL || val == NULL) {
        return -1;
    }

    uint64_t acc = 0;
    int got = 0;
    pthread_mutex_lock(&g_hwLock);
    for (int i = 0; i < ADC_OVERSAMPLE; i++) {
        uint32_t v = 0;
        if (adc_read_once(channel, &v) == 0) {
            acc += v;
            got++;
        }
    }
    pthread_mutex_unlock(&g_hwLock);

    if (got == 0) {
        return -1;
    }
    *val = (uint32_t)(acc / got);
    return 0;
}

int hw_box_env_read(float *temp_c, float *press_hpa, float *humidity_rh)
{
    pthread_mutex_lock(&g_hwLock);
    int ret = bme280_read(temp_c, press_hpa, humidity_rh);
    pthread_mutex_unlock(&g_hwLock);
    return ret;
}

int hw_pwm_set_duty(uint32_t percent)
{
    int ret = -1;
    if (g_pwmServ == NULL) {
        return -1;
    }
    if (percent > 100) {
        percent = 100;
    }

    pthread_mutex_lock(&g_hwLock);
    struct HdfSBuf *data  = HdfSBufObtainDefaultSize();
    struct HdfSBuf *reply = HdfSBufObtainDefaultSize();
    if (data == NULL || reply == NULL) {
        goto out;
    }
    if (!HdfSbufWriteUint32(data, percent)) {
        goto out;
    }
    if (g_pwmServ->dispatcher->Dispatch(&g_pwmServ->object, PWM_CMD_SET_DUTY, data, reply) != 0) {
        goto out;
    }
    ret = 0;
out:
    if (data != NULL)  { HdfSBufRecycle(data); }
    if (reply != NULL) { HdfSBufRecycle(reply); }
    pthread_mutex_unlock(&g_hwLock);
    return ret;
}

int hw_gpio_set_state(uint8_t level)
{
    int ret = -1;
    if (g_stateServ == NULL) {
        return -1;
    }

    pthread_mutex_lock(&g_hwLock);
    struct HdfSBuf *data  = HdfSBufObtainDefaultSize();
    struct HdfSBuf *reply = HdfSBufObtainDefaultSize();
    if (data == NULL || reply == NULL) {
        goto out;
    }
    if (!HdfSbufWriteUint8(data, level)) {
        goto out;
    }
    if (g_stateServ->dispatcher->Dispatch(&g_stateServ->object, STATE_CMD_SET, data, reply) != 0) {
        goto out;
    }
    ret = 0;
out:
    if (data != NULL)  { HdfSBufRecycle(data); }
    if (reply != NULL) { HdfSBufRecycle(reply); }
    pthread_mutex_unlock(&g_hwLock);
    return ret;
}
