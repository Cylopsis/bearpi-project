/*
 * Copyright (c) 2022 Nanjing Xiaoxiongpai Intelligent Technology Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include "E53_SC2.h"
#include "E53_Common.h"
#include "osal_time.h"

#define E53_SC2_Light1_GPIO  E53_IO_5
#define E53_SC2_Light2_GPIO  E53_IO_6

/***************************************************************
 * 函数功能: 通过I2C写入一个值到指定寄存器内
 * 输入参数: Addr：I2C设备地址
 *           Reg：目标寄存器
 *           Value：值
 * 返 回 值: 无
 * 说    明: 无
 **************************************************************/
static int MPU6050WriteData(uint8_t Reg, uint8_t Value)
{
    uint32_t ret;
    uint8_t send_data[MPU6050_DATA_2_BYTE] = { Reg, Value };

    ret = E53_IICWrite((MPU6050_ADDRESS << 1), send_data, sizeof(send_data));
    if (ret != 0) {
        dprintf("[%s][%d]\r\n",__func__,__LINE__);
        return -1;
    }
    return 0;
}

/***************************************************************
 * 函数功能: 通过I2C读取一段寄存器内容存放到指定的缓冲区内
 * 输入参数: Addr：I2C设备地址
 *           Reg：目标寄存器
 *           RegSize：寄存器尺寸(8位或者16位)
 *           pBuffer：缓冲区指针
 *           Length：缓冲区长度
 * 返 回 值: HAL_StatusTypeDef：操作结果
 * 说    明: 无
 **************************************************************/
static int MPU6050ReadBuffer(uint8_t Reg, uint8_t *pBuffer, uint16_t Length)
{
    uint32_t ret = 0;

    uint8_t buffer[1] = { Reg };
    ret = E53_IICWrite((MPU6050_ADDRESS << 1),buffer,1);
    if(ret != 0){
        dprintf("[%s][%d]\r\n",__func__,__LINE__);
        return -1;
    }
    ret = E53_IICRead((MPU6050_ADDRESS << 1),pBuffer,Length);
    for(int i = 0;i < Length;i++)
    {
        dprintf("read:%d\n",pBuffer[i]);
    }
    dprintf("debug!\n");
    if(ret != 0){
        dprintf("[%s][%d]\r\n",__func__,__LINE__);
        return -1;
    }
    return 0;
}
/***************************************************************
 * 函数功能: 写数据到MPU6050寄存器
 * 输入参数: 无
 * 返 回 值: 无
 * 说    明: 无
 ***************************************************************/
static void MPU6050WriteReg(uint8_t reg_add, uint8_t reg_dat)
{
    MPU6050WriteData(reg_add, reg_dat);
}

/***************************************************************
 * 函数功能: 从MPU6050寄存器读取数据
 * 输入参数: 无
 * 返 回 值: 无
 * 说    明: 无
 ***************************************************************/
static int MPU6050ReadData(uint8_t reg_add, unsigned char *read, uint8_t num)
{
    return MPU6050ReadBuffer(reg_add, read, num);
}

/***************************************************************
 * 函数功能: 读取MPU6050的加速度数据
 * 输入参数: 无
 * 返 回 值: 无
 * 说    明: 无
 ***************************************************************/
static int MPU6050ReadAcc(short *accData)
{
    int ret;
    uint8_t buf[ACCEL_DATA_LEN];
    ret = MPU6050ReadData(MPU6050_ACC_OUT, buf, ACCEL_DATA_LEN);
    if (ret != 0) {
        return -1;
    }
    accData[ACCEL_X_AXIS] = (buf[ACCEL_X_AXIS_LSB] << SENSOR_DATA_WIDTH_8_BIT) | buf[ACCEL_X_AXIS_MSB];
    accData[ACCEL_Y_AXIS] = (buf[ACCEL_Y_AXIS_LSB] << SENSOR_DATA_WIDTH_8_BIT) | buf[ACCEL_Y_AXIS_MSB];
    accData[ACCEL_Z_AXIS] = (buf[ACCEL_Z_AXIS_LSB] << SENSOR_DATA_WIDTH_8_BIT) | buf[ACCEL_Z_AXIS_MSB];
    return 0;
}


