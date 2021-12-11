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
 
#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif /* __cplusplus */
#endif /* __cplusplus */

/*
MMU内存映射表
| DDR   | DDR   | PMM   | PMM   | PMM   | Sbull | GIC   | ...   |
| 256MB | 256MB | 512MB | 512MB | 512MB | 128M  | 1M    | xxx   |
*/

#define OS_SYS_CLOCK             (50000000)
#define SYS_CTRL_REG_BASE        IO_DEVICE_ADDR(0x12020000)
#define REG_SC_CTRL              0

/* smp */
#define CRG_REG_ADDR              (0x12010000)
#define CRG_REG_BASE              IO_DEVICE_ADDR(CRG_REG_ADDR)
#define PERI_CRG30_BASE           IO_DEVICE_ADDR(CRG_REG_ADDR + 0x0078)  /* cpu freq-mode & reset CRG */

/* memory config */
#define CACHE_ALIGNED_SIZE        (64)

// 256MB
#ifdef LOSCFG_TEE_ENABLE
#define DDR_MEM_ADDR            0xC1000000
#define DDR_MEM_SIZE            0xf000000
#else
#define DDR_MEM_ADDR            0xBF000000
#define DDR_MEM_SIZE            0x11000000
#endif

#define SYS_MEM_SIZE_DEFAULT    0x2000000

/* Peripheral register address base and size */
#define PERIPH_PMM_BASE         0x40000000
#define PERIPH_PMM_SIZE         0x20000000

/* ddr ramfs */
#define DDR_RAMFS_VBASE         (PERIPH_UNCACHED_BASE + PERIPH_UNCACHED_SIZE)
#define DDR_RAMFS_ADDR          (DDR_MEM_ADDR + DDR_MEM_SIZE)
#define DDR_RAMFS_SIZE          (0x4000000)  /* 64M ramfs */
#define GIC_VIRT_SIZE           U32_C(GIC_PHY_SIZE)
#define DDR_RAMFS_REAL_SIZE     (0xa00000) 	/* ramfs real size */

/* kernel load address */
#define KERNEL_LOAD_ADDRESS     (0xC0100000)

/* hwi */
/**
 * 查手册获取中断号列表
 * Maximum number of supported hardware devices that generate hardware interrupts.
 * The maximum number of hardware devices that generate hardware interrupts is 288.
 */
#define OS_HWI_MAX_NUM                  288
/**
 * Maximum interrupt number.
 */
#define OS_HWI_MAX                      ((OS_HWI_MAX_NUM) - 1)
/**
 * Minimum interrupt number.
 */
#define OS_HWI_MIN                       0
/**
 * Maximum usable interrupt number.
 */
#define OS_USER_HWI_MAX                 OS_HWI_MAX
/**
 * Minimum usable interrupt number.
 */
#define OS_USER_HWI_MIN                 OS_HWI_MIN

/*
    通用定时器中断号设置
    使用 Non-secure 的时钟中断
*/
#define NUM_HAL_INTERRUPT_CNTPSIRQ      29  /* Secure physical timer event. */
#define NUM_HAL_INTERRUPT_CNTPNSIRQ     30  /* Non-secure physical timer event. */
#define OS_TICK_INT_NUM                 NUM_HAL_INTERRUPT_CNTPNSIRQ /* 使用 Non-secure physical timer 作为系统时钟中断 */

#define NUM_HAL_INTERRUPT_TIMER0        37
#define NUM_HAL_INTERRUPT_TIMER3        38
#define NUM_HAL_INTERRUPT_UART4         84

/* GIC base and size : 1M-align */
#define GIC_PHY_BASE            0xA0000000
#define GIC_PHY_SIZE            0x100000

/* GIC */
#define GIC_VIRT_BASE             (DDR_RAMFS_VBASE + DDR_RAMFS_SIZE)
#define GIC_BASE_ADDR             (GIC_VIRT_BASE + 0x20000)
#define GICD_OFFSET               0x1000                          /* interrupt distributor offset */
#define GICC_OFFSET               0x2000                          /* CPU interface register offset */

/* FB */
#define FB_VIRT_BASE                (GIC_VIRT_BASE + GIC_VIRT_SIZE)         /* 虚拟地址放在GIC_VIRT后面 */
#define FB_SIZE                     (0x200000 * 8)                          /* 2M * 2 */
#define FB_PHY_BASE                 (DDR_RAMFS_ADDR + DDR_RAMFS_SIZE)       /* 物理地址放在DDR_RAMFS后面 */

#define BIT(n)                    (1U << (n))

/* use UART4 as debug uart */
#define UART4_REG_ADDR             0x40010000
#define UART4_REG_PBASE           (UART4_REG_ADDR + 0x0000)
#define UART4_REG_BASE            IO_DEVICE_ADDR(UART4_REG_PBASE)
#define TTY_DEVICE                "/dev/uartdev-4"
#define UART_REG_BASE             UART4_REG_BASE
#define NUM_HAL_INTERRUPT_UART    NUM_HAL_INTERRUPT_UART4

#ifdef __cplusplus
#if __cplusplus
}
#endif /* __cplusplus */
#endif /* __cplusplus */

#endif
