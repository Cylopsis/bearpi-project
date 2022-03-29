/*
 * @Author: your name
 * @Date: 2022-03-17 15:52:29
 * @LastEditTime: 2022-03-22 09:54:19
 * @LastEditors: Please set LastEditors
 * @Description: 打开koroFileHeader查看配置 进行设置: https://github.com/OBKoro1/koro1FileHeader/wiki/%E9%85%8D%E7%BD%AE
 * @FilePath: \buff\pwm\led.c
 */

#include "stm32mp1_pwm.h" 


#define GPIOA_PHYADDR 0x50002000
#define GPIOA_SIZE 0x400
#define HDF_LOG_TAG pwm_driver

static int32_t StmPwmSetConfig(struct PwmDev *pwm, struct PwmConfig *config);
//私有pwm结构体
struct StmPwm {
    TIM_HandleTypeDef htim;
    struct PwmDev dev;
    TIM_OC_InitTypeDef sConfig;
    uint32_t physics_register;
    uint32_t register_size;
    uint32_t channel;
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

//设置pwm
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
    //暂停pwm，更新配置
    StmPwmDisable();

    if (pwm->cfg.polarity != config->polarity) {
        StmPwmSetPolarity( config->polarity);
    }
    StmPwmSetPeriod( config->period);
    StmPwmSetDuty( config->duty);
    //继续输出

    if (config->number == 0) {
        StmPwmAlwaysOutput();
    } else {
        StmPwmOutputNumberSquareWaves( config->number);
    }
    return HDF_SUCCESS;
}


//初始化gpio口
void HAL_TIM_MspPostInit(TIM_HandleTypeDef* htim)
{
    __HAL_RCC_GPIOA_CLK_ENABLE();
    dprintf("%s enter\r\n",__func__);
    GPIO_InitTypeDef GPIO_InitStruct;

    //gpioa addr
    unsigned char *gpioa= (unsigned char *)OsalIoRemap(GPIOA_PHYADDR, GPIOA_SIZE);
    /**TIM2 GPIO Configuration    
    PA5     ------> TIM2_CH1 
    */
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF1_TIM2;
    HAL_GPIO_Init((GPIO_TypeDef *)gpioa, &GPIO_InitStruct);
  
}
// Dispatch是对外提供的服务，用来处理用户态发下来的消息，同时返回数据
int32_t PwmDriverDispatch(struct HdfDeviceIoClient *client, int cmdCode, struct HdfSBuf *data, struct HdfSBuf *reply)
{
    
    return HDF_SUCCESS;
}


//驱动对外提供的服务能力，将相关的服务接口绑定到HDF框架
int32_t HdfPwmDriverBind(struct HdfDeviceObject *deviceObject)
{
    dprintf("%s enter\r\n",__func__);
    if (deviceObject == NULL)
    {
        HDF_LOGE("pwm driver bind failed!");
        return HDF_ERR_INVALID_OBJECT;
    }
	//定义对外提供的服务函数
    static struct IDeviceIoService pwmDriver = {
        .Dispatch = PwmDriverDispatch,
    };
	//将服务绑定到驱动
    deviceObject->service = (struct IDeviceIoService *)(&pwmDriver);
    HDF_LOGD("pwm driver bind success");
    return HDF_SUCCESS;
}