/***************************************************************
 * 函数功能: 读取MPU6050的温度数据，转化成摄氏度
 * 输入参数: 无
 * 返 回 值: 无
 * 说    明: 无
 **************************************************************/
static int MPU6050ReturnTemp(short *Temperature)
{
    int ret;
    short temp3;
    uint8_t buf[TEMP_DATA_LEN];

    ret = MPU6050ReadData(MPU6050_RA_TEMP_OUT_H, buf, TEMP_DATA_LEN); // 读取温度值
    if (ret != 0) {
        return -1;
    }
    temp3 = (buf[TEMP_LSB] << SENSOR_DATA_WIDTH_8_BIT) | buf[TEMP_MSB];
    *Temperature = (((double)(temp3 + MPU6050_CONSTANT_1)) / MPU6050_CONSTANT_2) - MPU6050_CONSTANT_3;
    return 0;
}

/***************************************************************
 * 函数功能: 自由落体中断
 * 输入参数: 无
 * 返 回 值: 无
 * 说    明: 无
 **************************************************************/
void FreeFallInterrupt(void) // 自由落体中断
{
    MPU6050WriteReg(MPU6050_RA_FF_THR, 0x01); // 自由落体阈值
    MPU6050WriteReg(MPU6050_RA_FF_DUR, 0x01); // 自由落体检测时间20ms 单位1ms 寄存器0X20
}
void MotionInterrupt(void) // 运动中断
{
    MPU6050WriteReg(MPU6050_RA_MOT_THR, 0x03); // 运动阈值
    MPU6050WriteReg(MPU6050_RA_MOT_DUR, 0x14); // 检测时间20ms 单位1ms 寄存器0X20
}
void ZeroMotionInterrupt(void) // 静止中断
{
    MPU6050WriteReg(MPU6050_RA_ZRMOT_THR, 0x20); // 静止阈值
    MPU6050WriteReg(MPU6050_RA_ZRMOT_DUR, 0x20); // 静止检测时间20ms 单位1ms 寄存器0X20
}

/***************************************************************
 * 函数功能: 初始化MPU6050芯片
 * 输入参数: 无
 * 返 回 值: 无
 * 说    明: 无
 ***************************************************************/
void MPU6050Init(void)
{
    MPU6050WriteReg(MPU6050_RA_PWR_MGMT_1, 0X80); // 复位MPU6050

    OsalMDelay(20);
    MPU6050WriteReg(MPU6050_RA_PWR_MGMT_1, 0X00); // 唤醒MPU6050
    MPU6050WriteReg(MPU6050_RA_INT_ENABLE, 0X00); // 关闭所有中断
    MPU6050WriteReg(MPU6050_RA_USER_CTRL, 0X00);  // I2C主模式关闭
    MPU6050WriteReg(MPU6050_RA_FIFO_EN, 0X00);    // 关闭FIFO
    MPU6050WriteReg(MPU6050_RA_INT_PIN_CFG,
        0X80); // 中断的逻辑电平模式,设置为0，中断信号为高电；设置为1，中断信号为低电平时。
    MotionInterrupt();                              // 运动中断
    MPU6050WriteReg(MPU6050_RA_CONFIG, 0x04);       // 配置外部引脚采样和DLPF数字低通滤波器
    MPU6050WriteReg(MPU6050_RA_ACCEL_CONFIG, 0x1C); // 加速度传感器量程和高通滤波器配置
    MPU6050WriteReg(MPU6050_RA_INT_PIN_CFG, 0X1C);  // INT引脚低电平平时
    MPU6050WriteReg(MPU6050_RA_INT_ENABLE, 0x40);   // 中断使能寄存器
}

