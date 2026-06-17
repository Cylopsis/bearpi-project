#ifndef CONTROL_H
#define CONTROL_H

#include <stdint.h>

/*******************************************************************************
 * GPIO state levels (PB1 heat/cool relay)
 ******************************************************************************/
#define HEAT 0      /* PB1 low  -> PWM drives PTC/MOS  */
#define COOL 1      /* PB1 high -> PWM drives fan      */

/*******************************************************************************
 * ADC channels (both on ADC instance 1)
 ******************************************************************************/
#define ADC_CH_NTC      1       /* E53_ADC1_INP1 : PTC temperature NTC */
#define ADC_CH_LM35     9       /* PB0           : box temperature LM35 */

/*******************************************************************************
 * Sensor / control constants
 ******************************************************************************/
#define NTC_R25             10000.0f    /* NTC resistance @25C (10k)        */
#define NTC_B_VALUE         3950.0f     /* NTC B value                      */
#define NTC_SERIES_R        10000.0f    /* series resistor (10k)            */
#define ADC_REF_VOLTAGE     3300.0f     /* reference voltage (mV)           */
#define ADC_RESOLUTION      65535.0f    /* 16-bit ADC                       */
#define LM35_MV_PER_DEG     10.0f       /* LM35 10mV/C                      */
#define PTC_MAX_SAFE_TEMP   110.0f      /* PTC over-temperature cutoff      */
#define SAMPLE_PERIOD_MS    1000        /* main loop sample period          */
#define CONTROL_PERIOD_MS   100         /* PID loop period                  */

/*******************************************************************************
 * PID controller
 ******************************************************************************/
typedef struct {
    float kp;
    float ki;
    float kd;
    float integral;
    float prev_error;
    float out_min;
    float out_max;
} pid_ctx_t;

typedef enum {
    CONTROL_STATE_HEATING = 0,
    CONTROL_STATE_WARMING,
    CONTROL_STATE_COOLING
} control_state_t;

/*******************************************************************************
 * Shared state (defined in pid_control.c)
 ******************************************************************************/
extern volatile float current_temperature;    /* box temp (LM35) */
extern volatile float target_temperature;      /* setpoint        */
extern volatile float ptc_temperature;         /* PTC temp (NTC)  */
extern volatile float ptc_target_temp;         /* PTC setpoint    */
extern volatile float final_pwm_duty;          /* 0.0 - 1.0       */
extern volatile control_state_t control_state;
extern volatile uint32_t ptc_state;            /* HEAT / COOL     */

extern float warming_threshold;
extern float warming_bias;
extern float heating_bias;
extern float hysteresis_band;
extern float fan_min;
extern float fan_max;

extern pid_ctx_t pid_box;   /* outer loop: box temp -> PTC setpoint */
extern pid_ctx_t pid_ptc;   /* inner loop: PTC temp -> PWM          */
extern pid_ctx_t pid_cool;  /* fan PI                               */

/*******************************************************************************
 * Interfaces
 ******************************************************************************/
const char *control_state_to_string(control_state_t state);
void tune(int argc, char **argv);
void remote_start(void);

#endif /* CONTROL_H */
