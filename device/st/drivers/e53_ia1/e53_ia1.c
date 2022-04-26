/* 
 * Copyright (c) 2022 Nanjing Xiaoxiongpai Intelligent Technology CO., LIMITED.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "hdf_device_desc.h" 
#include "hdf_log.h"         
#include "device_resource_if.h"
#include "osal_io.h"
#include "osal_mem.h"
#include "gpio_if.h"
#include "osal_time.h"
#include "e53_ia1.h"
#include <math.h>      


#define HDF_LOG_TAG e53_driver // 打印日志所包含的标签，如果不定义则用默认定义的HDF_TAG标签

#define I2C_READ_MSG_NUM   2
#define I2C_WRITE_MSG_NUM  1

typedef enum {
    E53_IA1_Start = 0,
    E53_IA1_Read,
    E53_IA1_SetMotor,
    E53_IA1_SetLight,
}E53_IA1Ctrl;

struct Stm32Mp1e53 {
    uint32_t i2c_channel;
    uint32_t motor_gpio;
    uint32_t led_gpio;
};

static struct Stm32Mp1e53 g_Stm32Mp1e53;
E53I2cClient *bearpi_sht30_clint = NULL;
E53I2cClient *bearpi_bh175_clint = NULL;
static E53Sensor bearpi_e53_sensor ;


int32_t E53I2cWriteRead(const E53I2cClient *client, uint8_t *writeBuf, uint32_t writeLen, uint8_t *readBuf,
    uint32_t readLen)
{
    struct I2cMsg msg[I2C_READ_MSG_NUM];
    (void)memset_s(msg, sizeof(msg), 0, sizeof(msg));

    msg[0].addr = client->i2cCfg.addr;
    msg[0].flags = 0;
    msg[0].len = writeLen;
    msg[0].buf = writeBuf;

    msg[1].addr = client->i2cCfg.addr;
    msg[1].flags = I2C_FLAG_READ;
    msg[1].len = readLen;
    msg[1].buf = readBuf;

    if (I2cTransfer(client->i2cHandle, msg,I2C_READ_MSG_NUM) != I2C_READ_MSG_NUM) {
        HDF_LOGE("%s: i2c read err", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t E53I2cRead(const E53I2cClient *client, uint8_t *readBuf, uint32_t readLen)
{
    struct I2cMsg msg={0};
    msg.addr = client->i2cCfg.addr;
    msg.flags = I2C_FLAG_READ;
    msg.len = readLen;
    msg.buf = readBuf;

    if (I2cTransfer(client->i2cHandle, &msg,1) != 1) {
        HDF_LOGE("%s: i2c read err", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}

int32_t E53I2cWrite(const E53I2cClient *client, uint8_t *writeBuf, uint32_t len)
{
    struct I2cMsg msg[I2C_WRITE_MSG_NUM];
    (void)memset_s(msg, sizeof(msg), 0, sizeof(msg));

    msg[0].addr = client->i2cCfg.addr;
    msg[0].flags = 0;
    msg[0].len = len;
    msg[0].buf = writeBuf;

    if (I2cTransfer(client->i2cHandle, msg, I2C_WRITE_MSG_NUM) != I2C_WRITE_MSG_NUM) {
        HDF_LOGE("%s: i2c write err", __func__);
        return HDF_FAILURE;
    }
    return HDF_SUCCESS;
}


static E53I2cClient *E53DriverInstance(void)
{
    E53I2cClient *touchDrv = (E53I2cClient *)OsalMemAlloc(sizeof(E53I2cClient));
    if (touchDrv == NULL) {
        HDF_LOGE("%s: instance touch driver failed", __func__);
        return NULL;
    }
    (void)memset_s(touchDrv, sizeof(E53I2cClient), 0, sizeof(E53I2cClient));
    return touchDrv;
}

int32_t E53_IA1_Init(struct Stm32Mp1e53 *e53)
{
    int ret = 0;

    /* 将电机GPIO管脚配置为输出 */
    ret = GpioSetDir(e53->motor_gpio, GPIO_DIR_OUT);
    if (ret != 0)
    {
        HDF_LOGE("GpioSerDir: failed, ret %d\n", ret);
        return ret;
    }
    /* 将LED GPIO管脚配置为输出 */
    ret = GpioSetDir(e53->led_gpio, GPIO_DIR_OUT);
    if (ret != 0)
    {
        HDF_LOGE("GpioSerDir: failed, ret %d\n", ret);
        return ret;
    }

    bearpi_sht30_clint = E53DriverInstance();
    if(bearpi_sht30_clint == NULL){
        HDF_LOGI("bearpi_sht30_clint error!!!");
    }

    bearpi_bh175_clint = E53DriverInstance();
    if(bearpi_bh175_clint == NULL){
        HDF_LOGI("bearpi_bh175_clint error!!!");
    }


    bearpi_sht30_clint->i2cCfg.busNum = e53->i2c_channel;
    bearpi_sht30_clint->i2cCfg.addr = SHT30_ADDR_WRITE;
    bearpi_sht30_clint->i2cHandle = I2cOpen(bearpi_sht30_clint->i2cCfg.busNum);
    if(bearpi_sht30_clint->i2cHandle == NULL)
    {
        HDF_LOGI("bearpi_sht30_clint I2cOpen error!!!");
        return 0;
    }
    dprintf("\r\n bearpi_sht30_clint I2cOpen success!!! \r\n");


    bearpi_bh175_clint->i2cCfg.busNum = e53->i2c_channel;
    bearpi_bh175_clint->i2cCfg.addr = BH1750_ADDR_WRITE;
    bearpi_bh175_clint->i2cHandle = I2cOpen(bearpi_bh175_clint->i2cCfg.busNum);
    if(bearpi_bh175_clint->i2cHandle == NULL)
    {
        HDF_LOGI("bearpi_bh175_clint I2cOpen error!!!");
        return 0;
    }
    dprintf("\r\n bearpi_bh175_clint I2cOpen success!!! \r\n");


    return 1;
}


