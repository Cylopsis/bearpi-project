#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <unistd.h>
#include <pthread.h>
#include "control.h"
#include "hardware.h"

/*******************************************************************************
 * Shared state
 ******************************************************************************/
volatile float current_temperature = 25.0f;
volatile float box_pressure        = 0.0f;
volatile float box_humidity        = 0.0f;
volatile float target_temperature  = 40.0f;
volatile float ptc_temperature     = 25.0f;
volatile float ptc_target_temp     = 40.0f;
volatile float final_pwm_duty      = 0.0f;
volatile control_state_t control_state = CONTROL_STATE_WARMING;
volatile uint32_t ptc_state = HEAT;

float warming_threshold = 3.0f;
float warming_bias      = 5.0f;
float heating_bias      = 30.0f;
float hysteresis_band   = 2.0f;
float fan_min           = 0.00f;
float fan_max           = 0.63f;

pid_ctx_t pid_box;
pid_ctx_t pid_ptc;
pid_ctx_t pid_cool;

/*******************************************************************************
 * Feed-forward tables
 ******************************************************************************/
typedef struct {
    float target_temp;     /* PTC target temperature */
    float base_pwm;        /* corresponding PWM duty (0-1) */
} ff_profile_t;

static ff_profile_t ff_table[] = {
    { 20.0f, 0.18f }, { 25.0f, 0.23f }, { 30.0f, 0.27f }, { 40.0f, 0.36f },
    { 50.0f, 0.46f }, { 60.0f, 0.55f }, { 70.0f, 0.64f }, { 80.0f, 0.73f },
    { 90.0f, 0.82f }, {100.0f, 0.91f }
};
static const int num_ff_profiles = sizeof(ff_table) / sizeof(ff_table[0]);

typedef struct {
    float target_temp;     /* box target temperature */
    float ptc_temp;        /* PTC temp needed to hold it */
} warming_ff_entry_t;

static warming_ff_entry_t warming_ff_table[] = {
    { 25.0f, 35.0f }, { 30.0f, 40.5f }, { 40.0f, 53.0f },
    { 57.0f, 85.0f }, { 70.0f, 100.0f }
};
static const int num_warming_ff_entries = sizeof(warming_ff_table) / sizeof(warming_ff_table[0]);

/*******************************************************************************
 * Helpers
 ******************************************************************************/
const char *control_state_to_string(control_state_t state)
{
    switch (state) {
        case CONTROL_STATE_HEATING: return "HEATING";
        case CONTROL_STATE_WARMING: return "WARMING";
        case CONTROL_STATE_COOLING: return "COOLING";
        default:                    return "ERROR!!!";
    }
}

static float get_feedforward_pwm(float target_temp)
{
    float dt = target_temp;
    if (dt <= ff_table[0].target_temp) return ff_table[0].base_pwm;
    if (dt >= ff_table[num_ff_profiles - 1].target_temp)
        return ff_table[num_ff_profiles - 1].base_pwm;

    for (int i = 0; i < num_ff_profiles - 1; i++) {
        ff_profile_t *a = &ff_table[i];
        ff_profile_t *b = &ff_table[i + 1];
        if (dt >= a->target_temp && dt <= b->target_temp) {
            float ratio = (dt - a->target_temp) / (b->target_temp - a->target_temp);
            return a->base_pwm + ratio * (b->base_pwm - a->base_pwm);
        }
    }
    return 0.0f;
}

static float get_warming_temp(float target_temp)
{
    float ptc_temp = warming_ff_table[0].ptc_temp;
    if (target_temp <= warming_ff_table[0].target_temp) {
        ptc_temp = warming_ff_table[0].ptc_temp;
    } else if (target_temp >= warming_ff_table[num_warming_ff_entries - 1].target_temp) {
        ptc_temp = warming_ff_table[num_warming_ff_entries - 1].ptc_temp;
    } else {
        for (int i = 0; i < num_warming_ff_entries - 1; i++) {
            warming_ff_entry_t *a = &warming_ff_table[i];
            warming_ff_entry_t *b = &warming_ff_table[i + 1];
            if (target_temp >= a->target_temp && target_temp <= b->target_temp) {
                float ratio = (target_temp - a->target_temp) / (b->target_temp - a->target_temp);
                ptc_temp = a->ptc_temp + ratio * (b->ptc_temp - a->ptc_temp);
                break;
            }
        }
    }
    return ptc_temp;
}

static float ntc_adc_to_temp(uint32_t adc_val)
{
    if (adc_val >= (uint32_t)ADC_RESOLUTION) return -100.0f;
    float voltage = (float)adc_val * ADC_REF_VOLTAGE / ADC_RESOLUTION;
    float r_ntc = NTC_SERIES_R * voltage / (ADC_REF_VOLTAGE - voltage);
    float ln_r = logf(r_ntc / NTC_R25);
    float t_kelvin = 1.0f / ((1.0f / 298.15f) + (ln_r / NTC_B_VALUE));
    return t_kelvin - 273.15f;
}

/*******************************************************************************
 * PID control thread
 ******************************************************************************/
