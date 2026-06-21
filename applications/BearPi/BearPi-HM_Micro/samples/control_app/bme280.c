/*
 * BME280 (I2C) 驱动 —— 由 bme280_app 测试程序移植而来, 输出浮点工程量。
 * 硬件: PF14-SCL / PF15-SDA -> HDF i2c bus 1 (控制器 0x40012000)。
 */

#include "bme280.h"

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "i2c_if.h"

#define BME280_I2C_BUS   1
#define BME280_ADDR_LOW  0x76
#define BME280_ADDR_HIGH 0x77

#define REG_ID        0xD0
#define REG_RESET     0xE0
#define REG_CTRL_HUM  0xF2
#define REG_CTRL_MEAS 0xF4
#define REG_CONFIG    0xF5
#define REG_DATA      0xF7
#define REG_CALIB00   0x88
#define REG_CALIB26   0xE1

#define BME280_CHIP_ID 0x60
#define BMP280_CHIP_ID 0x58

/*
 * 本平台 HDF I2C 驱动 (stm32mp1_i2c.c) 把 msg.addr 直接交给 STM32 HAL,
 * HAL 需要"已左移的 8 位地址"(读时驱动内部再 +1)。必须左移一位,
 * 与已工作的 E53 驱动 (BH1750_Addr<<1) 一致。
 */
#define I2C_HW_ADDR(a) ((uint16_t)((a) << 1))

static DevHandle g_i2c = NULL;
static uint16_t  g_addr = BME280_ADDR_LOW;
static int       g_hasHum = 1;   /* BMP280 没有湿度 */

static uint16_t dig_T1;
static int16_t  dig_T2, dig_T3;
static uint16_t dig_P1;
static int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
static uint8_t  dig_H1, dig_H3;
static int16_t  dig_H2, dig_H4, dig_H5;
static int8_t   dig_H6;

static int32_t  t_fine;

static int BmeWrite(uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    struct I2cMsg msg = { .addr = I2C_HW_ADDR(g_addr), .buf = buf, .len = 2, .flags = 0 };
    return (I2cTransfer(g_i2c, &msg, 1) == 1) ? 0 : -1;
}

static int BmeRead(uint8_t reg, uint8_t *data, uint16_t len)
{
    struct I2cMsg msgs[2];
    msgs[0].addr = I2C_HW_ADDR(g_addr); msgs[0].buf = &reg; msgs[0].len = 1; msgs[0].flags = 0;
    msgs[1].addr = I2C_HW_ADDR(g_addr); msgs[1].buf = data; msgs[1].len = len; msgs[1].flags = I2C_FLAG_READ;
    return (I2cTransfer(g_i2c, msgs, 2) == 2) ? 0 : -1;
}

static int Bme280ReadCalib(void)
{
    uint8_t c1[26];
    uint8_t c2[7];

    if (BmeRead(REG_CALIB00, c1, sizeof(c1)) != 0) {
        return -1;
    }
    if (BmeRead(REG_CALIB26, c2, sizeof(c2)) != 0) {
        return -1;
    }

    dig_T1 = (uint16_t)(c1[0]  | (c1[1]  << 8));
    dig_T2 = (int16_t) (c1[2]  | (c1[3]  << 8));
    dig_T3 = (int16_t) (c1[4]  | (c1[5]  << 8));
    dig_P1 = (uint16_t)(c1[6]  | (c1[7]  << 8));
    dig_P2 = (int16_t) (c1[8]  | (c1[9]  << 8));
    dig_P3 = (int16_t) (c1[10] | (c1[11] << 8));
    dig_P4 = (int16_t) (c1[12] | (c1[13] << 8));
    dig_P5 = (int16_t) (c1[14] | (c1[15] << 8));
    dig_P6 = (int16_t) (c1[16] | (c1[17] << 8));
    dig_P7 = (int16_t) (c1[18] | (c1[19] << 8));
    dig_P8 = (int16_t) (c1[20] | (c1[21] << 8));
    dig_P9 = (int16_t) (c1[22] | (c1[23] << 8));
    dig_H1 = c1[25];

    dig_H2 = (int16_t)(c2[0] | (c2[1] << 8));
    dig_H3 = c2[2];
    dig_H4 = (int16_t)((c2[3] << 4) | (c2[4] & 0x0F));
    dig_H5 = (int16_t)((c2[5] << 4) | (c2[4] >> 4));
    dig_H6 = (int8_t)c2[6];
    return 0;
}

/* Bosch 数据手册定点补偿公式 */
static int32_t CompensateT(int32_t adc_T)
{
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) *
            ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;          /* 0.01 °C */
}