void GetE53IA1Data(E53Sensor* e53_sensor)
{
    uint8_t rcv_buf[8] = {0};
    uint8_t send_buf[2] = {0};

    send_buf[0] = 0x20;
    E53I2cWrite(bearpi_bh175_clint, send_buf, 1);
    (void)memset_s(rcv_buf, sizeof(rcv_buf), 0, sizeof(rcv_buf));
    E53I2cRead(bearpi_bh175_clint, rcv_buf,2);
    HDF_LOGI("%x %x ---%d", rcv_buf[0], rcv_buf[1], (int)(((rcv_buf[0]<<8) |rcv_buf[1])/1.2));
    e53_sensor->light = ((rcv_buf[0]<<8) |rcv_buf[1])/1.2;
    HDF_LOGI("bearpi_bh175_clint e53_sensor.light = %.2f", e53_sensor->light);

    //初始化SHT30
    send_buf[0] = 0x2C;
    send_buf[1] = 0x0D;

    E53I2cWrite(bearpi_sht30_clint, send_buf, 2);
    (void)memset_s(rcv_buf, sizeof(rcv_buf), 0, sizeof(rcv_buf));
    E53I2cRead(bearpi_sht30_clint, rcv_buf,6);
    HDF_LOGI("%x %x %x %x %x %x", rcv_buf[0], rcv_buf[1], rcv_buf[2], rcv_buf[3], rcv_buf[4], rcv_buf[5]);
    e53_sensor->temperature = (float)((rcv_buf[0]<<8) | rcv_buf[1]) /65535 * 175 -45;
    e53_sensor->humidity = 100 * ((rcv_buf[3]<<8) | rcv_buf[4]) /65535;
    HDF_LOGI("bearpi_sht30_clint temperature: %.2f humidity: %.2f", e53_sensor->temperature, e53_sensor->humidity);

}