static void *pid_entry(void *parameter)
{
    (void)parameter;
    printf("PID control thread started.\n");
    float dt = CONTROL_PERIOD_MS / 1000.0f;
    float fan_cmd = 0.0f;

    while (1) {
        uint32_t adc_value = 0;
        if (hw_adc_read(ADC_CH_NTC, &adc_value) == 0) {
            ptc_temperature = ntc_adc_to_temp(adc_value);
        }

        float error = 0.0f;
        float output = 0.0f;

        switch (control_state) {
            case CONTROL_STATE_HEATING:
            case CONTROL_STATE_WARMING: {
                /* outer loop: box temp -> PTC setpoint */
                float outer_error = target_temperature - current_temperature;
                pid_box.integral += outer_error * dt;
                if (pid_box.integral > 100.0f) pid_box.integral = 100.0f;
                if (pid_box.integral < -100.0f) pid_box.integral = -100.0f;
                float outer_derivative = (outer_error - pid_box.prev_error) / dt;
                float outer_output = pid_box.kp * outer_error + pid_box.ki * pid_box.integral +
                                     pid_box.kd * outer_derivative;
                ptc_target_temp = get_warming_temp(target_temperature) + outer_output;
                float ratio = fabsf(outer_error + hysteresis_band) / (hysteresis_band * 2);
                if (ratio > 1) ratio = 1;
                float dynamic_bias = warming_bias + (heating_bias - warming_bias) * ratio;
                if (ptc_target_temp > target_temperature + dynamic_bias)
                    ptc_target_temp = target_temperature + dynamic_bias;
                else if (ptc_target_temp < target_temperature + warming_bias)
                    ptc_target_temp = target_temperature + warming_bias;
                if (ptc_target_temp > PTC_MAX_SAFE_TEMP) ptc_target_temp = PTC_MAX_SAFE_TEMP;

                pid_box.prev_error = outer_error;

                /* inner loop: PTC temp -> PWM */
                if (ptc_temperature >= PTC_MAX_SAFE_TEMP) {
                    output = 0.0f;
                    printf("WARNING: PTC Overheat! Temp: %.1f\n", ptc_temperature);
                } else {
                    float inner_error = ptc_target_temp - ptc_temperature;
                    pid_ptc.integral += inner_error * dt;
                    if (pid_ptc.integral > 50.0f) pid_ptc.integral = 50.0f;
                    if (pid_ptc.integral < -50.0f) pid_ptc.integral = -50.0f;
                    float inner_derivative = (inner_error - pid_ptc.prev_error) / dt;
                    output = pid_ptc.kp * inner_error + pid_ptc.ki * pid_ptc.integral +
                             pid_ptc.kd * inner_derivative;
                    output += get_feedforward_pwm(ptc_target_temp);
                    if (output > pid_ptc.out_max) output = pid_ptc.out_max;
                    if (output < pid_ptc.out_min) output = pid_ptc.out_min;
                    pid_ptc.prev_error = inner_error;
                }
                break;
            }
            case CONTROL_STATE_COOLING:
                error = current_temperature - target_temperature;
                pid_cool.integral += error * dt;
                if (pid_cool.integral > 50.0f) pid_cool.integral = 50.0f;
                if (pid_cool.integral < -50.0f) pid_cool.integral = -50.0f;
                output = pid_cool.kp * error + pid_cool.ki * pid_cool.integral;
                fan_cmd = fan_cmd + 0.37f * (output - fan_cmd);  /* 1st-order LPF */
                output = fan_cmd;
                if (output > pid_cool.out_max) output = pid_cool.out_max;
                if (output < pid_cool.out_min) output = pid_cool.out_min;
                pid_cool.prev_error = error;
                break;
            default:
                output = 0.0f;
                pid_box.integral *= 0.98f;
                pid_ptc.integral *= 0.98f;
                pid_cool.integral *= 0.98f;
                break;
        }

        final_pwm_duty = output;
        hw_pwm_set_duty((uint32_t)(final_pwm_duty * 100.0f));

        usleep(CONTROL_PERIOD_MS * 1000);
    }
    return NULL;
}

/*******************************************************************************
 * Initialization
 ******************************************************************************/
static int control_init(void)
{
    ptc_state = HEAT;
    control_state = CONTROL_STATE_WARMING;

    pid_ptc.kp = 1.37f; pid_ptc.ki = 0.10f; pid_ptc.kd = 0.70f;
    pid_ptc.out_min = 0.0f; pid_ptc.out_max = 1.0f;
    pid_box.kp = 0.10f; pid_box.ki = 0.76f; pid_box.kd = 0.37f;
    pid_box.out_min = 0.0f; pid_box.out_max = 1.0f;
    pid_cool.kp = 0.01f; pid_cool.ki = 0.001f; pid_cool.kd = 0.0f;
    pid_cool.out_min = fan_min; pid_cool.out_max = fan_max;

    if (hw_init() != 0) {
        printf("[control] hardware init failed\n");
        return -1;
    }
    hw_gpio_set_state(ptc_state);
    hw_pwm_set_duty(0);
    return 0;
}

