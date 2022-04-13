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


#define GPIOA_PHYADDR 0x50002000
#define GPIOA_SIZE 0x400
#define HDF_LOG_TAG pwm_driver
#define TIM_CLK_HZ 10000000U

#define PWM_DEFAULT_OCIDLESTATE 0
#define PWM_DEFAULT_PERIOD 1000
#define PWM_DEFAULT_DUTY 500
#define PWM_DEFAULT_OCPOLARITY 0

static int32_t StmPwmSetConfig(struct PwmDev *pwm, struct PwmConfig *config);

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
    uint32_t num;
};
struct StmPwm *sp;

struct PwmMethod g_pwmOps = {
    .setConfig = StmPwmSetConfig,
};

static inline void StmPwmDisable()
{
    HAL_TIM_PWM_Stop(&sp->htim, sp->channel);
}

static inline void StmPwmSetPeriod(uint32_t us)
{
    sp->htim.Init.Period = us;
    TIM_Base_SetConfig(sp->htim.Instance, &sp->htim.Init);
}
static inline void StmPwmSetDuty(uint32_t us)
{
    sp->sConfig.Pulse = us;
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

#if 0

void HAL_TIM_MspPostInit(uint32_t *iomux, uint32_t num)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    GPIO_TypeDef *port = (GPIO_TypeDef *)OsalIoRemap(sp->gpio_port_addr, 0x400);
    GPIO_InitStruct.Pin = 1 << iomux[1];
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

    HAL_GPIO_Init(port, &GPIO_InitStruct);
}
#endif
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    HDF_LOGE("%s enter\r\n",__func__);
    GPIO_InitTypeDef GPIO_InitStruct;

    unsigned char *gpioa= (unsigned char *)OsalIoRemap(GPIOA_PHYADDR, GPIOA_SIZE);
    /**
     * TIM2 GPIO Configuration    
     * PA5     ------> TIM2_CH1 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init((GPIO_TypeDef *)gpioa, &GPIO_InitStruct);
  
}

int32_t PwmDriverDispatch(struct HdfDeviceIoClient *client, int cmdCode, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    
    return HDF_SUCCESS;
}


//驱动对外提供的服务能力，将相关的服务接口绑定到HDF框架
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

int readHcs(struct HdfDeviceObject *obj)
{
    struct DeviceResourceIface *iface = NULL;

    iface = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (iface == NULL || iface->GetUint32 == NULL)
    {
        HDF_LOGE("%s: face is invalid", __func__);
        dprintf("%s: face is invalid", __func__);
        return HDF_FAILURE;
    }
    if (iface->GetUint32Array(obj->property, "pwmIomux", sp->iomux, 2, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read iomux fail", __func__);
        dprintf("%s: read iomux fail", __func__);
        return HDF_FAILURE;
    }
    dprintf(" read iomux %d,%d", sp->iomux[0], sp->iomux[1]);
    if (iface->GetUint32(obj->property, "channel", &sp->channel, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read channel fail", __func__);
        dprintf("%s: read channel fail\r\n", __func__);
        return HDF_FAILURE;
    }
    dprintf("read channel %d\r\n", sp->channel);
    if (iface->GetUint32(obj->property, "tim_clk_hz", &sp->tim_clk_hz, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read tim_clk_hz fail", __func__);
        dprintf("%s: read tim_clk_hz fail", __func__);
        return HDF_FAILURE;
    }
    dprintf("read tim clk = %d\r\n", sp->tim_clk_hz);
    if (iface->GetUint32(obj->property, "num", &sp->num, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read num fail", __func__);
        dprintf("%s: read num fail", __func__);
        return HDF_FAILURE;
    }
    dprintf(" read num %d\r\n", sp->num);
    if (iface->GetUint32(obj->property, "tim_addr", &sp->tim_addr, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read tim_addr fail", __func__);
        dprintf("%s: read tim_addr fail\r\n", __func__);
        return HDF_FAILURE;
    }
    dprintf("read tim_adrr %x\r\n", sp->tim_addr);
    if (iface->GetUint32(obj->property, "gpio_port_addr", &sp->gpio_port_addr, 0) != HDF_SUCCESS)
    {
        HDF_LOGE("%s: read gpio_port_addr fail", __func__);
        dprintf("%s: read gpio_port_addr fail\r\n", __func__);
        return HDF_FAILURE;
    }
    dprintf("read gpio_port_addr %x\r\n", sp->gpio_port_addr);
    return HDF_SUCCESS;
}


int32_t HdfPwmDriverInit(struct HdfDeviceObject *device)
{
    HDF_LOGD("%s enter\r\n",__func__);

    RCC_ClkInitTypeDef    clkconfig;
    uint32_t              pFLatency;
    uint32_t              uwTimclock;

    sp = (struct StmPwm *)OsalMemCalloc(sizeof(*sp));

    readHcs(device);

    HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);   
    __HAL_RCC_TIM2_CLK_ENABLE();
    uwTimclock = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_TIMG1);

    
    sp->sConfig.OCIdleState = PWM_DEFAULT_OCIDLESTATE;
    sp->sConfig.OCPolarity = PWM_DEFAULT_OCPOLARITY;
    sp->htim.Init.Period = PWM_DEFAULT_PERIOD;
    sp->sConfig.Pulse = PWM_DEFAULT_DUTY;

    sp->htim.Init.Prescaler = (uint32_t) ((uwTimclock / (sp->tim_clk_hz)) - 1U);
    sp->htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    sp->htim.Init.ClockDivision = 0U;
    sp->htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    sp->sConfig.OCMode = TIM_OCMODE_PWM1;
    sp->sConfig.OCFastMode = TIM_OCFAST_DISABLE;

    sp->dev.method = &g_pwmOps;
    sp->dev.cfg.duty = sp->sConfig.Pulse;  
    sp->dev.cfg.period = sp->htim.Init.Period;    
    sp->dev.cfg.polarity = sp->sConfig.OCPolarity;    
    sp->dev.cfg.status = PWM_ENABLE_STATUS;
    sp->dev.cfg.number = 0; //continuously ouput
    sp->dev.num = sp->num;

    sp->dev.busy = false;

    if (PwmDeviceAdd(device, &(sp->dev)) != HDF_SUCCESS) {
        
        return HDF_FAILURE;
    }

    sp->htim.Instance = (TIM_TypeDef *)OsalIoRemap(sp->tim_addr, 0x70);
    if (sp->htim.Instance == NULL) {
        HDF_LOGE("error OsalIoRemap for htim \r\n");
        return 1;
    }
    
    if (HAL_TIM_PWM_Init(&sp->htim) == HAL_OK)
    {

        HAL_TIM_PWM_ConfigChannel(&sp->htim, &sp->sConfig, sp->channel);
        //HAL_TIM_MspPostInit(sp->iomux, sp->num);
        HAL_TIM_MspPostInit(&sp->htim);
        
        HAL_TIM_PWM_Start(&sp->htim,sp->channel);
    }else{
        return HDF_FAILURE;
    }
    
    
    return HDF_SUCCESS;
}

void HdfPwnDriverRelease(struct HdfDeviceObject *deviceObject)
{
    if (deviceObject == NULL)
    {
        HDF_LOGE("pwm driver release failed!");
        return;
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