// Dispatch是用来处理用户态发下来的消息
int32_t E53DriverDispatch(struct HdfDeviceIoClient *client, int cmdCode, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    // uint8_t contrl;
    int ret;
    HDF_LOGE("E53 driver dispatch");
    if (client == NULL || client->device == NULL)
    {
        HDF_LOGE("E53 driver device is NULL");
        return HDF_ERR_INVALID_OBJECT;
    }

    switch (cmdCode)
    {
    case E53_IA1_Read:
        GetE53IA1Data(&bearpi_e53_sensor);
        char*replay_buf =  OsalMemAlloc(1000);
        (void)memset_s(replay_buf, 1000, 0, 1000);
        sprintf(replay_buf,"Lux:%d\n\
                            Hum:%d\n\
                            Tem:%d\n",\
                            (int)bearpi_e53_sensor.light,           \
                            (int)bearpi_e53_sensor.humidity,      \
                            (int)bearpi_e53_sensor.temperature);
        /* 把传感器数据写入reply, 可被带至用户程序 */
        if (!HdfSbufWriteString(reply, replay_buf))                
        {
            HDF_LOGE("replay is fail");
            return HDF_FAILURE;
        }
        OsalMemFree(replay_buf);
        break;
    case E53_IA1_SetMotor:
        const char* read = HdfSbufReadString(data);
        if(strcmp(read,"ON") == 0){
            GpioWrite(g_Stm32Mp1e53.motor_gpio, GPIO_VAL_HIGH);
        }else if(strcmp(read,"OFF") == 0){
            GpioWrite(g_Stm32Mp1e53.motor_gpio, GPIO_VAL_LOW);
        }else{
            HDF_LOGE("Command wrong!");
            return HDF_FAILURE;
        }            

        ret = HdfSbufWriteString(reply, "E53 IA1 set motor successful");
        if(ret ==0)
        {
            HDF_LOGE("replay is fail");
            return HDF_FAILURE;
        }
        break;
    case E53_IA1_SetLight:
        const char* readdata = HdfSbufReadString(data);
        if(strcmp(readdata,"ON") == 0){
            GpioWrite(g_Stm32Mp1e53.led_gpio, GPIO_VAL_HIGH);
        }else if(strcmp(readdata,"OFF") == 0){
            GpioWrite(g_Stm32Mp1e53.led_gpio, GPIO_VAL_LOW);
        }else{
            HDF_LOGE("Command wrong!");
            return HDF_FAILURE;
        }            
        ret = HdfSbufWriteString(reply, "E53_IA1 set light successful");
        if(ret ==0)
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
static int32_t HdfE53IA1ReadDrs(struct Stm32Mp1e53 *e53, const struct DeviceResourceNode *node)
{
    int32_t ret;
    struct DeviceResourceIface *drsOps = NULL;

    drsOps = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (drsOps == NULL || drsOps->GetUint32 == NULL) {
        HDF_LOGE("%s: invalid drs ops!", __func__);
        return HDF_FAILURE;
    }
    /* 读取led.hcs里面led_gpio_num的值 */
    ret = drsOps->GetUint32(node, "i2c_channel", &e53->i2c_channel, 0); 
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read e53 i2c_channel fail!", __func__);
        return ret;
    }

    ret = drsOps->GetUint32(node, "motor_gpio_num", &e53->motor_gpio, 0); 
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read e53 i2c_channel fail!", __func__);
        return ret;
    }

    ret = drsOps->GetUint32(node, "led_gpio_num", &e53->led_gpio, 0); 
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: read e53 i2c_channel fail!", __func__);
        return ret;
    }

    return HDF_SUCCESS;
}

//驱动对外提供的服务能力，将相关的服务接口绑定到HDF框架
int32_t HdfE53IA1DriverBind(struct HdfDeviceObject *deviceObject)
{
    if (deviceObject == NULL)
    {
        HDF_LOGE("e53 driver bind failed!");
        return HDF_ERR_INVALID_OBJECT;
    }
    static struct IDeviceIoService e53Driver = { 
        .Dispatch = E53DriverDispatch,
    };
    deviceObject->service = (struct IDeviceIoService *)(&e53Driver);
    HDF_LOGD("E53 driver bind success");
    return HDF_SUCCESS;
}

// 驱动自身业务初始的接口
int32_t HdfE53IA1DriverInit(struct HdfDeviceObject *device)
{
    struct Stm32Mp1e53 *e53 = &g_Stm32Mp1e53;
    int32_t ret;

    if (device == NULL || device->property == NULL) {
        HDF_LOGE("%s: device or property NULL!", __func__);
        return HDF_ERR_INVALID_OBJECT;
    }
    /* 读取hcs私有属性值 */
    ret = HdfE53IA1ReadDrs(e53, device->property);
    if (ret != HDF_SUCCESS) {
        HDF_LOGE("%s: get led device resource fail:%d", __func__, ret);
        return ret;
    }

    //初始化
    E53_IA1_Init(e53);
    dprintf("\r\n ********************e53 driver Init success *******************\r\n");
    return HDF_SUCCESS;
}

// 驱动资源释放的接口
void HdfE53IA1DriverRelease(struct HdfDeviceObject *deviceObject)
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
struct HdfDriverEntry g_E53DriverEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_E53_IA1",
    .Bind = HdfE53IA1DriverBind,
    .Init = HdfE53IA1DriverInit,
    .Release = HdfE53IA1DriverRelease,
};

// 调用HDF_INIT将驱动入口注册到HDF框架中
HDF_INIT(g_E53DriverEntry);
