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

#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include "osal_mem.h"
#include "osal_time.h"

#include "E53_IA1.h"
#include "E53_Common.h"

#define E53_IA1_MOTOR_GPIO  E53_IO_6
#define E53_IA1_LIGHT_GPIO  E53_IO_2


/***************************************************************
* 函数名称: Init_BH1750
* 说    明: 写命令初始化BH1750
* 参    数: 无
* 返 回 值: 无
***************************************************************/
int Init_BH1750(void)
{
    uint8_t send_data = 0x01;
	int ret;
    ret = E53_IICWrite((BH1750_Addr<<1),&send_data,1); 
    return ret;
}

/***************************************************************
* 函数名称: Start_BH1750
* 说    明: 启动BH1750
* 参    数: 无
* 返 回 值: 无
***************************************************************/
int Start_BH1750(void)
{
    uint8_t send_data[1] = { 0x20 };
	int ret;
    ret = E53_IICWrite((BH1750_Addr<<1),send_data,1);
    return ret; 
}

/***************************************************************
* 函数名称: SHT30_reset
* 说    明: SHT30复位
* 参    数: 无
* 返 回 值: 无
***************************************************************/
int SHT30_reset(void)
{
    uint8_t send_data[2] = { 0x30,0xA2 };
	int ret;
    ret = E53_IICWrite((SHT30_Addr<<1),send_data,2); 
    return ret;
}

/***************************************************************
* 函数名称: Init_SHT30
* 说    明: 初始化SHT30，设置测量周期
* 参    数: 无
* 返 回 值: 无
***************************************************************/
int Init_SHT30(void)
{
    uint8_t send_data[2] = { 0x22,0x36 };
    int ret;
    ret = E53_IICWrite((SHT30_Addr<<1),send_data,2);
    return ret;
}

/***************************************************************
* 函数名称: SHT3x_CheckCrc
* 说    明: 检查数据正确性
* 参    数: data：读取到的数据
						nbrOfBytes：需要校验的数量
						checksum：读取到的校对比验值
* 返 回 值: 校验结果，0-成功		1-失败
***************************************************************/
static uint8_t SHT3x_CheckCrc(uint8_t data[], uint8_t nbrOfBytes, uint8_t checksum)
{
	
    uint8_t crc = 0xFF;
    uint8_t bit = 0;
    uint8_t byteCtr ;
	const int16_t POLYNOMIAL = 0x131;
    //calculates 8-Bit checksum with given polynomial
    for(byteCtr = 0; byteCtr < nbrOfBytes; ++byteCtr)
    {
        crc ^= (data[byteCtr]);
        for ( bit = 8; bit > 0; --bit)
        {
            if (crc & 0x80) crc = (crc << 1) ^ POLYNOMIAL;
            else crc = (crc << 1);
        }
    }
	
    if(crc != checksum)
		return 1;
    else
		return 0;
	
}

/***************************************************************
* 函数名称: SHT3x_CalcTemperatureC
* 说    明: 温度计算
* 参    数: u16sT：读取到的温度原始数据
* 返 回 值: 计算后的温度数据
***************************************************************/
static float SHT3x_CalcTemperatureC(uint16_t u16sT)
{
	
    float temperatureC = 0;            // variable for result

    u16sT &= ~0x0003;           // clear bits [1..0] (status bits)
    //-- calculate temperature [℃] --
    temperatureC = (175 * (float)u16sT / 65535 - 45); //T = -45 + 175 * rawValue / (2^16-1)
	
    return temperatureC;
}

/***************************************************************
* 函数名称: SHT3x_CalcRH
* 说    明: 湿度计算
* 参    数: u16sRH：读取到的湿度原始数据
* 返 回 值: 计算后的湿度数据
***************************************************************/
static float SHT3x_CalcRH(uint16_t u16sRH)
{
	
    float humidityRH = 0;              // variable for result
	
    u16sRH &= ~0x0003;          // clear bits [1..0] (status bits)
    //-- calculate relative humidity [%RH] --
    humidityRH = (100 * (float)u16sRH / 65535);  // RH = rawValue / (2^16-1) * 10
	
    return humidityRH;
}

