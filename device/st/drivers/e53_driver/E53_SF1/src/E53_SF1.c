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
#include "hdf_log.h"  


#include "E53_SF1.h"
#include "E53_Common.h"
#include "osal_time.h"
static float R0; // 元件在洁净空气中的阻值


int E53_SF1GetVoltage(float* vol)
{
    uint32_t value;
    E53_ADCRead(&value);
    HDF_LOGE("E53 driver dispatch");
    *vol = (float)(value * 3.3 / 255);
    HDF_LOGE("E53_SF1 GetVoltage : %.2f V", *vol);
    return 0;
}

/***************************************************************
 * 函数名称: E53SF1Init
 * 说    明: 初始化E53_SF1扩展板
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
int E53_SF1_Init()
{
    E53_PWMOpen(3);
    E53_PWMSetPeriod(4000);
    E53_PWMSetDuty(1);
    E53_PWMSetPolarity(0);
    E53_PWMStart();
    E53_ADCOpen();

    MQ2PPMCalibration();
    OsalMDelay(1000);
    return 0;
}
/***************************************************************
 * 函数名称: E53SF1DeInit
 * 说    明: 初始化E53_SF1扩展板
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
int E53_SF1_DeInit()
{
    E53_PWMClose();
    E53_ADCClose();
    return 0;
}

/***************************************************************
 * 函数名称: GetMQ2PPM
 * 说    明: 获取PPM函数
 * 参    数: ppm 烟雾浓度
 * 返 回 值: 0 成功; -1 失败
 ***************************************************************/
int GetMQ2PPM(float *ppm)
{
    unsigned int ret;
    // unsigned short data;
    float voltage, RS;
    ret = E53_SF1GetVoltage(&voltage);
    if(ret != 0){
        return -1;
    }
    RS = (MQ2_CONSTANT_1 - voltage) / voltage * RL;     // 计算RS
    *ppm = MQ2_CONSTANT_2 * pow(RS / R0, MQ2_CONSTANT_3); // 计算ppm
    return 0;
}

/***************************************************************
 * 函数名称: MQ2PPMCalibration
 * 说    明: 传感器校准函数
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
int MQ2PPMCalibration(void)
{
    int ret;
    float voltage;
    ret = E53_SF1GetVoltage(&voltage);
    if(ret != 0){
        return -1;
    }
    if (voltage != 0) {
        float RS = (MQ2_CONSTANT_1 - voltage) / voltage * RL;
        R0 = RS / pow(CAL_PPM / MQ2_CONSTANT_2, 1 / MQ2_CONSTANT_3);
    }
    return 0;
}

/***************************************************************
 * 函数名称: BeepStatusSet
 * 说    明: 蜂鸣器报警与否
 * 参    数: status,ENUM枚举的数据
 *									OFF,蜂鸣器
 *									ON,开蜂鸣器
 * 返 回 值: 无
 ***************************************************************/
int BeepStatusSet(E53SF1Status status)
{
    if (status == ON) {
        E53_PWMSet(PWM_FREQ,PWM_DUTY);
    }
    if (status == OFF) {
        E53_PWMSet(PWM_FREQ,1);
    }
    return 0;
}
