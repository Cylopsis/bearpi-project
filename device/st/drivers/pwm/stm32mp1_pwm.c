#include "hdf_device_desc.h"
#include "hdf_log.h"
#include "device_resource_if.h"
#include "osal_io.h"
#include "osal.h"
#include "osal_mem.h"
#include "osal_spinlock.h"
#include "plat_log.h"

#include "stm32mp1xx_hal.h"
#include "stm32mp1xx_hal_tim.h"
#include "pwm_core.h"
#include "pwm_if.h"

#define GPIOA_SIZE 0x400
#define TIM_REG_SIZE 0X70

#define PWM_DEFAULT_OCIDLESTATE 0
#define PWM_DEFAULT_PERIOD 263
#define PWM_DEFAULT_DUTY 131
#define PWM_DEFAULT_OCPOLARITY 0
#define PWM_DEFAULT_OUTPUT_NUM 0    //continuously ouput

// private struct
struct StmPwm
{
    TIM_HandleTypeDef htim;
    struct PwmDev dev;
    TIM_OC_InitTypeDef sConfig;
    uint32_t tim_addr;
    uint32_t gpio_port_addr;
    uint32_t channel;
    uint32_t iomux[2];
    uint32_t tim_clk_hz;
    GPIO_TypeDef *gpio_port;
};
struct StmPwm *sp = NULL;



static inline void StmPwmDisable()
{
    HAL_TIM_PWM_Stop(&sp->htim, sp->channel);
}

static inline void StmPwmSetPeriod(uint32_t period)
{
    sp->htim.Init.Period = period;
    TIM_Base_SetConfig(sp->htim.Instance, &sp->htim.Init);
}
static inline void StmPwmSetDuty(uint32_t duty)
{
    sp->sConfig.Pulse = duty;
    HAL_TIM_PWM_ConfigChannel(&sp->htim, &sp->sConfig, sp->channel);
}
static inline void StmPwmSetPolarity(uint32_t polarity)
{
    sp->sConfig.OCPolarity = polarity;
    HAL_TIM_PWM_ConfigChannel(&sp->htim, &sp->sConfig, sp->channel);    
}
static inline void StmPwmAlwaysOutput()
{
    HAL_TIM_PWM_Start(&sp->htim, sp->channel);
}

static inline void StmPwmOutputNumberSquareWaves(uint32_t num)
{

    HAL_TIM_PWM_Start(&sp->htim, sp->channel);
}

static int32_t HdfPwmOpen(struct PwmDev *pwm)
{
    (void)pwm;
    return HDF_SUCCESS;
}

static int32_t HdfPwmClose(struct PwmDev *pwm)
{
    (void)pwm;
    return HDF_SUCCESS;
}

static int32_t StmPwmSetConfig(struct PwmDev *pwm, struct PwmConfig *config)
{

    if (pwm->cfg.polarity != config->polarity ) {
        HDF_LOGE("%s: not support set pwm polarity", __func__);
        return HDF_ERR_NOT_SUPPORT;
    }

    if (config->status == PWM_DISABLE_STATUS) {

        StmPwmDisable();
        return HDF_SUCCESS;
    }

    if (config->polarity != PWM_NORMAL_POLARITY && config->polarity != PWM_INVERTED_POLARITY) {
        HDF_LOGE("%s: polarity %u is invalid", __func__, config->polarity);
        return HDF_ERR_INVALID_PARAM;
    }

    if (config->period < 0) {
        HDF_LOGE("%s: period %u is not support, min period %u", __func__, config->period, 0);
        return HDF_ERR_INVALID_PARAM;
    }
    if (config->duty < 1 || config->duty > config->period) {
        HDF_LOGE("%s: duty %u is not support, min dutyCycle 1 max dutyCycle %u",
            __func__, config->duty, config->period);
        return HDF_ERR_INVALID_PARAM;
    }
    
    StmPwmDisable();

    if (pwm->cfg.polarity != config->polarity) {
        StmPwmSetPolarity( config->polarity);
    }
    StmPwmSetPeriod( config->period);
    StmPwmSetDuty( config->duty);
    
    if (config->number == 0) {
        StmPwmAlwaysOutput();
    } else {
        StmPwmOutputNumberSquareWaves( config->number);
    }
    return HDF_SUCCESS;
}


