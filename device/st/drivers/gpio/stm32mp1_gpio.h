/* 
 * Copyright (c) 2021 Nanjing Xiaoxiongpai Intelligent Technology CO., LIMITED.
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

#ifndef __STM32MP1_GPIO_H__
#define __STM32MP1_GPIO_H__

#include "device_resource_if.h"
#include "gpio_core.h"
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "osal_io.h"
#include "osal_irq.h"
#include "osal_mem.h"
#include "osal_spinlock.h"
#include "plat_log.h"
#include "stm32mp1xx_hal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define HDF_LOG_TAG gpio_stm32mp15xx

#define STM32MP15X_GPIO_DIR(base)        ((base) + 0x00)
#define STM32MP15X_GPIO_DATA(base)       ((base) + 0x18)
#define STM32MP15X_GPIO_ODR(base)       ((base) + 0x14)



#define GROUP_MAX 13
#define BIT_MAX   16

struct GpioGroup {
    volatile unsigned char *regBase;
    EXTI_TypeDef *exitBase;
    unsigned int index; 
    OsalIRQHandle irqFunc;
    OsalSpinlock lock;
};

struct Stm32GpioCntlr {
    struct GpioCntlr cntlr;
    volatile unsigned char *regBase;
    EXTI_TypeDef *exitBase;
    uint32_t gpioPhyBase;
    uint32_t gpioRegStep;
    uint32_t irqPhyBase;
    uint32_t iqrRegStep;
    uint16_t groupNum;
    uint16_t bitNum;
    struct GpioGroup *groups;
};

static struct Stm32GpioCntlr g_Stm32GpioCntlr = {
    .groups = NULL,
    .groupNum = GROUP_MAX,
    .bitNum = BIT_MAX,
};

enum GpioOps {
    GPIO_OPS_SET_DIR = 1,
    GPIO_OPS_GET_DIR,
    GPIO_OPS_WRITE,
    GPIO_OPS_READ
};

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
