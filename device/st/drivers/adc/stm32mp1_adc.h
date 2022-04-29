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
 
#ifndef __STM32MP1_ADC_H__
#define __STM32MP1_ADC_H__

#include "adc_core.h"

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

#define MP1XX_ADC_CHANNEL_COUNT_MAX          20

#define MP1_ADC_PIN_DATA_WIDTH               2
#define MP1XX_ADC_GPIO_A                     0
#define MP1XX_ADC_GPIO_B                     1
#define MP1XX_ADC_GPIO_C                     2
#define MP1XX_ADC_GPIO_D                     3
#define MP1XX_ADC_GPIO_E                     4
#define MP1XX_ADC_GPIO_F                     5
#define MP1XX_ADC_INJECTED_CHANNEL_TAG       253
#define MP1XX_ADC_DEDICATED_PIN_TAG          254
#define MP1XX_ADC_UNAVAILABLE_CHANNEL_TAG    255

#define MP1XX_ADC_DEVICE_1                   1
#define MP1XX_ADC_DEVICE_2                   2

/* EACH ADC REGISITERS OFFSET */
#define MP1XX_ADC_ISR_OFFSET           0x00  // ADC 中断和状态寄存器 (ADC_ISR)
#define MP1XX_ADC_IER_OFFSET           0x04  // ADC 中断使能寄存器（ADC_IER）
#define MP1XX_ADC_CR_OFFSET            0x08  // ADC 控制寄存器（ADC_CR）
#define MP1XX_ADC_CFGR_OFFSET          0x0C  // ADC 配置寄存器（ADC_CFGR）
#define MP1XX_ADC_CFGR2_OFFSET         0x10  // ADC 配置寄存器 2 (ADC_CFGR2)
#define MP1XX_ADC_SMPR1_OFFSET         0x14  // ADC 采样时间寄存器 1 (ADC_SMPR1)
#define MP1XX_ADC_SMPR2_OFFSET         0x18  // ADC 采样时间寄存器 2 (ADC_SMPR2)
#define MP1XX_ADC_PCSEL_OFFSET         0x1C  // ADC 通道预选寄存器（ADC_PCSEL）
#define MP1XX_ADC_LTR1_OFFSET          0x20  // ADC 看门狗阈值寄存器 1 (ADC_LTR1)
#define MP1XX_ADC_HTR1_OFFSET          0x24  // ADC 看门狗阈值寄存器 1 (ADC_HTR1)
#define MP1XX_ADC_SQR1_OFFSET          0x30  // ADC 常规序列寄存器 1 (ADC_SQR1)
#define MP1XX_ADC_SQR2_OFFSET          0x34  // ADC 常规序列寄存器 2 (ADC_SQR2)
#define MP1XX_ADC_SQR3_OFFSET          0x38  // ADC 常规序列寄存器 3 (ADC_SQR3)
#define MP1XX_ADC_SQR4_OFFSET          0x3C  // ADC 常规序列寄存器 4 (ADC_SQR4)
#define MP1XX_ADC_DR_OFFSET            0x40  // ADC 常规数据寄存器 (ADC_DR)
#define MP1XX_ADC_JSQR_OFFSET          0x4C  // ADC 注入序列寄存器 (ADC_JSQR)
#define MP1XX_ADC_OFR1_OFFSET          0x60  // ADC 注入通道 y 偏移寄存器 (ADC_OFR1)
#define MP1XX_ADC_OFR2_OFFSET          0x64  // ADC 注入通道 y 偏移寄存器 (ADC_OFR2)
#define MP1XX_ADC_OFR3_OFFSET          0x68  // ADC 注入通道 y 偏移寄存器 (ADC_OFR3)
#define MP1XX_ADC_OFR4_OFFSET          0x6C  // ADC 注入通道 y 偏移寄存器 (ADC_OFR4)
#define MP1XX_ADC_JDR1_OFFSET          0x80  // ADC 注入通道 y 数据寄存器 (ADC_JDR1)
#define MP1XX_ADC_JDR2_OFFSET          0x84  // ADC 注入通道 y 数据寄存器 (ADC_JDR2)
#define MP1XX_ADC_JDR3_OFFSET          0x88  // ADC 注入通道 y 数据寄存器 (ADC_JDR3)
#define MP1XX_ADC_JDR4_OFFSET          0x8C  // ADC 注入通道 y 数据寄存器 (ADC_JDR4)
#define MP1XX_ADC_AWD2CR_OFFSET        0xA0  // ADC 模拟看门狗 2 配置寄存器 (ADC_AWD2CR)
#define MP1XX_ADC_AWD3CR_OFFSET        0xA4  // ADC 模拟看门狗 3 配置寄存器 (ADC_AWD3CR)
#define MP1XX_ADC_LTR2_OFFSET          0xB0  // ADC 看门狗阈值下限寄存器 2 (ADC_LTR2)
#define MP1XX_ADC_HTR2_OFFSET          0xB4  // ADC 看门狗高阈值寄存器 2 (ADC_HTR2)
#define MP1XX_ADC_LTR3_OFFSET          0xB8  // ADC 看门狗阈值下限寄存器 3 (ADC_LTR3) 
#define MP1XX_ADC_HTR3_OFFSET          0xBC  // ADC 看门狗高阈值寄存器 3 (ADC_HTR3)
#define MP1XX_ADC_DIFSEL_OFFSET        0xC0  // ADC 差分模式选择寄存器（ADC_DIFSEL）
#define MP1XX_ADC_CALFACT_OFFSET       0xC4  // ADC 校准因子寄存器 (ADC_CALFACT)
#define MP1XX_ADC_CALFACT2_OFFSET      0xC8  // ADC 校准因子寄存器 2 (ADC_CALFACT2)
#define MP1XX_ADC_OR_OFFSET            0xD0  // ADC2 选项寄存器（ADC2_OR）