void HAL_TIM_MspPostInit(uint32_t *iomux, uint32_t num)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    sp->gpio_port = (GPIO_TypeDef *)OsalIoRemap(sp->gpio_port_addr, GPIOA_SIZE);
    GPIO_InitStruct.Pin = 1 << iomux[1];    //1<<4=16
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;

    switch (num)
    {
    case 1:
    case 2:
    case 16:
    case 17:
        GPIO_InitStruct.Alternate = GPIO_AF1_TIM1;
        break;
    case 3:
    case 4:
    case 5:
    case 12:
        GPIO_InitStruct.Alternate = GPIO_AF2_TIM3;
        break;
    case 8:
        GPIO_InitStruct.Alternate = GPIO_AF3_TIM8;
        break;
    case 15:
        GPIO_InitStruct.Alternate = GPIO_AF4_TIM15;
        break;
    case 13:
    case 14:
        GPIO_InitStruct.Alternate = GPIO_AF9_TIM13;
        break;
    default:
        break;
    }

    HAL_GPIO_Init(sp->gpio_port, &GPIO_InitStruct);
}

int32_t PwmDriverDispatch(struct HdfDeviceIoClient *client, int cmdCode, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    
    return HDF_SUCCESS;
}


int32_t HdfPwmDriverBind(struct HdfDeviceObject *deviceObject)
{
    HDF_LOGD("%s enter\r\n",__func__);
    if (deviceObject == NULL)
    {
        HDF_LOGE("pwm driver bind failed!");
        return HDF_ERR_INVALID_OBJECT;
    }

    static struct IDeviceIoService pwmDriver = {
        .Dispatch = PwmDriverDispatch,
    };

    deviceObject->service = (struct IDeviceIoService *)(&pwmDriver);
    HDF_LOGD("pwm driver bind success");
    return HDF_SUCCESS;
}

