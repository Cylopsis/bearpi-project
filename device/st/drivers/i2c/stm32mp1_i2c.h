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
#ifndef __STM32MP1_I2C_H__
#define __STM32MP1_I2C_H__

#include "stm32mp1xx_hal_i2c.h"
#include "device_resource_if.h"
#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "i2c_core.h"
#include "i2c_dev.h"
#include "osal_io.h"
#include "osal_mem.h"
#include "osal_spinlock.h"
#include "stm32mp1xx_hal.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

struct Stm32mp1I2cCntlr {
    struct I2cCntlr cntlr;
    OsalSpinlock    spin;
    I2C_HandleTypeDef hi2c;
    int16_t bus;
    uint16_t regSize;
    uint32_t regBasePhy;
    uint32_t i2cClkIomux[2];
    uint32_t i2cDataIomux[2];
};

struct Stm32mp1TransferData {
    struct I2cMsg *msgs;
    int16_t index;
    int16_t count;
};

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif 
