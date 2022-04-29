#include <stdint.h>
#include <string.h>

#include "E53_SC2.h"
#include "E53_Common.h"

#include "hdf_device_desc.h" 
#include "hdf_log.h"         
#include "device_resource_if.h"
#include "osal_io.h"
#include "osal_mem.h"
#include "gpio_if.h"
#include "osal_time.h"


static E53_SC2Data sc2Data;
static uint8_t Status;
static int X = 0, Y = 0, Z = 0;

typedef enum {
    E53_SC2_Start = 0,
    E53_SC2_Stop,
    E53_SC2_Read,
}E53_SC2Ctrl;

int32_t E53_SC2_DriverDispatch(struct HdfDeviceIoClient *client, int cmdCode, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    int ret = -1;
    char *replay_buf;

    HDF_LOGE("E53 driver dispatch");
    if (client == NULL || client->device == NULL)
    {
        HDF_LOGE("E53 driver device is NULL");
        return HDF_ERR_INVALID_OBJECT;
    }
    switch (cmdCode)
    {
    case E53_SC2_Start:
        ret = E53_SC2Init();
        if(ret != 0){
            HDF_LOGE("E53 SC2 Init err");
            return HDF_FAILURE;
        }
        ret = HdfSbufWriteString(reply, "E53 SC2 Init successful");
        if(ret ==0){
            HDF_LOGE("reply failed");
            return HDF_FAILURE;
        }
        break;
    case E53_SC2_Stop:
        ret = E53_SC2Init();
        if(ret != 0){
            HDF_LOGE("E53 SC2 DeInit err");
            return HDF_FAILURE;
        }
        ret = HdfSbufWriteString(reply, "E53 SC2 Init successful");
        if(ret ==0){
            HDF_LOGE("reply failed");
            return HDF_FAILURE;
        }
        break;
    /* 接收到用户态发来的LED_WRITE_READ命令 */
    case E53_SC2_Read:
        ret = E53_SC2ReadData(&sc2Data);
        if(ret != 0){
            HDF_LOGE("E53 SC2 Read Data err");
            return HDF_FAILURE;
        }
        HDF_LOGE("\r\n**************Temperature      is  %d\r\n", (int)sc2Data.Temperature);
        HDF_LOGE("\r\n**************Accel[0]         is  %d\r\n", (int)sc2Data.Accel[0]);
        HDF_LOGE("\r\n**************Accel[1]         is  %d\r\n", (int)sc2Data.Accel[1]);
        HDF_LOGE("\r\n**************Accel[2]         is  %d\r\n", (int)sc2Data.Accel[2]);
        if (X == 0 && Y == 0 && Z == 0) {
            X = (int)sc2Data.Accel[0];
            Y = (int)sc2Data.Accel[1];
            Z = (int)sc2Data.Accel[2];
        } else {
            if (X + 100 < sc2Data.Accel[0] || X - 100 > sc2Data.Accel[0] || Y + 100 < sc2Data.Accel[1] ||
                Y - 100 > sc2Data.Accel[1] || Z + 100 < sc2Data.Accel[2] || Z - 100 > sc2Data.Accel[2]) {
                E53_SC2LedD1StatusSet(0);
                E53_SC2LedD2StatusSet(1);
                Status = 1;

            } else {
                E53_SC2LedD1StatusSet(1);
                E53_SC2LedD2StatusSet(0);
                Status = 0;
            }
        }
        
        replay_buf = OsalMemAlloc(100);
        (void)memset_s(replay_buf, 100, 0, 100);
        sprintf(replay_buf,"{\"Temp\":%d,\"Status\":%d}",\
                            sc2Data.Temperature,\
                            Status);
        /* 把传感器数据写入reply, 可被带至用户程序 */
        if (!HdfSbufWriteString(reply, replay_buf))                
        {
            HDF_LOGE("replay is fail");
            return HDF_FAILURE;
        }
        OsalMemFree(replay_buf);
        break;
    default:
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}


//驱动对外提供的服务能力，将相关的服务接口绑定到HDF框架
static int32_t Hdf_E53_SC2_DriverBind(struct HdfDeviceObject *deviceObject)
{
    if (deviceObject == NULL)
    {
        HDF_LOGE("e53 driver bind failed!");
        return HDF_ERR_INVALID_OBJECT;
    }
    static struct IDeviceIoService e53Driver = { 
        .Dispatch = E53_SC2_DriverDispatch,
    };
    deviceObject->service = (struct IDeviceIoService *)(&e53Driver);
    HDF_LOGD("E53 driver bind success");
    return HDF_SUCCESS;
}


static int32_t Hdf_E53_SC2_DriverInit(struct HdfDeviceObject *device)
{
    return HDF_SUCCESS;
}

// 驱动资源释放的接口
void Hdf_E53_SC2_DriverRelease(struct HdfDeviceObject *deviceObject)
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
static struct HdfDriverEntry g_E53_SC2DriverEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_E53_SC2",
    .Bind = Hdf_E53_SC2_DriverBind,
    .Init = Hdf_E53_SC2_DriverInit,
    .Release = Hdf_E53_SC2_DriverRelease,
};

// 调用HDF_INIT将驱动入口注册到HDF框架中
HDF_INIT(g_E53_SC2DriverEntry);

