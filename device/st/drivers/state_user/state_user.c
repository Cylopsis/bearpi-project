#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "gpio_if.h"

#define HDF_LOG_TAG state_user_driver

/* Command codes shared with user-space app */
#define STATE_CMD_SET     1   /* in: uint8 level (0 or 1) */

/* Heat/cool relay control pin: PB1 = group1*16 + pin1 = 17 */
#define STATE_GPIO_NUM    17

int32_t StateUserDispatch(struct HdfDeviceIoClient *client, int cmdCode,
                          struct HdfSBuf *data, struct HdfSBuf *reply)
{
    (void)client;
    switch (cmdCode) {
    case STATE_CMD_SET: {
        uint8_t level;
        if (!HdfSbufReadUint8(data, &level)) {
            return HDF_ERR_INVALID_PARAM;
        }
        uint16_t val = (level != 0) ? GPIO_VAL_HIGH : GPIO_VAL_LOW;
        if (GpioWrite(STATE_GPIO_NUM, val) != HDF_SUCCESS) {
            HDF_LOGE("%s: GpioWrite(%d) failed", __func__, STATE_GPIO_NUM);
            return HDF_FAILURE;
        }
        HdfSbufWriteInt32(reply, level);
        HDF_LOGD("%s: PB1 -> %u", __func__, level);
        break;
    }
    default:
        return HDF_ERR_NOT_SUPPORT;
    }
    return HDF_SUCCESS;
}

int32_t HdfStateUserBind(struct HdfDeviceObject *deviceObject)
{
    if (deviceObject == NULL) {
        return HDF_ERR_INVALID_OBJECT;
    }
    static struct IDeviceIoService stateUserService = {
        .Dispatch = StateUserDispatch,
    };
    deviceObject->service = (struct IDeviceIoService *)(&stateUserService);
    HDF_LOGD("state user driver bind success");
    return HDF_SUCCESS;
}

int32_t HdfStateUserInit(struct HdfDeviceObject *device)
{
    (void)device;
    int32_t ret = GpioSetDir(STATE_GPIO_NUM, GPIO_DIR_OUT);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: GpioSetDir(%d) failed:%d", __func__, STATE_GPIO_NUM, ret);
        return ret;
    }
    /* Default to HEAT (low), matching the original control logic */
    GpioWrite(STATE_GPIO_NUM, GPIO_VAL_LOW);
    HDF_LOGD("state user driver init ok (PB1, gpio %d)", STATE_GPIO_NUM);
    return HDF_SUCCESS;
}

void HdfStateUserRelease(struct HdfDeviceObject *deviceObject)
{
    (void)deviceObject;
    HDF_LOGD("state user driver release success");
}

struct HdfDriverEntry g_stateUserDriverEntry = {
    .moduleVersion = 1,
    .moduleName    = "HDF_STATE_USER",
    .Bind          = HdfStateUserBind,
    .Init          = HdfStateUserInit,
    .Release       = HdfStateUserRelease,
};

HDF_INIT(g_stateUserDriverEntry);
