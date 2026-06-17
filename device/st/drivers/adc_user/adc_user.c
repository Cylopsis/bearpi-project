#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "adc_if.h"

#define HDF_LOG_TAG adc_user_driver

/* Command codes shared with user-space app */
#define ADC_CMD_READ      1   /* in: uint32 channel; out: uint32 raw value */

/* NTC + LM35 both hang off ADC instance 1 (E53_ADC1_INP1 = ch1, PB0 = ch9) */
#define ADC_DEV_NUM       1

static DevHandle g_adcHandle = NULL;

int32_t AdcUserDispatch(struct HdfDeviceIoClient *client, int cmdCode,
                        struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)client;
    if (g_adcHandle == NULL) {
        HDF_LOGE("%s: adc handle not open", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }

    switch (cmdCode) {
    case ADC_CMD_READ: {
        uint32_t channel;
        uint32_t val = 0;
        if (!HdfSbufReadUint32(data, &channel)) {
            return HDF_ERR_INVALID_PARAM;
        }
        if (AdcRead(g_adcHandle, channel, &val) != HDF_SUCCESS) {
            HDF_LOGE("%s: AdcRead ch%u failed", __func__, channel);
            return HDF_FAILURE;
        }
        if (!HdfSbufWriteUint32(reply, val)) {
            return HDF_FAILURE;
        }
        HDF_LOGD("%s: ch%u -> %u", __func__, channel, val);
        break;
    }
    default:
        return HDF_ERR_NOT_SUPPORT;
    }

    return HDF_SUCCESS;
}

int32_t HdfAdcUserBind(struct HdfDeviceObject *deviceObject)
{
    if (deviceObject == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    static struct IDeviceIoService adcUserService = {
        .Dispatch = AdcUserDispatch,
    };
    deviceObject->service = (struct IDeviceIoService *)(&adcUserService);
    HDF_LOGD("adc user driver bind success");
    return HDF_SUCCESS;
}

int32_t HdfAdcUserInit(struct HdfDeviceObject *device)
{
    (void)device;
    g_adcHandle = AdcOpen(ADC_DEV_NUM);
    if (g_adcHandle == NULL) {
        HDF_LOGE("%s: AdcOpen(%d) failed", __func__, ADC_DEV_NUM);
        return HDF_FAILURE;
    }
    HDF_LOGD("adc user driver init ok (ADC%d)", ADC_DEV_NUM);
    return HDF_SUCCESS;
}

void HdfAdcUserRelease(struct HdfDeviceObject *deviceObject)
{
    (void)deviceObject;
    if (g_adcHandle != NULL) {
        AdcClose(g_adcHandle);
        g_adcHandle = NULL;
    }
    HDF_LOGD("adc user driver release success");
}

struct HdfDriverEntry g_adcUserDriverEntry = {
    .moduleVersion = 1,
    .moduleName    = "HDF_ADC_USER",
    .Bind          = HdfAdcUserBind,
    .Init          = HdfAdcUserInit,
    .Release       = HdfAdcUserRelease,
};

HDF_INIT(g_adcUserDriverEntry);