/*******************************************************************************
 * Wi-Fi association
 *
 * On the board, Wi-Fi is brought up simply by running:
 *   ./bin/wpa_supplicant -i wlan0 -d -c /etc/wpa_supplicant.conf
 * (SSID/PSK come from /etc/wpa_supplicant.conf). The -d form runs in the
 * foreground and stays alive holding the association, so we launch it from a
 * detached thread: system() blocks there, never on the main control path.
 ******************************************************************************/
static void *wifi_thread_entry(void *parameter)
{
    (void)parameter;
    int ret = system("./bin/wpa_supplicant -i wlan0 -d -c /etc/wpa_supplicant.conf");
    /* Only reached if wpa_supplicant exits (e.g. config missing / AP gone). */
    printf("[wifi] wpa_supplicant exited, ret=%d\n", ret);
    return NULL;
}

static void try_connect_wifi(void)
{
    pthread_t tid;
    if (pthread_create(&tid, NULL, wifi_thread_entry, NULL) != 0) {
        printf("[wifi] failed to launch wpa_supplicant thread\n");
        return;
    }
    pthread_detach(tid);
    printf("[wifi] associating on wlan0 via wpa_supplicant...\n");
    /* Give the association a moment to come up before clients connect. */
    sleep(3);
}

/*******************************************************************************
 * Main: state machine loop
 ******************************************************************************/
int main(void)
{
    printf("Temperature control starting...\n");

    if (control_init() != 0) {
        return -1;
    }

    pthread_t pid_tid;
    if (pthread_create(&pid_tid, NULL, pid_entry, NULL) != 0) {
        printf("[control] failed to create PID thread\n");
        return -1;
    }

    try_connect_wifi();
    remote_start();

    while (1) {
        float box_temp = 0.0f, box_press = 0.0f, box_hum = 0.0f;
        if (hw_box_env_read(&box_temp, &box_press, &box_hum) == 0) {
            current_temperature = box_temp;   /* BME280 (replaces LM35) */
            box_pressure        = box_press;
            box_humidity        = box_hum;
        }

        control_state_t previous_state = control_state;
        float upper_bound = target_temperature + hysteresis_band;
        float lower_bound = target_temperature - hysteresis_band;
        if (current_temperature < lower_bound) {
            control_state = CONTROL_STATE_HEATING;
        } else if (current_temperature > upper_bound + 1) {
            control_state = CONTROL_STATE_COOLING;
        } else if (current_temperature < upper_bound) {
            control_state = CONTROL_STATE_WARMING;
        }

        if (control_state != previous_state) {
            hw_pwm_set_duty(0);
            usleep(20 * 1000);
            if (control_state == CONTROL_STATE_COOLING) {
                ptc_state = COOL;
                hw_gpio_set_state(ptc_state);
                pid_cool.integral = 0.0f; pid_cool.prev_error = 0.0f;
                pid_box.integral = 0.0f;  pid_box.prev_error = 0.0f;
                pid_ptc.integral = 0.0f;  pid_ptc.prev_error = 0.0f;
            } else {
                ptc_state = HEAT;
                hw_gpio_set_state(ptc_state);
            }
        }

        usleep(SAMPLE_PERIOD_MS * 1000);
    }

    return 0;
}

/*******************************************************************************
 * Parameter tuning (called from remote.c)
 ******************************************************************************/
void tune(int argc, char **argv)
{
    if (argc < 2) {
        return;
    }

    const char *cmd = argv[1];

    if (strcmp(cmd, "target") == 0 && argc == 3) {
        target_temperature = atof(argv[2]);
    } else if (strcmp(cmd, "hys") == 0 && argc == 3) {
        hysteresis_band = atof(argv[2]);
    } else if (strcmp(cmd, "warmbias") == 0 && argc == 3) {
        warming_bias = atof(argv[2]);
    } else if (strcmp(cmd, "heatbias") == 0 && argc == 3) {
        heating_bias = atof(argv[2]);
    } else if (strcmp(cmd, "box") == 0 && argc == 4) {
        const char *p = argv[2];
        float v = atof(argv[3]);
        if (strcmp(p, "kp") == 0) pid_box.kp = v;
        else if (strcmp(p, "ki") == 0) pid_box.ki = v;
        else if (strcmp(p, "kd") == 0) pid_box.kd = v;
    } else if (strcmp(cmd, "heat") == 0 && argc == 4) {
        const char *p = argv[2];
        float v = atof(argv[3]);
        if (strcmp(p, "kp") == 0) pid_ptc.kp = v;
        else if (strcmp(p, "ki") == 0) pid_ptc.ki = v;
        else if (strcmp(p, "kd") == 0) pid_ptc.kd = v;
    } else if (strcmp(cmd, "cool") == 0 && argc == 4) {
        const char *p = argv[2];
        float v = atof(argv[3]);
        if (strcmp(p, "kp") == 0) pid_cool.kp = v;
        else if (strcmp(p, "ki") == 0) pid_cool.ki = v;
    }
}