/***************************************************************
 * 函数功能: 读取MPU6050的ID
 * 输入参数: 无
 * 返 回 值: 无
 * 说    明: 无
 ***************************************************************/
int MPU6050ReadID(void)
{
    unsigned char Re = 0;
    MPU6050ReadData(MPU6050_RA_WHO_AM_I, &Re, 1); // 读器件地址
    if (Re != 0x68) {
        dprintf("[%s][%d]:%d\r\n",__func__,__LINE__,Re);
        return -1;
    } else {
        return 0;
    }
}
/***************************************************************
 * 函数名称: E53_SC2Init
 * 说    明: 初始化E53_SC2
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
int E53_SC2Init(void)
{
    uint32_t ret = 0;
    E53_GPIOInit(E53_SC2_Light1_GPIO,E53_GPIO_Out_PullNone);
    E53_GPIOInit(E53_SC2_Light2_GPIO,E53_GPIO_Out_PullNone);
    E53_IICOpen();
    
    MPU6050Init();
    ret = MPU6050ReadID();
    if (ret != 0) {
        return -1;
    }

    OsalMDelay(20);

    return 0;
}
/***************************************************************
 * 函数名称: E53_SC2DeInit
 * 说    明: 初始化E53_SC2
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
int E53_SC2DeInit(void)
{
    E53_GPIODeinit(E53_SC2_Light1_GPIO);
    E53_GPIODeinit(E53_SC2_Light2_GPIO);
    E53_IICClose();

    return 0;
}
/***************************************************************
 * 函数名称: E53_SC2ReadData
 * 说    明: 读取数据
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
int E53_SC2ReadData(E53_SC2Data *ReadData)
{
    int ret;
    short Accel[ACCEL_AXIS_NUM];
    short Temp;
    if (MPU6050ReadID() != 0) {
        return -1;
    }
    ret = MPU6050ReadAcc(Accel);
    if (ret != 0) {
        return -1;
    }

    ret = MPU6050ReturnTemp(&Temp);
    if (ret != 0) {
        return -1;
    }
    ReadData->Temperature = Temp;
    ReadData->Accel[ACCEL_X_AXIS] = Accel[ACCEL_X_AXIS];
    ReadData->Accel[ACCEL_Y_AXIS] = Accel[ACCEL_Y_AXIS];
    ReadData->Accel[ACCEL_Z_AXIS] = Accel[ACCEL_Z_AXIS];
    OsalMDelay(20);

    return 0;
}

/***************************************************************
 * 函数名称: LedD1StatusSet
 * 说    明: LED_D1状态设置
 * 参    数: status,ENUM枚举的数据
 *									OFF,关
 *									ON,开
 * 返 回 值: 无
 ***************************************************************/
void E53_SC2LedD1StatusSet(E53_SC2Status status)
{
    if (status == ON) {
        E53_GPIOWrite(E53_SC2_Light1_GPIO,1); // 设置GPIO输出高电平点亮LED灯
    }

    if (status == OFF) {
        E53_GPIOWrite(E53_SC2_Light1_GPIO,0); // 设置GPIO输出低电平点亮LED灯
    }
}

/***************************************************************
 * 函数名称: LedD2StatusSet
 * 说    明: LED_D2状态设置
 * 参    数: status,ENUM枚举的数据
 *									OFF,关
 *									ON,开
 * 返 回 值: 无
 ***************************************************************/
void E53_SC2LedD2StatusSet(E53_SC2Status status)
{
    if (status == ON) {
        E53_GPIOWrite(E53_SC2_Light2_GPIO,1); // 设置GPIO输出高电平点亮LED灯
    }

    if (status == OFF) {
        E53_GPIOWrite(E53_SC2_Light2_GPIO,0); // 设置GPIO输出低电平点亮LED灯
    }
}
