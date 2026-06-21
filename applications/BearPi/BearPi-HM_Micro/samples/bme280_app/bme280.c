/*
 * BME280 I2C 测试程序 (用户态)
 *
 * 硬件连接: PF14 -> SCL, PF15 -> SDA
 * 对应 HDF I2C 总线号 1 (控制器 0x40012000, 见 i2c_config.hcs)。
 * I2C 管理器 HDF_PLATFORM_I2C_MANAGER 的 policy=2, 已发布到用户态,
 * 因此本程序可直接通过 i2c_if.h 的 I2cOpen/I2cTransfer 访问。
 *
 * 程序流程: 打开总线 -> 探测芯片 -> 读出厂校准 -> 配置 -> 循环读取并打印。
 */

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "i2c_if.h"

#define BME280_I2C_BUS   1      /* PF14/PF15 -> HDF i2c bus 1 */
#define BME280_ADDR_LOW  0x76  /* SDO 接地 (大多数模块默认) */
#define BME280_ADDR_HIGH 0x77  /* SDO 接 VDD */

/* 寄存器 */
#define REG_ID        0xD0
#define REG_RESET     0xE0
#define REG_CTRL_HUM  0xF2
#define REG_CTRL_MEAS 0xF4
#define REG_CONFIG    0xF5
#define REG_DATA      0xF7  /* press(3) temp(3) hum(2) = 8 字节 */
#define REG_CALIB00   0x88  /* 0x88..0xA1, 26 字节 */
#define REG_CALIB26   0xE1  /* 0xE1..0xE7, 7 字节 */

#define BME280_CHIP_ID 0x60
#define BMP280_CHIP_ID 0x58  /* 没有湿度的兄弟型号 */

/* 出厂校准系数 */
static uint16_t dig_T1;
static int16_t  dig_T2, dig_T3;
static uint16_t dig_P1;
static int16_t  dig_P2, dig_P3, dig_P4, dig_P5, dig_P6, dig_P7, dig_P8, dig_P9;
static uint8_t  dig_H1, dig_H3;
static int16_t  dig_H2, dig_H4, dig_H5;
static int8_t   dig_H6;

static int32_t  t_fine;        /* 温度补偿中间量, 供压力/湿度使用 */
static uint16_t g_addr = BME280_ADDR_LOW;

/*
 * 注意: 本平台 HDF I2C 驱动 (stm32mp1_i2c.c) 直接把 msg.addr 传给
 * HAL_I2C_Master_Transmit, 而 STM32 HAL 要求的是"已左移的 8 位地址"
 * (读时驱动内部再 +1 置读位)。因此这里必须传 7 位地址左移一位的值,
 * 与已工作的 E53 驱动 (BH1750_Addr<<1) 保持一致。
 */
#define I2C_HW_ADDR(a) ((uint16_t)((a) << 1))

/* 写一个寄存器: [reg, val] */
static int BmeWrite(DevHandle h, uint8_t reg, uint8_t val)
{
    uint8_t buf[2] = { reg, val };
    struct I2cMsg msg = { .addr = I2C_HW_ADDR(g_addr), .buf = buf, .len = 2, .flags = 0 };
    return (I2cTransfer(h, &msg, 1) == 1) ? 0 : -1;
}

/* 先写寄存器地址, 再读 len 字节 (重复起始) */
static int BmeRead(DevHandle h, uint8_t reg, uint8_t *data, uint16_t len)
{
    struct I2cMsg msgs[2];
    msgs[0].addr = I2C_HW_ADDR(g_addr); msgs[0].buf = &reg; msgs[0].len = 1; msgs[0].flags = 0;
    msgs[1].addr = I2C_HW_ADDR(g_addr); msgs[1].buf = data; msgs[1].len = len; msgs[1].flags = I2C_FLAG_READ;
    return (I2cTransfer(h, msgs, 2) == 2) ? 0 : -1;
}

