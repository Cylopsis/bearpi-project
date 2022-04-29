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

#ifndef __E53_IS1_H__
#define __E53_IS1_H__
#include <stdint.h>


#define WIFI_IOT_IO_NAME_GPIO_7 7
#define WIFI_IOT_IO_NAME_GPIO_8 8
#define WIFI_IOT_PWM_PORT_PWM1 1
#define WIFI_IOT_IO_FUNC_GPIO_8_PWM1_OUT 5
#define WIFI_IOT_IO_FUNC_GPIO_7_GPIO 0
#define PWM_DUTY 50
#define PWM_FREQ 4000

typedef void (*E53IS1CallbackFunc) (char *arg);

struct  E53_IS1Hooks
{
    int (*E53_IS1InitPeripheral)(void);
    int (*E53_IS1DeInitPeripheral)(void);
    int (*E53_IS1GpioIsrSet)(E53IS1CallbackFunc func);
    int (*E53_E53_PWMSet)(uint32_t period,uint32_t duty);
};

typedef enum {
    E53_IS1BeepOFF = 0,
    E53_IS1BeepON,
} E53IS1Status;

int E53_IS1_Init(void);
int E53_IS1_DeInit(void);
int E53_IS1RegisterHooks(struct E53_IS1Hooks* hooks);
int E53_IS1BeepStatusSet(E53IS1Status status);

#endif