static uint32_t CompensateP(int32_t adc_P)
{
    int64_t var1, var2, p;
    var1 = ((int64_t)t_fine) - 128000;
    var2 = var1 * var1 * (int64_t)dig_P6;
    var2 = var2 + ((var1 * (int64_t)dig_P5) << 17);
    var2 = var2 + (((int64_t)dig_P4) << 35);
    var1 = ((var1 * var1 * (int64_t)dig_P3) >> 8) + ((var1 * (int64_t)dig_P2) << 12);
    var1 = (((((int64_t)1) << 47) + var1)) * ((int64_t)dig_P1) >> 33;
    if (var1 == 0) {
        return 0;
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (uint32_t)p;                       /* Q24.8 Pa */
}

static uint32_t CompensateH(int32_t adc_H)
{
    int32_t v;
    v = (t_fine - ((int32_t)76800));
    v = (((((adc_H << 14) - (((int32_t)dig_H4) << 20) - (((int32_t)dig_H5) * v)) +
           ((int32_t)16384)) >> 15) *
         (((((((v * ((int32_t)dig_H6)) >> 10) *
              (((v * ((int32_t)dig_H3)) >> 11) + ((int32_t)32768))) >> 10) +
            ((int32_t)2097152)) * ((int32_t)dig_H2) + 8192) >> 14));
    v = (v - (((((v >> 15) * (v >> 15)) >> 7) * ((int32_t)dig_H1)) >> 4));
    v = (v < 0) ? 0 : v;
    v = (v > 419430400) ? 419430400 : v;
    return (uint32_t)(v >> 12);               /* Q22.10 %RH */
}

static int Bme280Probe(void)
{
    uint8_t addrs[2] = { BME280_ADDR_LOW, BME280_ADDR_HIGH };
    int i;
    for (i = 0; i < 2; i++) {
        uint8_t id = 0;
        g_addr = addrs[i];
        if (BmeRead(REG_ID, &id, 1) == 0 &&
            (id == BME280_CHIP_ID || id == BMP280_CHIP_ID)) {
            g_hasHum = (id == BME280_CHIP_ID) ? 1 : 0;
            printf("[bme280] found chip id=0x%02x at addr 0x%02x\n", id, g_addr);
            return 0;
        }
    }
    return -1;
}

int bme280_init(void)
{
    if (g_i2c != NULL) {
        return 0;
    }
    g_i2c = I2cOpen(BME280_I2C_BUS);
    if (g_i2c == NULL) {
        printf("[bme280] I2cOpen(%d) failed\n", BME280_I2C_BUS);
        return -1;
    }
    if (Bme280Probe() != 0) {
        printf("[bme280] sensor not found on 0x76/0x77\n");
        I2cClose(g_i2c);
        g_i2c = NULL;
        return -1;
    }

    BmeWrite(REG_RESET, 0xB6);
    usleep(10 * 1000);

    if (Bme280ReadCalib() != 0) {
        printf("[bme280] read calibration failed\n");
        I2cClose(g_i2c);
        g_i2c = NULL;
        return -1;
    }

    /* 湿度 x1; 温度 x1, 气压 x1, normal 模式; standby 1000ms, 滤波关闭 */
    BmeWrite(REG_CTRL_HUM, 0x01);
    BmeWrite(REG_CTRL_MEAS, 0x27);
    BmeWrite(REG_CONFIG, 0xA0);
    usleep(50 * 1000);
    return 0;
}

void bme280_deinit(void)
{
    if (g_i2c != NULL) {
        I2cClose(g_i2c);
        g_i2c = NULL;
    }
}

int bme280_read(float *temp_c, float *press_hpa, float *humidity_rh)
{
    if (g_i2c == NULL) {
        return -1;
    }

    uint8_t d[8];
    if (BmeRead(REG_DATA, d, sizeof(d)) != 0) {
        return -1;
    }

    int32_t adc_P = ((int32_t)d[0] << 12) | ((int32_t)d[1] << 4) | (d[2] >> 4);
    int32_t adc_T = ((int32_t)d[3] << 12) | ((int32_t)d[4] << 4) | (d[5] >> 4);
    int32_t adc_H = ((int32_t)d[6] << 8) | d[7];

    int32_t t = CompensateT(adc_T);   /* 必须先算, 更新 t_fine */
    if (temp_c != NULL) {
        *temp_c = (float)t / 100.0f;
    }
    if (press_hpa != NULL) {
        *press_hpa = (float)CompensateP(adc_P) / 256.0f / 100.0f;
    }
    if (humidity_rh != NULL) {
        *humidity_rh = g_hasHum ? ((float)CompensateH(adc_H) / 1024.0f) : 0.0f;
    }
    return 0;
}