/* MASTER AND SLAVE ADC COMMON REGISITERS OFFSET */
#define MP1XX_ADC_COMMON_REG_BASE      0x48003300
#define MP1XX_ADC_COMMON_REG_SIZE      0x100
#define MP1XX_ADC_CCR_OFFSET           0x08
#define MP1XX_ADC_CSR_OFFSET           0x00   // ADC 通用状态寄存器（ADC_CSR）
#define MP1XX_ADC_CCR_OFFSET           0x08   // ADC 通用控制寄存器（ADC_CCR）
#define MP1XX_ADC_CDR_OFFSET           0x0C   // ADC 双模通用常规数据寄存器 (ADC_CDR)
#define MP1XX_ADC_CDR2_OFFSET          0x10   // 32位双模ADC常用常规数据寄存器 (ADC_CDR2)
#define MP1XX_ADC_HWCFGR0_OFFSET       0x3F0  // ADC 硬件配置寄存器（ADC_HWCFGR0）
#define MP1XX_ADC_VERR_OFFSET          0x3F4  // ADC 版本寄存器（ADC_VERR）
#define MP1XX_ADC_IPDR_OFFSET          0x3F8  // ADC 识别寄存器（ADC_IPIDR）
#define MP1XX_ADC_SIDR_OFFSET          0x3FC  // ADC 大小识别寄存器（ADC_SIDR）

#define MP1XX_GPIO_BASE                0x50003000
#define MP1XX_GPIO_GROUP_SIZE          0x1000
#define MP1XX_GPIO_GROUP_NUMBER        11
#define MP1XX_GPIO_MODE_REG_OFFSET     0x0
#define MP1XX_GPIO_ANALOG_MODE_MASK    0x3
#define MP1XX_GPIO_REG_PIN_SHIFT       2

#define MP1XX_ADC_DEVICE_2             2
#define MP1XX_ADC_DATA_WIDTH_MAX       16
#define MP1XX_CHANNLE_NUM_PER_REG      10
#define MP1XX_SAMPLE_TIME_MASK         0x7
#define MP1XX_SAMPLE_TIME_BITS         3
#define MP1XX_ADC_JQDIS_SHIFT          31
#define MP1XX_ADC_SQ1_SHIFT            6
#define MP1XX_ADC_ADCAL_SHIFT          31
#define MP1XX_ADC_ADSTART_SHIFT        2
#define MP1XX_ADC_ENABLE               0x1
#define MP1XX_ADC_EOC_MASK             (0x1 << 2)
#define MP1XX_ADC_REGULATOR_EN         (0x1 << 28)
#define MP1XX_ADC_REGULATOR_RDY_SHIFT  12
#define MP1XX_ADC_CKMODE_SEL           (0x2 << 16)
#define MP1XX_ADC_CONV_TIME_OUT        100
#define MP1XX_ADC_CAL_TIME_OUT         10
#define MP1XX_ADC_VDDCORE_CHANNEL      19
#define MP1XX_ADC_VREF_CHANNEL         18
#define MP1XX_ADC_TSEN_CHANNEL         17
#define MP1XX_ADC_VBAT_CHANNEL         16
#define MP1XX_ADC_VREF_SHIFT           22
#define MP1XX_ADC_TSEN_SHIFT           23
#define MP1XX_ADC_VBAT_SHIFT           24

struct Mp1xxAdcDevice {
    struct AdcDevice device;
    uint32_t regBasePhy;
    volatile unsigned char  *regBase;
    uint32_t regSize;
    uint32_t devNum;
    uint32_t dataWidth;
    uint32_t sampleTime;
    bool adcEnable;
    uint8_t validChannel[MP1XX_ADC_CHANNEL_COUNT_MAX];
    uint8_t pins[MP1XX_ADC_CHANNEL_COUNT_MAX * MP1_ADC_PIN_DATA_WIDTH];
};

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif /* __STM32MP1_ADC_H__ */
