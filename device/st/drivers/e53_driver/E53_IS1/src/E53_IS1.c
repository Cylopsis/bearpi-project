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
#include "osal_io.h"
#include "hdf_device_desc.h" 
#include "hdf_log.h"         
#include "device_resource_if.h"

#include "E53_IS1.h"
#include "E53_Common.h"

#define E53_SI1_IRQ_GPIO_PIN    E53_IO_5

/***************************************************************
 * 函数名称: E53IS1Init
 * 说    明: 初始化E53_IS1
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
int E53_IS1_Init(void)
{
    int ret = -1;

    ret = E53_GPIOInit(E53_SI1_IRQ_GPIO_PIN, E53_GPIO_In_Down);
    if (ret != 0) {
        HDF_LOGE("%s: gpio init failed", __func__);
        return HDF_FAILURE;
    }

    E53_PWMOpen(3);
    E53_PWMSetPeriod(4000);
    E53_PWMSetDuty(1);
    E53_PWMSetPolarity(0);
    E53_PWMStart();

    return 0;
}
/***************************************************************
 * 函数名称: E53IS1Init
 * 说    明: 初始化E53_IS1
 * 参    数: 无
 * 返 回 值: 无
 ***************************************************************/
int E53_IS1_DeInit(void)
{
    E53_PWMClose();
    E53_ADCClose();

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
int E53_IS1BeepStatusSet(E53IS1Status status)
{
    if (status == E53_IS1BeepON) {
        
        E53_PWMSet(PWM_FREQ,300);
    }
    if (status == E53_IS1BeepOFF) {
        E53_PWMSet(PWM_FREQ,1);
    }
    return 0;
}
