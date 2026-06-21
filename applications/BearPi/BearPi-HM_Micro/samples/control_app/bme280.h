#ifndef BME280_H
#define BME280_H

/*
 * BME280 温湿度气压传感器 (I2C, PF14-SCL / PF15-SDA -> HDF i2c bus 1)。
 * 在 control_app 中用于替代 LM35 读取箱体环境温度。
 */

/* 打开 I2C、探测芯片、读校准并配置为连续测量模式。返回 0 成功。*/
int bme280_init(void);

/* 关闭 I2C 句柄。 */
void bme280_deinit(void);

/*
 * 读取一次数据。任意指针可为 NULL 表示不需要该项。
 *   temp_c     : 温度, °C
 *   press_hpa  : 气压, hPa
 *   humidity_rh: 相对湿度, %RH
 * 返回 0 成功。
 */
int bme280_read(float *temp_c, float *press_hpa, float *humidity_rh);

#endif /* BME280_H */
