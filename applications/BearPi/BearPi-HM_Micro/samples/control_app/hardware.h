#ifndef HARDWARE_H
#define HARDWARE_H

#include <stdint.h>

/*
 * Hardware abstraction over the HDF user-space services:
 *   hdf_adc_user   - ADC read (LM35 / NTC)
 *   hdf_pwm_user   - PA6 PWM duty
 *   hdf_state_user - PB1 heat/cool GPIO
 *
 * hw_init() binds all three services and must be called once at start-up.
 */
int  hw_init(void);
void hw_deinit(void);

/* Read raw ADC value (0 - 65535) from the given channel. Returns 0 on success. */
int hw_adc_read(uint32_t channel, uint32_t *val);

/* Set PA6 PWM duty cycle, percent 0 - 100. Returns 0 on success. */
int hw_pwm_set_duty(uint32_t percent);

/* Set PB1 level: HEAT(0) or COOL(1). Returns 0 on success. */
int hw_gpio_set_state(uint8_t level);

#endif /* HARDWARE_H */