static void Mp1xxPwmRccConfig(uint32_t num)
{
    RCC_PeriphCLKInitTypeDef TIMx_clock_source_config;
    switch (num)
    {
    case 1:
        __HAL_RCC_TIM1_CLK_ENABLE();
        TIMx_clock_source_config.TIMG2PresSelection = RCC_TIMG2PRES_ACTIVATED;
        TIMx_clock_source_config.PeriphClockSelection = RCC_PERIPHCLK_TIMG2;
        HAL_RCCEx_PeriphCLKConfig(&TIMx_clock_source_config);
        break;

    case 2:
        __HAL_RCC_TIM2_CLK_ENABLE();
        TIMx_clock_source_config.TIMG1PresSelection = RCC_TIMG1PRES_ACTIVATED;
        TIMx_clock_source_config.PeriphClockSelection = RCC_PERIPHCLK_TIMG1;
        HAL_RCCEx_PeriphCLKConfig(&TIMx_clock_source_config);
        break;

    case 3:
        __HAL_RCC_TIM3_CLK_ENABLE();
        TIMx_clock_source_config.TIMG1PresSelection = RCC_TIMG1PRES_ACTIVATED;
        TIMx_clock_source_config.PeriphClockSelection = RCC_PERIPHCLK_TIMG1;
        HAL_RCCEx_PeriphCLKConfig(&TIMx_clock_source_config);
        break;

    case 4:
        __HAL_RCC_TIM4_CLK_ENABLE();
        TIMx_clock_source_config.TIMG1PresSelection = RCC_TIMG1PRES_ACTIVATED;
        TIMx_clock_source_config.PeriphClockSelection = RCC_PERIPHCLK_TIMG1;
        HAL_RCCEx_PeriphCLKConfig(&TIMx_clock_source_config);
        break;

    case 5:
        __HAL_RCC_TIM5_CLK_ENABLE();
        TIMx_clock_source_config.TIMG1PresSelection = RCC_TIMG1PRES_ACTIVATED;
        TIMx_clock_source_config.PeriphClockSelection = RCC_PERIPHCLK_TIMG1;
        HAL_RCCEx_PeriphCLKConfig(&TIMx_clock_source_config);
        break;

    case 8:
        __HAL_RCC_TIM8_CLK_ENABLE();
        TIMx_clock_source_config.TIMG2PresSelection = RCC_TIMG2PRES_ACTIVATED;
        TIMx_clock_source_config.PeriphClockSelection = RCC_PERIPHCLK_TIMG2;
        HAL_RCCEx_PeriphCLKConfig(&TIMx_clock_source_config);
        break;

    case 12:
        __HAL_RCC_TIM12_CLK_ENABLE();
        TIMx_clock_source_config.TIMG1PresSelection = RCC_TIMG1PRES_ACTIVATED;
        TIMx_clock_source_config.PeriphClockSelection = RCC_PERIPHCLK_TIMG1;
        HAL_RCCEx_PeriphCLKConfig(&TIMx_clock_source_config);
        break;

    case 13:
        __HAL_RCC_TIM13_CLK_ENABLE();
        TIMx_clock_source_config.TIMG1PresSelection = RCC_TIMG1PRES_ACTIVATED;
        TIMx_clock_source_config.PeriphClockSelection = RCC_PERIPHCLK_TIMG1;
        HAL_RCCEx_PeriphCLKConfig(&TIMx_clock_source_config);
        break;

    case 14:
        __HAL_RCC_TIM14_CLK_ENABLE();
        TIMx_clock_source_config.TIMG1PresSelection = RCC_TIMG1PRES_ACTIVATED;
        TIMx_clock_source_config.PeriphClockSelection = RCC_PERIPHCLK_TIMG1;
        HAL_RCCEx_PeriphCLKConfig(&TIMx_clock_source_config);
        break;

    case 15:
        __HAL_RCC_TIM15_CLK_ENABLE();
        TIMx_clock_source_config.TIMG2PresSelection = RCC_TIMG2PRES_ACTIVATED;
        TIMx_clock_source_config.PeriphClockSelection = RCC_PERIPHCLK_TIMG2;
        HAL_RCCEx_PeriphCLKConfig(&TIMx_clock_source_config);
        break;

    case 16:
        __HAL_RCC_TIM16_CLK_ENABLE();
        TIMx_clock_source_config.TIMG2PresSelection = RCC_TIMG2PRES_ACTIVATED;
        TIMx_clock_source_config.PeriphClockSelection = RCC_PERIPHCLK_TIMG2;
        HAL_RCCEx_PeriphCLKConfig(&TIMx_clock_source_config);
        break;

    case 17:
        __HAL_RCC_TIM17_CLK_ENABLE();
        TIMx_clock_source_config.TIMG2PresSelection = RCC_TIMG2PRES_ACTIVATED;
        TIMx_clock_source_config.PeriphClockSelection = RCC_PERIPHCLK_TIMG2;
        HAL_RCCEx_PeriphCLKConfig(&TIMx_clock_source_config);
        break;

    default:
        break;
    }
}

