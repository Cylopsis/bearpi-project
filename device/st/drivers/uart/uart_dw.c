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

#include "linux/delay.h"
#include "asm/io.h"

#include "los_typedef.h"
#include "los_task.h"
#include "los_base.h"
#include "los_event.h"
#include "errno.h"

#include "linux/interrupt.h"
#include "linux/kernel.h"
#include "linux/spinlock.h"
#include "uart_dw.h"
#include <sys/bus.h>

#include "uart_dev.h"
#include "string.h"

#include "los_magickey.h"

// offset
#define USART_CR1       (0x00)      /**< USART control register 1,                  offset: 0x00 串口控制寄存器1                */
#define USART_CR2       (0x04)      /**< USART control register 2,                  offset: 0x04 串口控制寄存器2                */
#define USART_CR3       (0x08)      /**< USART control register 3,                  offset: 0x08 串口控制寄存器3                */
#define USART_BRR       (0x0C)      /**< USART Baud Rate register,                  offset: 0x0C 串口波特率寄存器               */
#define USART_GTPR      (0x10)      /**< USART guard time and prescaler register,   offset: 0x10 串口保护时间和预分频器寄存器    */
#define USART_RTOR      (0x14)      /**< USART receiver timeout register,           offset: 0x14 串口接收超时寄存器             */
#define USART_RQR       (0x18)      /**< USART request register,                    offset: 0x18 串口请求寄存器                 */
#define USART_ISR       (0x1C)      /**< USART interrupt and status register,       offset: 0x1C 串口中断与状态寄存器           */
#define USART_ICR       (0x20)      /**< USART interrupt flag clear register ,      offset: 0x20 串口中断状态清除寄存器          */
#define USART_RDR		(0x24)      /**< USART receive data register,               offset: 0x24 串口接收数据寄存器              */
#define USART_TDR		(0x28)      /**< USART transmit data register,              offset: 0x28 串口发送数据寄存器              */
#define USART_PRESC     (0x2C)      /**< USART prescaler register,                  offset: 0x2C 串口预分频器寄存器              */

#define USART_CR1_EN        (0x1U << 0) // 串口使能
#define USART_CR1_RE        (0x1U << 2) // 串口接收使能
#define USART_CR1_TE        (0x1U << 3) // 串口发送使能
#define USART_CR1_RXNEIE    (0x1U << 5) // 串口接收中断使能
#define USART_CR1_FIFOEN    (0x1U << 29)// fifo模式使能

#define USART_ISR_TXE   (0x1U << 7) // 该位为 1 表示可以写
#define USART_ISR_RXNE  (0x1U << 5) // 该位为 1 表示可读

/* Parity control enable */
#define USART_CR1_PCE       (0x1U << 10) // 串口校验使能

/* word length */
#define USART_CR1_M0        (0x1U << 12)
#define USART_CR1_M1        (0x1U << 28)
#define USART_CR1_WL_MASK   (USART_CR1_M0 | USART_CR1_M1)
#define USART_CR1_WL_8B     (0)
#define USART_CR1_WL_9B     (USART_CR1_M0)
#define USART_CR1_WL_7B     (USART_CR1_M1)

/* stop bit */
#define USART_CR2_STOP_OFFSET   (12)
#define USART_CR2_STOP_MASK     (0x3)
#define USART_CR2_STOP_1P       (0x0)
#define USART_CR2_STOP_P5       (0x1)
#define USART_CR2_STOP_2P       (0x2)
#define USART_CR2_STOP_1P5      (0x3)

#ifdef LOSCFG_QUICK_START
__attribute__ ((section(".data"))) UINT32 g_uart_fputc_en = 0;
#else
__attribute__ ((section(".data"))) UINT32 g_uart_fputc_en = 1;
#endif

LITE_OS_SEC_BSS STATIC SPIN_LOCK_INIT(g_uartOutputSpin);

STATIC VOID UartPutcReg(UINTPTR base, CHAR c)
{
    /* Spin while fifo is full */
    while ((GET_UINT32(base + USART_ISR) & USART_ISR_TXE) == 0) {};
    WRITE_UINT32(c, base + USART_TDR);
}

UINTPTR uart_to_ptr(UINTPTR n)
{
    (VOID)n;
    return UART_REG_BASE;
}

char uart_putc(UINTPTR base, char c)
{
    UartPutcReg(base, c);
    return 1;
}

/* test */
void uart_putc_phy(CHAR c)
{
	volatile INT32 *uart_tdr = (INT32 *)(0x40010000 + USART_TDR);
    volatile INT32 *uart_isr = (INT32 *)(0x40010000 + USART_ISR);
    while (((*uart_isr) & USART_ISR_TXE) == 0);
    *uart_tdr = c;
}

void uart_putc_mmu(CHAR c)
{
    UINTPTR base = uart_to_ptr(0);
	uart_putc(base, c);
}

VOID UartPutStr(UINTPTR base, const CHAR *s, UINT32 len)
{
    UINT32 i;
    for (i = 0; i < len; i++) {
        if (*(s + i) == '\n') {
            uart_putc(base, '\r');
        }
        uart_putc(base, *(s + i));
    }
}

