
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

#ifndef __E53_IA1_H__
#define __E53_IA1_H__

#include "hdf_types.h"
#include "i2c_if.h"

#define    BH1750_ADDR_WRITE    0x46    //01000110
#define    BH1750_ADDR_READ     0x47    //01000111
#define    SHT30_ADDR_WRITE     0x44<<1        //10001000
#define    SHT30_ADDR_READ      (0x44<<1)+1        //10001011

#define CRC8_POLYNOMIAL 0x31

typedef enum
{
    POWER_OFF_CMD    =    0x00,    //断电：无激活状态
    POWER_ON_CMD    =    0x01,    //通电：等待测量指令
    RESET_REGISTER    =    0x07,    //重置数字寄存器（在断电状态下不起作用）
    CONT_H_MODE        =    0x10,    //连续H分辨率模式：在11x分辨率下开始测量，测量时间120ms
    CONT_H_MODE2    =    0x11,    //连续H分辨率模式2：在0.51x分辨率下开始测量，测量时间120ms
    CONT_L_MODE        =    0x13,    //连续L分辨率模式：在411分辨率下开始测量，测量时间16ms
    ONCE_H_MODE        =    0x20,    //一次高分辨率模式：在11x分辨率下开始测量，测量时间120ms，测量后自动设置为断电模式
    ONCE_H_MODE2    =    0x21,    //一次高分辨率模式2：在0.51x分辨率下开始测量，测量时间120ms，测量后自动设置为断电模式
    ONCE_L_MODE        =    0x23    //一次低分辨率模式：在411x分辨率下开始测量，测量时间16ms，测量后自动设置为断电模式
} BH1750_MODE;

typedef enum
{
    /* 软件复位命令 */

    SOFT_RESET_CMD = 0x30A2,    
    /*
    单次测量模式
    命名格式：Repeatability_CS_CMD
    CS： Clock stretching
    */
    HIGH_ENABLED_CMD    = 0x2C06,
    MEDIUM_ENABLED_CMD  = 0x2C0D,
    LOW_ENABLED_CMD     = 0x2C10,
    HIGH_DISABLED_CMD   = 0x2400,
    MEDIUM_DISABLED_CMD = 0x240B,
    LOW_DISABLED_CMD    = 0x2416,

    /*
    周期测量模式
    命名格式：Repeatability_MPS_CMD
    MPS：measurement per second
    */
    HIGH_0_5_CMD   = 0x2032,
    MEDIUM_0_5_CMD = 0x2024,
    LOW_0_5_CMD    = 0x202F,
    HIGH_1_CMD     = 0x2130,
    MEDIUM_1_CMD   = 0x2126,
    LOW_1_CMD      = 0x212D,
    HIGH_2_CMD     = 0x2236,
    MEDIUM_2_CMD   = 0x2220,
    LOW_2_CMD      = 0x222B,
    HIGH_4_CMD     = 0x2334,
    MEDIUM_4_CMD   = 0x2322,
    LOW_4_CMD      = 0x2329,
    HIGH_10_CMD    = 0x2737,
    MEDIUM_10_CMD  = 0x2721,
    LOW_10_CMD     = 0x272A,
    /* 周期测量模式读取数据命令 */
    READOUT_FOR_PERIODIC_MODE = 0xE000,
} SHT30_CMD;


typedef struct {
    uint16_t busNum;
    uint16_t addr;
} I2cConfig;

typedef struct {
    struct DevHandle *i2cHandle;
    I2cConfig i2cCfg;
} E53I2cClient;


typedef struct {
    double temperature;
    double humidity;
    double light;
    uint16_t led_status;
    uint16_t motor_status;
} E53Sensor;

#endif