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

#include <stdint.h>
#include <string.h>
#include "osal_io.h"
#include "osal_mem.h"

#include "E53_Common.h"
#include "i2c_if.h"
#include "hdf_log.h"     

#define BM_E53_IIC_CHANNEL_NUM 1

static DevHandle i2cHandler = NULL;

E53_Status E53_IICOpen()
{

    if(i2cHandler != NULL){
        E53_Log("I2C has opened,please don't open again!");
        return E53_Failed;
    }
    i2cHandler = I2cOpen(BM_E53_IIC_CHANNEL_NUM);
    if(i2cHandler == NULL){
        E53_Log("I2C open failed!");
        return E53_Failed;
    }
    return E53_Ok;
}             

E53_Status E53_IICClose()
{
    if(i2cHandler == NULL){
        E53_Log("I2C is closed,please don't close again!");
        return E53_Failed;
    }
    I2cClose(i2cHandler);
    i2cHandler = NULL;
    return E53_Ok;
}


E53_Status E53_IICTransmit(E53_IIC_Msg *msgs, int16_t count)
{
    if(i2cHandler == NULL){
        E53_Log("I2C didn't open!");
        return E53_Failed;
    }
    if (I2cTransfer(i2cHandler,(struct I2cMsg*)msgs,count) != count) {
        E53_Log("I2c read error!");
        return E53_Failed;
    }
    return E53_Ok;
}

E53_Status E53_IICWrite(uint32_t addr,uint8_t* data,uint32_t len)
{
    E53_IIC_Msg msg[1];
    (void)memset_s(msg, sizeof(msg), 0, sizeof(msg));
    msg[0].addr = addr;
    msg[0].buf = data;
    msg[0].len = len;
    msg[0].flags = 0;
    if (E53_IICTransmit(msg, 1) != E53_Ok) {
        HDF_LOGE("i2c write failed");
        return -1;
    }
    return 0;
} 

E53_Status E53_IICRead(uint32_t addr,uint8_t* data,uint32_t len)
{
    E53_IIC_Msg msg;
    msg.addr = addr;
    msg.buf = data;
    msg.len = len;
    msg.flags = E53_I2C_FLAG_READ;
    if (E53_IICTransmit(&msg, 1) != E53_Ok) {
        HDF_LOGE("i2c read err");
        return -1;
    }
    return 0;
} 

E53_Status E53_IICWriteRead(uint32_t addr,uint8_t* wdata,uint32_t wlen,uint8_t* rdata,uint32_t rlen)
{
    E53_IIC_Msg msg[2];
    (void)memset_s(msg, sizeof(msg), 0, sizeof(msg));
    msg[0].addr = addr;
    msg[0].buf = wdata;
    msg[0].len = wlen;
    msg[0].flags = 0;
    msg[1].addr = addr;
    msg[1].buf = rdata;
    msg[1].len = rlen;
    msg[1].flags = E53_I2C_FLAG_READ;
    if (E53_IICTransmit(msg, 2) != E53_Ok) {
        HDF_LOGE("i2c write read err");
        return -1;
    }
    return 0;
}







