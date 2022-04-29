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

#ifndef __E53_COMMON_H__
#define __E53_COMMON_H__


#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif /* __cplusplus */

#include <stdint.h>

#define E53_Log(fmt, ...) do { dprintf("[E53 INFO][%s][%s]: " fmt "\r\n",__FILE__,__func__, ##__VA_ARGS__);} while (0)

    typedef enum {
        E53_Ok = 0,
        E53_Failed,
    }E53_Status;

    typedef enum {
        E53_IO_0 = 0,
        E53_IO_1,
        E53_IO_2,
        E53_IO_3,
        E53_IO_4,
        E53_IO_5,
        E53_IO_6,
        E53_IO_7,
        E53_IO_8,
        E53_IO_9,
        E53_IO_10,
        E53_IO_11,
        E53_IO_12,
        E53_IO_13,
        E53_IO_14,
    }E53_IO_Name;

    typedef enum {
        E53_IO0_Func_GPIO,
        E53_IO0_Func_SPI_CLK,
    }E53_IO0_Func;

    typedef enum {
        E53_IO1_Func_GPIO,
        E53_IO1_Func_SPI_NSS,
    }E53_IO1_Func;

    typedef enum {
        E53_IO2_Func_GPIO,
    }E53_IO2_Func;

    typedef enum {
        E53_IO3_Func_GPIO,
        E53_IO3_Func_ADC,
    }E53_IO3_Func;

    typedef enum {
        E53_IO4_Func_GPIO,
        E53_IO4_Func_DAC,
    }E53_IO4_Func;

    typedef enum {
        E53_IO5_Func_GPIO,
    }E53_IO5_Func;

    typedef enum {
        E53_IO6_Func_GPIO,
    }E53_IO6_Func;

    typedef enum {
        E53_IO7_Func_GPIO,
        E53_IO7_Func_IIC_SCL,
    }E53_IO7_Func;

    typedef enum {
        E53_IO8_Func_GPIO,
        E53_IO8_Func_IIC_SDA,
    }E53_IO8_Func;

    typedef enum {
        E53_IO9_Func_GPIO,
    }E53_IO9_Func;

    typedef enum {
        E53_IO10_Func_GPIO,
        E53_IO10_Func_UART_RXD,
    }E53_IO10_Func;

    typedef enum {
        E53_IO11_Func_GPIO,
        E53_IO11_Func_UART_TXD,
    }E53_IO11_Func;

    typedef enum {
        E53_IO12_Func_GPIO,
    }E53_IO12_Func;

    typedef enum {
        E53_IO13_Func_GPIO,
        E53_IO13_Func_SPI_MOSI,
    }E53_IO13_Func;

    typedef enum {
        E53_IO14_Func_GPIO,
        E53_IO14_Func_SPI_MISO,
    }E53_IO14_Func;


    typedef enum {
        E53_Level_Down = 0,
        E53_Level_High = 1,
    }E53_GPIO_Level;

    typedef enum {
        E53_GPIO_Out_PullUp = 0,
        E53_GPIO_Out_PullDown,
        E53_GPIO_Out_PullNone,
        E53_GPIO_Out_DrainUp,
        E53_GPIO_Out_DrainDown,
        E53_GPIO_In_Floating,
        E53_GPIO_In_Up,
        E53_GPIO_In_Down,
    }E53_GPIO_Mode;

    typedef enum {
        E53_GPIO_IRQ_TRIGGER_NONE,
        /** Rising edge triggered */
        E53_GPIO_IRQ_TRIGGER_RISING,
        /** Falling edge triggered */
        E53_GPIO_IRQ_TRIGGER_FALLING,
        /** High-level triggered */
        E53_GPIO_IRQ_TRIGGER_HIGH,
        /** Low-level triggered */
        E53_GPIO_IRQ_TRIGGER_LOW,
        /** execute interrupt service routine in thread context */
        E53_GPIO_IRQ_USING_THREAD = (0x1 << 8),
    }E53_GPIO_IRQ_Type;

    typedef enum {
        E53_I2C_FLAG_READ = (0x1 << 0),
        /** 10-bit addressing flag. The value <b>1</b> indicates that a 10-bit address is used. */
        E53_I2C_FLAG_ADDR_10BIT = (0x1 << 4),
        /** Non-ACK read flag. The value <b>1</b> indicates that no ACK signal is sent during the read process. */
        E53_I2C_FLAG_READ_NO_ACK = (0x1 << 11),
        /** Ignoring no-ACK flag. The value <b>1</b> indicates that the non-ACK signal is ignored. */
        E53_I2C_FLAG_IGNORE_NO_ACK = (0x1 << 12),
        /**
         * No START condition flag. The value <b>1</b> indicates that there is no START condition for the message
         * transfer.
         */
         E53_I2C_FLAG_NO_START = (0x1 << 14),
         /** STOP condition flag. The value <b>1</b> indicates that the current transfer ends with a STOP condition. */
         E53_I2C_FLAG_STOP = (0x1 << 15),
    }E53_IIC_Flag;

    typedef struct {
        /** Address of the I2C device */
        uint16_t addr;
        /** Address of the buffer for storing transferred data */
        uint8_t *buf;
        /** Length of the transferred data */
        uint16_t len;
        /**
         * Transfer Mode Flag | Description
         * ------------| -----------------------
         * I2C_FLAG_READ | Read flag
         * I2C_FLAG_ADDR_10BIT | 10-bit addressing flag
         * I2C_FLAG_READ_NO_ACK | No-ACK read flag
         * I2C_FLAG_IGNORE_NO_ACK | Ignoring no-ACK flag
         * I2C_FLAG_NO_START | No START condition flag
         * I2C_FLAG_STOP | STOP condition flag
         */
        uint16_t flags;

    }E53_IIC_Msg;

    typedef enum {
        E53_UartDataBits_8 = 0,
        E53_UartDataBits_7 = 1,
        E53_UartDataBits_6 = 2,
        E53_UartDataBits_5 = 3,
    }E53_UART_DataBits;

    typedef enum {
        E53_UartParityNone = 0,
        E53_UartParityOdd = 1,
        E53_UartParityEven = 2,
        E53_UartParityMark = 3,
    }E53_UART_Parity;

    typedef enum {
        E53_UartStopBits_1 = 0,
        E53_UartStopBits_1P5 = 1,
        E53_UartStopBits_2 = 2,
    }E53_UART_StopBits;

    typedef struct {
        E53_UART_DataBits dataBits;
        E53_UART_Parity parity;
        E53_UART_StopBits stopBits;
    }E53_UART_Attr;


    typedef enum {
        E53_SPI_TimeOut_1us = 1,
        E53_SPI_TimeOut_10us = 10,
        E53_SPI_TimeOut_50us = 50,
        E53_SPI_TimeOut_100us = 100,
        E53_SPI_TimeOut_500us = 500,
        E53_SPI_TimeOut_1ms = 1000,
        E53_SPI_TimeOut_10ms = 10000,
        E53_SPI_TimeOut_50ms = 50000,
        E53_SPI_TimeOut_100ms = 100000,
        E53_SPI_TimeOut_500ms = 500000,
    }E53_SPI_TimeOut;

    typedef int32_t(*E53_GpioIrqFunc)(uint16_t gpio, void *data);

    E53_Status E53_GPIOInit(E53_IO_Name io_name, E53_GPIO_Mode mode);
    E53_Status E53_GPIODeinit(E53_IO_Name io_name);
    E53_Status E53_GPIOWrite(E53_IO_Name io_name, E53_GPIO_Level level);
    E53_Status E53_GPIORead(E53_IO_Name io_name, uint16_t *ret);
    E53_Status E53_GPIOSetIRQ(E53_IO_Name io_name, E53_GPIO_IRQ_Type type, E53_GpioIrqFunc func, void *arg);
    E53_Status E53_GPIOUnsetIRQ(E53_IO_Name io_name);
    E53_Status E53_GPIOStartIRQ(E53_IO_Name io_name);
    E53_Status E53_GPIOStopIRQ(E53_IO_Name io_name);

    E53_Status E53_ADCOpen(void);
    E53_Status E53_ADCClose(void);
    E53_Status E53_ADCRead(uint32_t* value);

    E53_Status E53_DACOpen(void);
    E53_Status E53_DACClose(void);
    E53_Status E53_DACWrite(uint32_t value);

    E53_Status E53_IICOpen(void);
    E53_Status E53_IICClose(void);
    E53_Status E53_IICTransmit(E53_IIC_Msg *msgs, int16_t count);
    E53_Status E53_IICWrite(uint32_t addr, uint8_t* data, uint32_t len);
    E53_Status E53_IICRead(uint32_t addr, uint8_t* data, uint32_t len);
    E53_Status E53_IICWriteRead(uint32_t addr, uint8_t* wdata, uint32_t wlen, uint8_t* rdata, uint32_t rlen);

    E53_Status E53_SPIOpen(void);
    E53_Status E53_SPIClose(void);
    E53_Status E53_SPIReceive(uint8_t *receValue, uint32_t size);
    E53_Status E53_SPITransmit(uint8_t *writeValue, uint32_t size);
    E53_Status E53_SPITransmitReceive(uint8_t *writeValue, uint8_t* receValue, uint32_t size, E53_SPI_TimeOut timeout);

    E53_Status E53_UARTOpen(E53_UART_Attr attr);
    E53_Status E53_UARTClose(void);
    E53_Status E53_UARTSetBaud(uint32_t baudRate);
    E53_Status E53_UARTGetBaud(uint32_t* baudRate);
    E53_Status E53_UARTSendByte(uint8_t sendValue);
    E53_Status E53_UARTSendBuffer(uint8_t* sendValue, uint32_t size);
    E53_Status E53_UARTReadFromBuffer(uint8_t* readValue, uint32_t size);
    E53_Status E53_UARTReceiveHandler(uint8_t* receiveValue, uint32_t size);

    E53_Status E53_PWMOpen(uint32_t pwmNum);
    E53_Status E53_PWMClose(void);
    E53_Status E53_PWMStart(void);
    E53_Status E53_PWMStop(void);
    E53_Status E53_PWMSetPeriod(uint32_t period);
    E53_Status E53_PWMSetDuty(uint32_t duty);
    E53_Status E53_PWMSetPolarity(uint8_t polarity);
    E53_Status E53_PWMSet(uint32_t period, uint32_t duty);

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */

#endif // __E53_COMMON_H__
