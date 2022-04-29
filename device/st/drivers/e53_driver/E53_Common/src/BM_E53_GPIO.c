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
#include "gpio_if.h"


int BM_E53_IO_Remap(E53_IO_Name io_name)
{
    if(io_name < E53_IO_0 || io_name > E53_IO_14){
        E53_Log("E53 interface don't has io_name");
        return -1;
    }
    switch (io_name)
    {
    case E53_IO_0:
        return 140;
    case E53_IO_1:
        return 15;
    case E53_IO_2:
        return 16;
    case E53_IO_3:
        return -1;
    case E53_IO_4:
        return 5;
    case E53_IO_5:
        return 17;
    case E53_IO_6:
        return 6;
    case E53_IO_7:
        return 94;
    case E53_IO_8:
        return 95;
    case E53_IO_9:
        return 28;
    case E53_IO_10:
        return 21;
    case E53_IO_11:
        return 29;
    case E53_IO_12:
        return 72;
    case E53_IO_13:
        return 142;
    case E53_IO_14:
        return 141;
    default:
        return -1;
    }
}

E53_Status E53_GPIOInit(E53_IO_Name io_name,E53_GPIO_Mode mode)
{   
    int gpio_num,ret = -1;
    gpio_num = BM_E53_IO_Remap(io_name);
    if(gpio_num  < 0){
        E53_Log("GPIO number check failed");
        return E53_Failed;
    }
    if(mode >= E53_GPIO_Out_PullUp && mode <= E53_GPIO_Out_DrainDown){
        ret = GpioSetDir(gpio_num,GPIO_DIR_OUT);
    }else if(mode >= E53_GPIO_In_Floating && mode <= E53_GPIO_In_Down){
        ret = GpioSetDir(gpio_num,GPIO_DIR_IN);
    }
    if(ret != 0){
        E53_Log("E53_interface set gpio direction failed!");
        return E53_Failed;
    }
    return E53_Ok;
}

E53_Status E53_GPIODeinit(E53_IO_Name io_name)
{
    (void)io_name;

    return E53_Ok;
}


E53_Status E53_GPIOWrite(E53_IO_Name io_name,E53_GPIO_Level level)
{
    int gpio_num,ret;
    gpio_num = BM_E53_IO_Remap(io_name);
    if(gpio_num  < 0){
        E53_Log("GPIO number check failed");
        return E53_Failed;
    }
    ret = GpioWrite(gpio_num,level);
    if(ret != 0){
    E53_Log("E53_interface gpio write level failed!");
        return E53_Failed;
    }
    return E53_Ok;
}

E53_Status E53_GPIORead(E53_IO_Name io_name,uint16_t *level)
{
    int gpio_num,ret;
    gpio_num = BM_E53_IO_Remap(io_name);
    if(gpio_num  < 0){
        E53_Log("GPIO number check failed");
        return E53_Failed;
    }
    ret = GpioRead(gpio_num,(uint16_t*)level);
    if(ret != 0){
        E53_Log("E53_interface gpio read level failed!");
        return E53_Failed;
    }
    return E53_Ok;
}

E53_Status E53_GPIOSetIRQ(E53_IO_Name io_name,E53_GPIO_IRQ_Type type,E53_GpioIrqFunc func,void *arg)
{
    int gpio_num,ret;    
    int irq_type;
    gpio_num = BM_E53_IO_Remap(io_name);
    if(gpio_num  < 0){
        E53_Log("GPIO number check failed");
        return E53_Failed;
    }
    switch (type)
    {
    case E53_GPIO_IRQ_TRIGGER_NONE:
        irq_type = GPIO_IRQ_TRIGGER_NONE;
        break;
    case E53_GPIO_IRQ_TRIGGER_RISING:
        irq_type = GPIO_IRQ_TRIGGER_RISING;
        break;
    case E53_GPIO_IRQ_TRIGGER_FALLING:
        irq_type = GPIO_IRQ_TRIGGER_FALLING;
        break;
    case E53_GPIO_IRQ_TRIGGER_HIGH:
        irq_type = GPIO_IRQ_TRIGGER_HIGH;
        break;
    case E53_GPIO_IRQ_TRIGGER_LOW:
        irq_type = GPIO_IRQ_TRIGGER_LOW;
        break;
    case E53_GPIO_IRQ_USING_THREAD:
        irq_type = GPIO_IRQ_USING_THREAD;
        break;
    default:
        break;
    }
    ret = GpioSetDir(gpio_num, GPIO_DIR_IN);
    if (ret != HDF_SUCCESS) {
        E53_Log("gpio set dir failed");
        return ret;
    }
    ret = GpioSetIrq(gpio_num,irq_type,func,arg);
    if(ret != 0){
        E53_Log("E53_interface gpio read level failed!");
        return E53_Failed;
    }
    return E53_Ok;
}

E53_Status E53_GPIOUnsetIRQ(E53_IO_Name io_name)
{
    int gpio_num,ret;    
    gpio_num = BM_E53_IO_Remap(io_name);
    if(gpio_num  < 0){
        E53_Log("GPIO number check failed");
        return E53_Failed;
    }
    ret = GpioUnSetIrq(gpio_num);
    if(ret != 0){
        E53_Log("E53_interface gpio read level failed!");
        return E53_Failed;
    }
    return E53_Ok;
}


E53_Status E53_GPIOStartIRQ(E53_IO_Name io_name)
{
    int gpio_num,ret;    
    gpio_num = BM_E53_IO_Remap(io_name);
    if(gpio_num  < 0){
        E53_Log("GPIO number check failed");
        return E53_Failed;
    }
    ret = GpioEnableIrq(gpio_num);
    if(ret != 0){
        E53_Log("E53_interface gpio read level failed!");
        return E53_Failed;
    }
    return E53_Ok;
}


E53_Status E53_GPIOStopIRQ(E53_IO_Name io_name)
{
    int gpio_num,ret;    
    gpio_num = BM_E53_IO_Remap(io_name);
    if(gpio_num  < 0){
        E53_Log("GPIO number check failed");
        return E53_Failed;
    }
    ret = GpioDisableIrq(gpio_num);
    if(ret != 0){
        E53_Log("E53_interface gpio read level failed!");
        return E53_Failed;
    }
    return E53_Ok;
}