static int Bme280ReadCalib(DevHandle h)
{
    uint8_t c1[26];
    uint8_t c2[7];

    if (BmeRead(h, REG_CALIB00, c1, sizeof(c1)) != 0) {
        return -1;
    }
    if (BmeRead(h, REG_CALIB26, c2, sizeof(c2)) != 0) {
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
    /* c1[24] 保留 */
    dig_H1 = c1[25];

    dig_H2 = (int16_t)(c2[0] | (c2[1] << 8));
    dig_H3 = c2[2];
    dig_H4 = (int16_t)((c2[3] << 4) | (c2[4] & 0x0F));
    dig_H5 = (int16_t)((c2[5] << 4) | (c2[4] >> 4));
    dig_H6 = (int8_t)c2[6];
    return 0;
}

/* 以下补偿公式来自 Bosch BME280 数据手册 (定点实现) */

/* 返回温度, 单位 0.01 °C */
static int32_t CompensateT(int32_t adc_T)
{
    int32_t var1, var2;
    var1 = ((((adc_T >> 3) - ((int32_t)dig_T1 << 1))) * ((int32_t)dig_T2)) >> 11;
    var2 = (((((adc_T >> 4) - ((int32_t)dig_T1)) * ((adc_T >> 4) - ((int32_t)dig_T1))) >> 12) *
            ((int32_t)dig_T3)) >> 14;
    t_fine = var1 + var2;
    return (t_fine * 5 + 128) >> 8;
}

/* 返回气压, 单位 Q24.8 Pa (即 结果/256 = Pa) */
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
        return 0; /* 避免除零 */
    }
    p = 1048576 - adc_P;
    p = (((p << 31) - var2) * 3125) / var1;
    var1 = (((int64_t)dig_P9) * (p >> 13) * (p >> 13)) >> 25;
    var2 = (((int64_t)dig_P8) * p) >> 19;
    p = ((p + var1 + var2) >> 8) + (((int64_t)dig_P7) << 4);
    return (uint32_t)p;
}

/* 返回湿度, 单位 Q22.10 %RH (即 结果/1024 = %RH) */
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
    return (uint32_t)(v >> 12);
}

/* 探测芯片, 成功后返回芯片 ID, 失败返回 -1 */
static int Bme280Probe(DevHandle h)
{
    uint8_t addrs[2] = { BME280_ADDR_LOW, BME280_ADDR_HIGH };
    int i;
    for (i = 0; i < 2; i++) {
        uint8_t id = 0;
        int ret;
        g_addr = addrs[i];
        ret = BmeRead(h, REG_ID, &id, 1);
        printf("[bme280] probe addr 0x%02x: read_ret=%d id=0x%02x\r\n", g_addr, ret, id);
        if (ret == 0 && (id == BME280_CHIP_ID || id == BMP280_CHIP_ID)) {
            printf("[bme280] found chip id=0x%02x at addr 0x%02x\r\n", id, g_addr);
            return id;
        }
    }
    return -1;
}

int main(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf("\n==== BME280 I2C test (bus %d, SCL=PF14 SDA=PF15) ====\r\n", BME280_I2C_BUS);

    DevHandle h = I2cOpen(BME280_I2C_BUS);
    if (h == NULL) {
        printf("[bme280] I2cOpen(%d) failed\r\n", BME280_I2C_BUS);
        return -1;
    }

    int id = Bme280Probe(h);
    if (id < 0) {
        printf("[bme280] no BME280/BMP280 on 0x76/0x77 -- check wiring/power\r\n");
        I2cClose(h);
        return -1;
    }

    /* 软复位并等待上电完成 */
    BmeWrite(h, REG_RESET, 0xB6);
    usleep(10 * 1000);

    if (Bme280ReadCalib(h) != 0) {
        printf("[bme280] read calibration failed\r\n");
        I2cClose(h);
        return -1;
    }

    /* 配置: 湿度 x1; 温度 x1, 气压 x1, normal 模式; standby 1000ms, 滤波关闭 */
    BmeWrite(h, REG_CTRL_HUM, 0x01);
    BmeWrite(h, REG_CTRL_MEAS, 0x27);
    BmeWrite(h, REG_CONFIG, 0xA0);
    usleep(50 * 1000);

    printf("[bme280] start sampling, press Ctrl-C to stop...\r\n");
    while (1) {
        uint8_t d[8];
        if (BmeRead(h, REG_DATA, d, sizeof(d)) != 0) {
            printf("[bme280] read data failed\r\n");
            usleep(1000 * 1000);
            continue;
        }

        int32_t adc_P = ((int32_t)d[0] << 12) | ((int32_t)d[1] << 4) | (d[2] >> 4);
        int32_t adc_T = ((int32_t)d[3] << 12) | ((int32_t)d[4] << 4) | (d[5] >> 4);
        int32_t adc_H = ((int32_t)d[6] << 8) | d[7];

        int32_t  t = CompensateT(adc_T);       /* 0.01 °C  (必须先算, 更新 t_fine) */
        uint32_t p = CompensateP(adc_P);        /* Q24.8 Pa */
        uint32_t hum = (id == BME280_CHIP_ID) ? CompensateH(adc_H) : 0; /* Q22.10 %RH */

        printf("T=%d.%02d C  P=%u.%02u hPa  H=%u.%02u %%RH\r\n",
               t / 100, (t < 0 ? -t : t) % 100,
               (p >> 8) / 100, (((p >> 8) % 100)),          /* Pa -> hPa */
               hum >> 10, ((hum & 0x3FF) * 100) >> 10);

        usleep(1000 * 1000); /* 1s */
    }

    I2cClose(h);
    return 0;
}