//读取配置文件   pa5 tim2 channel 1
int readHcs(struct HdfDeviceObject *obj)
{
    dprintf("%s enter\r\n",__func__);


    struct DeviceResourceIface *iface = NULL;

    iface = DeviceResourceGetIfaceInstance(HDF_CONFIG_SOURCE);
    if (iface == NULL || iface->GetUint32 == NULL) {
        HDF_LOGE("%s: face is invalid", __func__);
        return HDF_FAILURE;
    }
    if (iface->GetUint32(obj->property, "Period", &sp->htim.Init.Period, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read num fail", __func__);
        return HDF_FAILURE;
    }
    if (iface->GetUint32(obj->property, "Pulse", &sp->sConfig.Pulse, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read num fail", __func__);
        return HDF_FAILURE;
    }
    if (iface->GetUint32(obj->property, "physics_register", &sp->physics_register, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read num fail", __func__);
        return HDF_FAILURE;
    }
    if (iface->GetUint32(obj->property, "register_size", &sp->register_size, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read num fail", __func__);
        return HDF_FAILURE;
    }
    if (iface->GetUint32(obj->property, "channel", &sp->channel, 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read num fail", __func__);
        return HDF_FAILURE;
    }
    if (iface->GetUint32(obj->property, "Polarity", &sp->sConfig.OCPolarity , 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read num fail", __func__);
        return HDF_FAILURE;
    }
    if (iface->GetUint32(obj->property, "IdleState", &sp->sConfig.OCIdleState , 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read num fail", __func__);
        return HDF_FAILURE;
    }
    if (iface->GetUint32(obj->property, "IdleState", &sp->dev.num , 0) != HDF_SUCCESS) {
        HDF_LOGE("%s: read num fail", __func__);
        return HDF_FAILURE;
    }
    dprintf("Period =%d,Pulse=%d,physics_register=%x,register_size=%x,channel=%d,Polarity=%d,IdleState=%d,num=%d", sp->htim.Init.Period, sp->sConfig.Pulse, sp->physics_register, sp->register_size, sp->channel, sp->sConfig.OCPolarity, sp->sConfig.OCIdleState,sp->dev.num);

    return 0;
}



// 驱动自身业务初始的接口（设置IO口为输出） HDF框架在加载驱动的时候，会将私有配置信息保存在HdfDeviceObject 中的property里面
int32_t HdfPwmDriverInit(struct HdfDeviceObject *device)
{
    dprintf("%s enter\r\n",__func__);

    RCC_ClkInitTypeDef    clkconfig;
    uint32_t              pFLatency;
    uint32_t              uwTimclock;

    sp = (struct StmPwm *)OsalMemCalloc(sizeof(*sp));

    //读取配置文件
    readHcs(device);

    //获取时钟频率
    HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);   
    __HAL_RCC_TIM2_CLK_ENABLE();
    uwTimclock = HAL_RCCEx_GetPeriphCLKFreq(RCC_PERIPHCLK_TIMG1);
    dprintf(" uwTimclock = %d\r\n",uwTimclock);

    sp->htim.Init.Prescaler = (uint32_t) ((uwTimclock / 10000000U) - 1U);    //10MHZ 改进下pwm的精度
    sp->htim.Init.CounterMode = TIM_COUNTERMODE_UP;
    sp->htim.Init.ClockDivision = 0U;
    sp->htim.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    sp->sConfig.OCMode = TIM_OCMODE_PWM1;
    sp->sConfig.OCFastMode = TIM_OCFAST_DISABLE;

    sp->dev.method = &g_pwmOps;
    sp->dev.cfg.duty = sp->sConfig.Pulse;  //0.1us
    sp->dev.cfg.period = sp->htim.Init.Period;    //0.1us
    sp->dev.cfg.polarity = sp->sConfig.OCPolarity;    
    sp->dev.cfg.status = PWM_ENABLE_STATUS;
    sp->dev.cfg.number = 0; //continuously

    sp->dev.busy = false;

    //添加到核心层
    if (PwmDeviceAdd(device, &(sp->dev)) != HDF_SUCCESS) {
        
        return HDF_FAILURE;
    }

    sp->htim.Instance = (TIM_TypeDef *)OsalIoRemap(sp->physics_register, sp->register_size);
    if (sp->htim.Instance == NULL) {
        dprintf("error OsalIoRemap for htim \r\n");
        return -1;
    }
    
    if (HAL_TIM_PWM_Init(&sp->htim) == HAL_OK)
    {
        dprintf("pwm init ok config channel \r\n");

        HAL_TIM_PWM_ConfigChannel(&sp->htim, &sp->sConfig, sp->channel);

        HAL_TIM_MspPostInit(&sp->htim);//初始化gpio
        
        HAL_TIM_PWM_Start(&sp->htim,sp->channel);
    }else{
        return -1;
    }
    
    
    return HDF_SUCCESS;
}


// 驱动资源释放的接口
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

// 定义驱动入口的对象，必须为HdfDriverEntry（在hdf_device_desc.h中定义）类型的全局变量
struct HdfDriverEntry g_pwmDriverEntry = {
    .moduleVersion = 1,
    .moduleName = "HDF_PLATFORM_PWM",	//必须与device_info.hcs中的字段一样，用于与驱动设备资源匹配
    .Bind = HdfPwmDriverBind,
    .Init = HdfPwmDriverInit,
    .Release = HdfPwnDriverRelease,
};

// 调用HDF_INIT将驱动入口注册到HDF框架中
HDF_INIT(g_pwmDriverEntry);
