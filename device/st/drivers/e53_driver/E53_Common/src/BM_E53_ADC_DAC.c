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

#include "E53_Common.h"
#include "adc_if.h"

#define BM_E53_ADC_NUM 1
#define BM_E53_ADC_CHANNEL_NUM 1

#define BM_E53_DAC_NUM 1
#define BM_E53_DAC_CHANNEL_NUM 1

static DevHandle adcHandler = NULL;

E53_Status E53_ADCOpen(void)
{
    if(adcHandler != NULL){
        E53_Log("ADC has opened,please don't open it again!");
        return E53_Failed;
    }
    adcHandler = AdcOpen(BM_E53_ADC_NUM); 
    if(adcHandler == NULL){
        E53_Log("ADC Open failed");
        return E53_Failed;
    }
    return E53_Ok;
}

E53_Status E53_ADCClose(void)
{
    if(adcHandler == NULL){
        E53_Log("ADC has closed,please don't close it again!");
        return E53_Failed;
    }
    AdcClose(adcHandler); 
    adcHandler = NULL;
    return E53_Ok;
}

E53_Status E53_ADCRead(uint32_t* value)
{
    if(adcHandler == NULL){
        E53_Log("ADC didn't open");
        return E53_Failed;
    }
    if(AdcRead(adcHandler,BM_E53_ADC_CHANNEL_NUM,value) != 0){
        E53_Log("ADC read failed!");
        return E53_Failed;
    }
    return E53_Ok;
}