UINT32 UartPutsReg(UINTPTR base, const CHAR *s, UINT32 len, BOOL isLock)
{
    UINT32 intSave;

    if (isLock) {
        LOS_SpinLockSave(&g_uartOutputSpin, &intSave);
        UartPutStr(base, s, len);
        LOS_SpinUnlockRestore(&g_uartOutputSpin, intSave);
    } else {
        UartPutStr(base, s, len);
    }

    return len;
}

VOID UartPuts(const CHAR *s, UINT32 len, BOOL isLock)
{
    UINTPTR base = uart_to_ptr(0);
    UartPutsReg(base, s, len, isLock);
}

#define FIFO_SIZE    128
static irqreturn_t dw_irq(int irq, void *data)
{
    char buf[FIFO_SIZE];
    unsigned int count = 0;
    struct dw_port *port = NULL;
    struct uart_driver_data *udd = (struct uart_driver_data *)data;
    unsigned int isr;
    int max_count = 256;
    unsigned char ch = 0;

    if (udd == NULL) {
        uart_error("udd is null!\n");
        return IRQ_HANDLED;
    }
    port = (struct dw_port *)udd->private;

    isr = GET_UINT32(port->phys_base + USART_ISR);

    // if have data to read
    if(isr & USART_ISR_RXNE) {
        do {
            // read data from RDR
            ch = GET_UINT8(port->phys_base + USART_RDR);

            // add data to buffer
            buf[count++] = (char)ch;

            if (CheckMagicKey(buf[count - 1])) {
                goto end;
            }

            // read isr again
            isr = GET_UINT32(port->phys_base + USART_ISR);

        } while((isr & USART_ISR_RXNE) && (max_count-- > 0));
        udd->recv(udd, buf, count);
    }

end:
    return IRQ_HANDLED;
}

static int dw_config_enable(struct uart_driver_data *udd, BOOL enable)
{
    int ret = 0;
    struct dw_port *port = NULL;

    if (udd == NULL) {
        uart_error("udd is null!\n");
        return -EFAULT;
    }

    port = (struct dw_port *)udd->private;
    if (!port) {
        uart_error("port is null!");
        return -EFAULT;
    }

    if (enable) {
        REG_SET_BIT(port->phys_base + USART_CR1, USART_CR1_EN);
    } else {
        REG_CLR_BIT(port->phys_base + USART_CR1, USART_CR1_EN);
    }

    return ret;
}

static int dw_config_fifo(struct uart_driver_data *udd)
{
    int ret = 0;
    struct dw_port *port = NULL;
    struct __uart_attr *attr = NULL;

    if (udd == NULL) {
        uart_error("udd is null!\n");
        return -EFAULT;
    }

    port = (struct dw_port *)udd->private;
    if (!port) {
        uart_error("port is null!");
        return -EFAULT;
    }

    attr = &(udd->attr);
    if (!attr) {
        uart_error("attr is null!");
        return -EFAULT;
    }

    if (attr->fifo_rx_en || attr->fifo_tx_en) {
        REG_SET_BIT(port->phys_base + USART_CR1, USART_CR1_FIFOEN);
    } else {
        REG_CLR_BIT(port->phys_base + USART_CR1, USART_CR1_FIFOEN);
    }

    return ret;
}

static int dw_config_stop_bit(struct uart_driver_data *udd)
{
    int ret = 0;
    struct dw_port *port = NULL;
    struct __uart_attr *attr = NULL;
    UINT32 val = 0;

    if (udd == NULL) {
        uart_error("udd is null!\n");
        return -EFAULT;
    }

    port = (struct dw_port *)udd->private;
    if (!port) {
        uart_error("port is null!");
        return -EFAULT;
    }

    attr = &(udd->attr);
    if (!attr) {
        uart_error("attr is null!");
        return -EFAULT;
    }

    switch (attr->stop_bits) {
        case UART_ATTR_STOPBIT_1:
            val = USART_CR2_STOP_1P;
        break;
        case UART_ATTR_STOPBIT_1P5:
            val = USART_CR2_STOP_1P5;
        break;
        case UART_ATTR_STOPBIT_2:
            val = USART_CR2_STOP_2P;
        break;
        default:/* unknow stop bit */
            return -EFAULT;
        break;
    }

    /* clear stop config */
    REG_CLR_BIT(port->phys_base + USART_CR2, USART_CR2_STOP_MASK);

    /* set config */
    REG_SET_BIT(port->phys_base + USART_CR2, val);

    return ret;
}

static int dw_config_parity(struct uart_driver_data *udd)
{
    int ret = 0;
    struct dw_port *port = NULL;
    struct __uart_attr *attr = NULL;

    if (udd == NULL) {
        uart_error("udd is null!\n");
        return -EFAULT;
    }

    port = (struct dw_port *)udd->private;
    if (!port) {
        uart_error("port is null!");
        return -EFAULT;
    }

    attr = &(udd->attr);
    if (!attr) {
        uart_error("attr is null!");
        return -EFAULT;
    }

    switch(attr->parity)
    {
        case UART_ATTR_PARITY_NONE:
            REG_CLR_BIT(port->phys_base + USART_CR1, USART_CR1_PCE);
        break;
        default:
            return -EFAULT;
        break;
    }

    return ret;
}