static int Stm32PwmReadConfig(struct HdfDeviceObject *obj)
{
    struct DeviceResourceIface *iface = NULL;

    iface = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (iface == NULL || iface->GetUint32 == NULL)
    {
        HDF_LOGE("%s: face is invalid", __func__);
        return HDF_FAILURE;
    }
    if (iface->GetUint32Array(obj->property, "pwmIomux", sp->iomux, 2, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read iomux fail", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGD(" read iomux %d,%d", sp->iomux[0], sp->iomux[1]);
    if (iface->GetUint32(obj->property, "channel", &sp->channel, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read channel fail", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGD("read channel %d\r\n", sp->channel);
    if (iface->GetUint32(obj->property, "tim_clk_hz", &sp->tim_clk_hz, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read tim_clk_hz fail", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGD("read tim clk = %d\r\n", sp->tim_clk_hz);
    if (iface->GetUint32(obj->property, "num", &sp->dev.num, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read num fail", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGD(" read num %d\r\n",sp->dev.num);
    if (iface->GetUint32(obj->property, "tim_addr", &sp->tim_addr, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read tim_addr fail", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGD("read tim_adrr %x\r\n", sp->tim_addr);
    if (iface->GetUint32(obj->property, "gpio_port_addr", &sp->gpio_port_addr, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read gpio_port_addr fail", __func__);
        return HDF_FAILURE;
    }
    HDF_LOGD("read gpio_port_addr %x\r\n", sp->gpio_port_addr);
    return HDF_SUCCESS;
}

static int Stm32PwmHalInit(struct StmPwm *sp,uint32_t uwTimclock)
{
    sp->htim.Instance = (TIM_TypeDef *)OsalIoRemap(sp->tim_addr, TIM_REG_SIZE);
    if (sp->htim.Instance == NULL) {
        HDF_LOGE("error OsalIoRemap for htim \r\n");
        return HDF_FAILURE;
    }

    sp->sConfig.OCIdleState = PWM_DEFAULT_OCIDLESTATE;
    sp->sConfig.OCPolarity = PWM_DEFAULT_OCPOLARITY;
    sp->sConfig.Pulse = PWM_DEFAULT_DUTY;
    sp->sConfig.OCMode = TIM_OCMODE_PWM1;
    sp->sConfig.OCFastMode = TIM_OCFAST_DISABLE;

    sp->htim.Init.Period = PWM_DEFAULT_PERIOD;
    sp->htim.Init.Prescaler = (uint32_t) ((uwTimclock / (sp->tim_clk_hz)) - 1U);
    sp->htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    sp->htim.Init.ClockDivision = 0U;
    sp->htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    
    if (HAL_TIM_PWM_Init(&sp->htim) == HAL_OK)
    {
        HAL_TIM_PWM_ConfigChannel(&sp->htim, &sp->sConfig, sp->channel);
        HAL_TIM_MspPostInit(sp->iomux, sp->dev.num);
        HAL_TIM_PWM_Start(&sp->htim,sp->channel);
        return HDF_SUCCESS;
    }else{
        return HDF_FAILURE;
    }    
}

struct PwmMethod g_pwmOps = {
    .setConfig = StmPwmSetConfig,
    .open      = HdfPwmOpen,
    .close     = HdfPwmClose,
};


int32_t HdfPwmDriverInit(struct HdfDeviceObject *device)
{
    HDF_LOGD("%s enter\r\n",__func__);

    RCC_ClkInitTypeDef    clkconfig;
    uint32_t              pFLatency;
    uint32_t              uwTimclock;

    sp = (struct StmPwm *)OsalMemCalloc(sizeof(*sp));
    if(sp == NULL)
    {
        return HDF_FAILURE;
    }
    Stm32PwmReadConfig(device);
    //config rcc clock
    Mp1xxPwmRccConfig(sp->dev.num);
    //get tim clk
    HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);
    if (sp->dev.num <= 14)
    {
        uwTimclock = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_TIMG1);
    }
    else
    {
        uwTimclock = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_TIMG2);
    }
    
    sp->dev.method = &g_pwmOps;
    sp->dev.cfg.duty = PWM_DEFAULT_DUTY;  
    sp->dev.cfg.period = PWM_DEFAULT_PERIOD;    
    sp->dev.cfg.polarity = PWM_DEFAULT_OCPOLARITY;    
    sp->dev.cfg.status = PWM_ENABLE_STATUS;
    sp->dev.cfg.number = PWM_DEFAULT_OUTPUT_NUM; 

    sp->dev.busy = false;

    //add pwm device to pwm core
    if (PwmDeviceAdd(device, &(sp->dev)) != HDF_SUCCESS) {
        return HDF_FAILURE;
    }
    
    return Stm32PwmHalInit(sp,uwTimclock);
}

void HdfPwnDriverRelease(struct HdfDeviceObject *deviceObject)
{
    if (deviceObject == NULL)
    {
        HDF_LOGE("pwm driver release failed!");
        return;
    }
    //release the tim
    HAL_TIM_PWM_DeInit(&sp->htim);
    
    if(sp->htim.Instance){
        OsalIoUnmap(sp->htim.Instance);
    }
    
    //release the gpio
    HAL_GPIO_DeInit(sp->gpio_port, (1<<sp->iomux[1]));
    if(sp->gpio_port){
        OsalIoUnmap(sp->gpio_port);
    }
    PwmDeviceRemove(deviceObject, &(sp->dev));
    //release the sp
    if(sp)
    {
        OsalMemFree(sp);
        sp = NULL;
    }
    HDF_LOGD("pwm driver release success");
    return;
}


struct HdfDriverEntry g_pwmDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_PLATFORM_PWM",	
    .Bind = HdfPwmDriverBind,
    .Init = HdfPwmDriverInit,
    .Release = HdfPwnDriverRelease,
};


HDF_INIT(g_pwmDriverEntry);
