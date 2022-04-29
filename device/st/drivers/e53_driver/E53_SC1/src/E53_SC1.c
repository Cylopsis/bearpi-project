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

#include "E53_SC1.h"
#include "E53_Common.h"
#include "osal_time.h"

#define E53_SC1_Light_GPIO  E53_IO_5

/***************************************************************
 * 函数名称: Init_BH1750
 * 说    明: 写命令初始化BH1750
 * 参    数: 无
 * 返 回 值: 0 成功; -1 失败
 ***************************************************************/
static int InitBH1750(void)
{
    int ret;
    uint8_t send_data[1] = { 0x01 };
    ret = E53_IICWrite((BH1750_ADDR << 1), send_data, 1);
    if (ret != 0) {
        return -1;
    }
    return 0;
}

/***************************************************************
 * 函数名称: Start_BH1750
 * 说    明: 启动BH1750
 * 参    数: 无
 * 返 回 值: 0 成功; -1 失败
 ***************************************************************/
static int StartBH1750(void)
{
    int ret;
    uint8_t send_data[1] = { 0x20 };
    ret = E53_IICWrite((BH1750_ADDR << 1), send_data, 1);
    if (ret != 0) {
        return -1;
    }
    return 0;
}
/***************************************************************
 * 函数名称: E53SC1Init
 * 说    明: 初始化E53_SC1
 * 参    数: 无
 * 返 回 值: 0 成功; -1 失败
 ***************************************************************/
int E53_SC1Init(void)
{
    int ret;

    E53_GPIOInit(E53_SC1_Light_GPIO, E53_GPIO_Out_PullNone);
    E53_IICOpen();
    
    ret = InitBH1750();
    if (ret != 0) {
        return -1;
    }
    return 0;
}
/***************************************************************
 * 函数名称: E53SC1DeInit
 * 说    明: 初始化E53_SC1
 * 参    数: 无
 * 返 回 值: 0 成功; -1 失败
 ***************************************************************/
int E53_SC1DeInit(void)
{
    int ret;
    E53_GPIODeinit(E53_SC1_Light_GPIO);
    E53_IICClose();
    
    ret = InitBH1750();
    if (ret != 0) {
        return -1;
    }
    return 0;
}
/***************************************************************
 * 函数名称: E53_SC1_Read_Data
 * 说    明: 测量光照强度
 * 参    数: data,光照强度数据指针
 * 返 回 值: 0 成功; -1 失败
 ***************************************************************/
int E53_SC1ReadData(float *data)
{
    int ret;
    ret = StartBH1750(); // 启动传感器采集数据
    if (ret != 0) {
        return -1;
    }
    uint8_t recv_data[BH1750_RECV_DATA_LEN] = { 0 };
    ret = E53_IICRead((BH1750_ADDR << 1), recv_data, sizeof(recv_data)); // 读取传感器数据

    if (ret != 0) {
        return -1;
    }
    *data = (float)(((recv_data[0] << DATA_WIDTH_8_BIT) + recv_data[1]) / BH1750_COEFFICIENT_LUX); // 合成数据，即光照数据
    return 0;
}
/***************************************************************
 * 函数名称: LightStatusSet
 * 说    明: 灯状态设置
 * 参    数: status,ENUM枚举的数据
 *									OFF,光灯
 *									ON,开灯
 * 返 回 值: 无
 ***************************************************************/
void E53_SC1LightStatusSet(E53SC1Status status)
{
    if (status == ON) {
        E53_GPIOWrite(E53_SC1_Light_GPIO,1); // 设置GPIO输出高电平点亮LED灯
    }

    if (status == OFF) {
        E53_GPIOWrite(E53_SC1_Light_GPIO,0); // 设置GPIO输出低电平点亮LED灯
    }
}
