#include "hdf_device_desc.h" 
#include "hdf_log.h"         
#include "device_resource_if.h"
#include "osal_io.h"
#include "osal.h"
#include "osal_mem.h"
#include "gpio_if.h"

#define HDF_LOG_TAG led_driver // 打印日志所包含的标签，如果不定义则用默认定义的HDF_TAG标签
#define LED_WRITE_READ 1       // 读写操作码1

enum LedOps {
    LED_OFF,
    LED_ON,  
    LED_TOGGLE,
};

struct Stm32Mp1ILed {
    uint32_t gpioNum;
};
static struct Stm32Mp1ILed g_Stm32Mp1ILed;
uint8_t status = 0;
// Dispatch是用来处理用户态发下来的消息
int32_t LedDriverDispatch(struct HdfDeviceIoClient *client, int cmdCode, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    uint8_t contrl;
    HDF_LOGE("Led driver dispatch");
    if (client == NULL || client->device == NULL)
    {
        HDF_LOGE("Led driver device is NULL");
        return HDF_ERR_INVALID_OBJECT;
    }

    switch (cmdCode)
    {
    /* 接收到用户态发来的LED_WRITE_READ命令 */
    case LED_WRITE_READ:
        /* 读取data里的数据，赋值给contrl */
        HdfSbufReadUint8(data,&contrl);                  
        switch (contrl)
        {
        /* 开灯 */
        case LED_ON:                                            
            GpioWrite(g_Stm32Mp1ILed.gpioNum, GPIO_VAL_LOW);
            status = 1;
            break;
        /* 关灯 */
        case LED_OFF:                                           
            GpioWrite(g_Stm32Mp1ILed.gpioNum, GPIO_VAL_HIGH);
            status = 0;
            break;
        /* 状态翻转 */
        case LED_TOGGLE:
            if(status == 0)
            {
                GpioWrite(g_Stm32Mp1ILed.gpioNum, GPIO_VAL_LOW);
                status = 1;
            }
            else
            {
                GpioWrite(g_Stm32Mp1ILed.gpioNum, GPIO_VAL_HIGH);
                status = 0;
            }                                        
            break;
        default:
            break;
        }
        /* 把LED的状态值写入reply, 可被带至用户程序 */
        if (!HdfSbufWriteInt32(reply, status))                
        {
            HDF_LOGE("replay is fail");
            return HDF_FAILURE;
        }
        break;
    default:
        break;
    }
    return HDF_SUCCESS;
}

// 读取驱动私有配置
static int32_t Stm32LedReadDrs(struct Stm32Mp1ILed *led, const struct DeviceResourceNode *node)
{
    int32_t ret;
    struct DeviceResourceIface *drsOps = NULL;

    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetUint32 == NULL) {
        HDF_LOGE("%s: invalid drs ops!", __func__);
        return HDF_FAILURE;
    }
    /* 读取led.hcs里面led_gpio_num的值 */
    ret = drsOps->GetUint32(node, "led_gpio_num", &led->gpioNum, 0); 
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read led gpio num fail!", __func__);
        return ret;
    }
    return HDF_SUCCESS;
}

//驱动对外提供的服务能力，将相关的服务接口绑定到HDF框架
int32_t HdfLedDriverBind(struct HdfDeviceObject *deviceObject)
{
    if (deviceObject == NULL)
    {
        HDF_LOGE("Led driver bind failed!");
        return HDF_ERR_INVALID_OBJECT;
    }
    static struct IDeviceIoService ledDriver = {
        .Dispatch = LedDriverDispatch,
    };
    deviceObject->service = (struct IDeviceIoService *)(&ledDriver);
    HDF_LOGD("Led driver bind success");
    return HDF_SUCCESS;
}

// 驱动自身业务初始的接口
int32_t HdfLedDriverInit(struct HdfDeviceObject *device)
{
    struct Stm32Mp1ILed *led = &g_Stm32Mp1ILed;
    int32_t ret;

    if (device == NULL || device->property == NULL) {
        HDF_LOGE("%s: device or property NULL!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    /* 读取hcs私有属性值 */
    ret = Stm32LedReadDrs(led, device->property);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: get led device resource fail:%d", __func__, ret);
        return ret;
    }
    /* 将GPIO管脚配置为输出 */
    ret = GpioSetDir(led->gpioNum, GPIO_DIR_OUT);
    if (ret != 0)
    {
        HDF_LOGE("GpioSerDir: failed, ret %d\n", ret);
        return ret;
    }
    HDF_LOGD("Led driver Init success");
    return HDF_SUCCESS;
}

// 驱动资源释放的接口
void HdfLedDriverRelease(struct HdfDeviceObject *deviceObject)
{
    if (deviceObject == NULL)
    {
        HDF_LOGE("Led driver release failed!");
        return;
    }
    HDF_LOGD("Led driver release success");
    return;
}

// 定义驱动入口的对象，必须为HdfDriverEntry（在hdf_device_desc.h中定义）类型的全局变量
struct HdfDriverEntry g_ledDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_LED",
    .Bind = HdfLedDriverBind,
    .Init = HdfLedDriverInit,
    .Release = HdfLedDriverRelease,
};

// 调用HDF_INIT将驱动入口注册到HDF框架中
HDF_INIT(g_ledDriverEntry);