/***************************************************************
* 函数名称: E53_IA1_Init
* 说    明: 初始化E53_IA1
* 参    数: 无
* 返 回 值: 无
***************************************************************/
int E53_IA1_Init(void)
{
    E53_GPIOInit(E53_IA1_LIGHT_GPIO,E53_GPIO_Out_PullNone);
    E53_GPIOInit(E53_IA1_MOTOR_GPIO,E53_GPIO_Out_PullNone);
    E53_IICOpen();
    
    int ret0, ret1;
	ret0 = Init_BH1750();
    ret1 = Init_SHT30();
    OsalMDelay(180);
    return ret0 | ret1;
}

/***************************************************************
* 函数名称: E53_IA1_DeInit
* 说    明: 初始化E53_IA1
* 参    数: 无
* 返 回 值: 无
***************************************************************/
int E53_IA1_DeInit(void)
{
    E53_GPIODeinit(E53_IA1_LIGHT_GPIO);
    E53_GPIODeinit(E53_IA1_MOTOR_GPIO);
    E53_IICClose();
    return 0;
}


/***************************************************************
* 函数名称: E53_IA1_Read_Data
* 说    明: 测量光照强度、温度、湿度
* 参    数: 无
* 返 回 值: 无
***************************************************************/
int E53_IA1_Read_Data(void)
{
    Start_BH1750(); // 启动传感器采集数据

    uint8_t recv_data[2] = { 0 };
	int ret;
    ret = E53_IICRead( (BH1750_Addr<<1),recv_data,2);   // 读取传感器数据

    if(ret != 0){
        return ret;
    }
	E53_IA1_Data.Lux = (float)(((recv_data[0]<<8) + recv_data[1])/1.2);   


    //初始化SHT30
    uint8_t send_data[2] = { 0xE0,0x00};
    uint8_t rcv_buf[8]={0};
    ret = E53_IICWrite((SHT30_Addr<<1), send_data, 2);
    E53_IICRead((SHT30_Addr<<1), rcv_buf,6);

    uint8_t  data[3];    //data array for checksum verification
    uint16_t dat,tmp;
    //    /* check tem */
    data[0] = rcv_buf[0];
    data[1] = rcv_buf[1];
    data[2] = rcv_buf[2];
    tmp=SHT3x_CheckCrc(data, 2, data[2]);
    if( !tmp ) /* value is ture */
    {
        dat = ((uint16_t)data[0] << 8) | data[1];
        E53_IA1_Data.Temperature = SHT3x_CalcTemperatureC( dat );    
    }
    //    /* check humidity */
    data[0] = rcv_buf[3];
    data[1] = rcv_buf[4];
    data[2] = rcv_buf[5];
    tmp=SHT3x_CheckCrc(data, 2, data[2]);
    if( !tmp ) /* value is ture */
    {
        dat = ((uint16_t)data[0] << 8) | data[1];
        E53_IA1_Data.Humidity = SHT3x_CalcRH( dat );    
    }
    return 0;    
}

/***************************************************************
* 函数名称: Light_StatusSet
* 说    明: 灯状态设置
* 参    数: status,ENUM枚举的数据
*									OFF,关
*									ON,开
* 返 回 值: 无
***************************************************************/
int Light_StatusSet(E53_IA1_Status_ENUM status)
{
    int ret = 0;
	if(status == ON)
		ret = E53_GPIOWrite(E53_IA1_LIGHT_GPIO, 1);//设置输出高电平点亮灯
	if(status == OFF)
		ret = E53_GPIOWrite(E53_IA1_LIGHT_GPIO, 0);//设置输出低电平关闭灯
    return ret;
}

/***************************************************************
* 函数名称: Motor_StatusSet
* 说    明: 电机状态设置
* 参    数: status,ENUM枚举的数据
*									OFF,关
*									ON,开
* 返 回 值: 无
***************************************************************/
int Motor_StatusSet(E53_IA1_Status_ENUM status)
{
    int ret = 0;
	if(status == ON)
		ret = E53_GPIOWrite(E53_IA1_MOTOR_GPIO, 1);//设置输出高电平打开电机
	if(status == OFF)
		ret = E53_GPIOWrite(E53_IA1_MOTOR_GPIO, 0);//设置输出低电平关闭电机
    return ret;
}