static int dw_config_data_bits(struct uart_driver_data *udd)
{
    int ret = 0;
    struct dw_port *port = NULL;
    struct __uart_attr *attr = NULL;
    UINT32 val = 0;

    if (udd == NULL) {
        uart_error("udd is null!\n");
        return -EFAULT;
    }

    port = (struct dw_port *)udd->private;
    if (!port) {
        uart_error("port is null!");
        return -EFAULT;
    }

    attr = &(udd->attr);
    if (!attr) {
        uart_error("attr is null!");
        return -EFAULT;
    }

    switch(attr->data_bits)
    {
        case UART_ATTR_DATABIT_8:
            val = USART_CR1_WL_8B;
        break;
        case UART_ATTR_DATABIT_7:
            val = USART_CR1_WL_7B;
        break;
        default:
            return -EFAULT;
        break;
    }

    /* clear bits */
    REG_CLR_BIT(port->phys_base + USART_CR1, USART_CR1_WL_MASK);

    /* set config */
    REG_SET_BIT(port->phys_base + USART_CR1, val);

    return ret;
}

static int dw_config_in(struct uart_driver_data *udd)
{
    /* uart disable */
    dw_config_enable(udd, 0);

    /* set stop bit */
    dw_config_stop_bit(udd);

    /* set parity */
    dw_config_parity(udd);

    /* set data bit */
    dw_config_data_bits(udd);

    /* fifo mode set */
    dw_config_fifo(udd);

    /* uart enable */
    dw_config_enable(udd, 1);

    return 0;
}

static int dw_startup(struct uart_driver_data *udd) 
{
    int ret = 0;
    struct dw_port *port = NULL;

    if (udd == NULL) {
        uart_error("udd is null!\n");
        return -EFAULT;
    }

    port = (struct dw_port *)udd->private;
    if (!port) {
        uart_error("port is null!");
        return -EFAULT;
    }
    /* enable the clock */
    LOS_TaskLock();
    //uart_clk_cfg(udd->num, true); //use for hi3518
    LOS_TaskUnlock();

    /* uart disable */
    dw_config_enable(udd, 0);

    /* enable Transmitter and Receiver */
    REG_SET_BIT(port->phys_base + USART_CR1, USART_CR1_TE);
    REG_SET_BIT(port->phys_base + USART_CR1, USART_CR1_RE);

    /* enable rx interrupt */
    REG_SET_BIT(port->phys_base + USART_CR1, USART_CR1_RXNEIE);

    /* uart enable */
    dw_config_enable(udd, 1);

    // request_irq
    ret = request_irq(port->irq_num, (irq_handler_t)dw_irq, 0, "uart_dw", udd);

    dw_config_in(udd);

    return ret;
}

static int dw_shutdown(struct uart_driver_data *udd)
{
    struct dw_port *port = NULL;
    unsigned int cr1;

    if (udd == NULL) {
        uart_error("udd is null!\n");
        return -EFAULT;
    }

    port = (struct dw_port *)udd->private;
    if (!port) {
        uart_error("port is null!");
        return -EFAULT;
    }

    /* uart disable */
    cr1 = GET_UINT32(port->phys_base + USART_CR1);
    cr1 &= ~USART_CR1_EN;
    WRITE_UINT32(cr1, port->phys_base + USART_CR1);

    return 0;
}

static int dw_start_tx(struct uart_driver_data *udd, const char *buf, size_t count)
{
    unsigned int tx_len = count;
    struct dw_port *port = NULL;
    char value;
    unsigned int i;
    int ret = 0;

    if (udd == NULL) {
        uart_error("udd is null!\n");
        return -EFAULT;
    }

    port = (struct dw_port *)udd->private;
    if (!port) {
        uart_error("port is null!");
        return -EFAULT;
    }

    /* UART_WITH_LOCK: there is a spinlock in the function to write reg in order. */
    for (i = 0; i < tx_len; i++ ){
        ret = LOS_CopyToKernel((void *)&value, sizeof(char),(void *)(buf++), sizeof(char));
        if (ret) {
            return i;
        }
        (void)UartPutsReg(port->phys_base, &value, 1, UART_WITH_LOCK);
    }

    return count;
}

static int dw_config(struct uart_driver_data *udd)
{
    return dw_config_in(udd);
}

static struct uart_ops dw_uops = {
    .startup        = dw_startup,
    .shutdown       = dw_shutdown,
    .start_tx       = dw_start_tx,
    .config         = dw_config,
};

#define MAX_DEV_NAME_SIZE  32
extern const struct file_operations_vfs uartdev_fops;
extern struct uart_driver_data *get_udd_by_unit(int unit);

struct uart_ops *dw_get_ops(void)
{
    return &dw_uops;
}
