#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>

/*
 * Hardware abstraction over the HDF user-space services:
 *   hdf_adc_user   - ADC read (NTC PTC temperature)
 *   hdf_pwm_user   - PA6 PWM duty
 *   hdf_state_user - PB1 heat/cool GPIO
 * plus the BME280 box-temperature sensor on I2C bus 1 (replaces the LM35 ADC).
 *
 * hw_init() binds all three services, opens the BME280, and must be called
 * once at start-up.
 */
int  hw_init(void);
void hw_deinit(void);

/* Read raw ADC value (0 - 65535) from the given channel. Returns 0 on success. */
int hw_adc_read(uint32_t channel, uint32_t *val);

/*
 * Read box/ambient environment from the BME280. Any pointer may be NULL.
 *   temp_c      : temperature, degC
 *   press_hpa   : pressure, hPa
 *   humidity_rh : relative humidity, %RH
 * Returns 0 on success.
 */
int hw_box_env_read(float *temp_c, float *press_hpa, float *humidity_rh);

/* Set PA6 PWM duty cycle, percent 0 - 100. Returns 0 on success. */
int hw_pwm_set_duty(uint32_t percent);

/* Set PB1 level: HEAT(0) or COOL(1). Returns 0 on success. */
int hw_gpio_set_state(uint8_t level);

#endif /* HARDWARE_H */
