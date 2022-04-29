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

#include <stdio.h>

#include "E53_Common.h"
#include "pwm_if.h"

static DevHandle pwmHandler = NULL;

E53_Status E53_PWMOpen(uint32_t pwmNum)
{
    if (pwmHandler != NULL) {
        E53_Log("PWM has opened,please don't open it again!");
        return E53_Failed;
    }

    pwmHandler = PwmOpen(pwmNum);
    if (pwmHandler == NULL) {
        E53_Log("PWM Open failed");
        return E53_Failed;
    }
    return E53_Ok;
}

E53_Status E53_PWMClose(void)
{
    if (pwmHandler == NULL) {
        E53_Log("Pwm has closed,please don't close it again!");
        return E53_Failed;
    }
    PwmClose(pwmHandler);
    pwmHandler = NULL;
    return E53_Ok;
}

E53_Status E53_PWMStart(void)
{
    uint32_t ret;
    if (pwmHandler == NULL) {
        E53_Log("Pwm didn't opened!");
        return E53_Failed;
    }
    ret = PwmEnable(pwmHandler);
    if (ret != E53_Ok) {
        E53_Log("Pwm Start failed!");
        return E53_Failed;
    }
    return E53_Ok;
}

E53_Status E53_PWMStop(void)
{
    uint32_t ret;
    if (pwmHandler == NULL) {
        E53_Log("Pwm didn't opened!");
        return E53_Failed;
    }
    ret = PwmDisable(pwmHandler);
    if (ret != E53_Ok) {
        E53_Log("Pwm Start failed!");
        return E53_Failed;
    }
    return E53_Ok;
}


E53_Status E53_PWMSetPeriod(uint32_t period)
{
    uint32_t ret;
    if (pwmHandler == NULL) {
        E53_Log("Pwm didn't opened!");
        return E53_Failed;
    }
    ret = PwmSetPeriod(pwmHandler, period);
    if (ret != E53_Ok) {
        E53_Log("Pwm set period failed!");
        return E53_Failed;
    }
    E53_Log("here");
    return E53_Ok;
}

E53_Status E53_PWMSetDuty(uint32_t duty)
{
    uint32_t ret;
    if (pwmHandler == NULL) {
        E53_Log("Pwm didn't opened!");
        return E53_Failed;
    }
    ret = PwmSetDuty(pwmHandler, duty);
    if (ret != E53_Ok) {
        E53_Log("Pwm set duty failed!");
        return E53_Failed;
    }
    E53_Log("here");
    return E53_Ok;
}

E53_Status E53_PWMSetPolarity(uint8_t polarity)
{
    uint32_t ret;
    if (pwmHandler == NULL) {
        E53_Log("Pwm didn't opened!");
        return E53_Failed;
    }
    ret = PwmSetPolarity(pwmHandler, polarity);
    if (ret != 0) {
        E53_Log("Pwm set polarity failed!");
        return E53_Failed;
    }
    return E53_Ok;
}
E53_Status E53_PWMSet(uint32_t period, uint32_t duty)
{
    uint32_t ret;

    ret = E53_PWMSetPeriod(period);
    if (ret != E53_Ok) {
        E53_Log("Pwm set polarity failed!");
        return E53_Failed;
    }
    ret = E53_PWMSetDuty(duty);
    if (ret != E53_Ok) {
        E53_Log("Pwm set polarity failed!");
        return E53_Failed;
    }
    return E53_Ok;
}






