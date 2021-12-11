/**
  ******************************************************************************
  * @file    stm32mp1xx_hal_i2c.c
  * @author  MCD Application Team
  * @brief   I2C HAL module driver.
  *          This file provides firmware functions to manage the following
  *          functionalities of the Inter Integrated Circuit (I2C) peripheral:
  *           + Initialization and de-initialization functions
  *           + IO operation functions
  *           + Peripheral State and Errors functions
  *
  @verbatim
  ==============================================================================
                        ##### How to use this driver #####
  ==============================================================================
    [..]
    The I2C HAL driver can be used as follows:

    (#) Declare a I2C_HandleTypeDef handle structure, for example:
        I2C_HandleTypeDef  hi2c;

    (#)Initialize the I2C low level resources by implementing the @ref HAL_I2C_MspInit() API:
        (##) Enable the I2Cx interface clock
        (##) I2C pins configuration
            (+++) Enable the clock for the I2C GPIOs
            (+++) Configure I2C pins as alternate function open-drain
        (##) NVIC configuration if you need to use interrupt process
            (+++) Configure the I2Cx interrupt priority
            (+++) Enable the NVIC I2C IRQ Channel
        (##) DMA Configuration if you need to use DMA process
            (+++) Declare a DMA_HandleTypeDef handle structure for the transmit or receive stream
            (+++) Enable the DMAx interface clock using
            (+++) Configure the DMA handle parameters
            (+++) Configure the DMA Tx or Rx stream
            (+++) Associate the initialized DMA handle to the hi2c DMA Tx or Rx handle
            (+++) Configure the priority and enable the NVIC for the transfer complete interrupt on
                  the DMA Tx or Rx stream

    (#) Configure the Communication Clock Timing, Own Address1, Master Addressing mode, Dual Addressing mode,
        Own Address2, Own Address2 Mask, General call and Nostretch mode in the hi2c Init structure.

    (#) Initialize the I2C registers by calling the @ref HAL_I2C_Init(), configures also the low level Hardware
        (GPIO, CLOCK, NVIC...etc) by calling the customized @ref HAL_I2C_MspInit(&hi2c) API.

    (#) To check if target device is ready for communication, use the function @ref HAL_I2C_IsDeviceReady()

    (#) For I2C IO and IO MEM operations, three operation modes are available within this driver :

    *** Polling mode IO operation ***
    =================================
    [..]
      (+) Transmit in master mode an amount of data in blocking mode using @ref HAL_I2C_Master_Transmit()
      (+) Receive in master mode an amount of data in blocking mode using @ref HAL_I2C_Master_Receive()
      (+) Transmit in slave mode an amount of data in blocking mode using @ref HAL_I2C_Slave_Transmit()
      (+) Receive in slave mode an amount of data in blocking mode using @ref HAL_I2C_Slave_Receive()

    *** Polling mode IO MEM operation ***
    =====================================
    [..]
      (+) Write an amount of data in blocking mode to a specific memory address using @ref HAL_I2C_Mem_Write()
      (+) Read an amount of data in blocking mode from a specific memory address using @ref HAL_I2C_Mem_Read()


    *** Interrupt mode IO operation ***
    ===================================
    [..]
      (+) Transmit in master mode an amount of data in non-blocking mode using @ref HAL_I2C_Master_Transmit_IT()
      (+) At transmission end of transfer, @ref HAL_I2C_MasterTxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_MasterTxCpltCallback()
      (+) Receive in master mode an amount of data in non-blocking mode using @ref HAL_I2C_Master_Receive_IT()
      (+) At reception end of transfer, @ref HAL_I2C_MasterRxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_MasterRxCpltCallback()
      (+) Transmit in slave mode an amount of data in non-blocking mode using @ref HAL_I2C_Slave_Transmit_IT()
      (+) At transmission end of transfer, @ref HAL_I2C_SlaveTxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_SlaveTxCpltCallback()
      (+) Receive in slave mode an amount of data in non-blocking mode using @ref HAL_I2C_Slave_Receive_IT()
      (+) At reception end of transfer, @ref HAL_I2C_SlaveRxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_SlaveRxCpltCallback()
      (+) In case of transfer Error, @ref HAL_I2C_ErrorCallback() function is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_ErrorCallback()
      (+) Abort a master I2C process communication with Interrupt using @ref HAL_I2C_Master_Abort_IT()
      (+) End of abort process, @ref HAL_I2C_AbortCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_AbortCpltCallback()
      (+) Discard a slave I2C process communication using @ref __HAL_I2C_GENERATE_NACK() macro.
           This action will inform Master to generate a Stop condition to discard the communication.


    *** Interrupt mode or DMA mode IO sequential operation ***
    ==========================================================
    [..]
      (@) These interfaces allow to manage a sequential transfer with a repeated start condition
          when a direction change during transfer
    [..]
      (+) A specific option field manage the different steps of a sequential transfer
      (+) Option field values are defined through @ref I2C_XFEROPTIONS and are listed below:
      (++) I2C_FIRST_AND_LAST_FRAME: No sequential usage, functionnal is same as associated interfaces in no sequential mode
      (++) I2C_FIRST_FRAME: Sequential usage, this option allow to manage a sequence with start condition, address
                            and data to transfer without a final stop condition
      (++) I2C_FIRST_AND_NEXT_FRAME: Sequential usage (Master only), this option allow to manage a sequence with start condition, address
                            and data to transfer without a final stop condition, an then permit a call the same master sequential interface
                            several times (like @ref HAL_I2C_Master_Seq_Transmit_IT() then @ref HAL_I2C_Master_Seq_Transmit_IT()
                            or @ref HAL_I2C_Master_Seq_Transmit_DMA() then @ref HAL_I2C_Master_Seq_Transmit_DMA())
      (++) I2C_NEXT_FRAME: Sequential usage, this option allow to manage a sequence with a restart condition, address
                            and with new data to transfer if the direction change or manage only the new data to transfer
                            if no direction change and without a final stop condition in both cases
      (++) I2C_LAST_FRAME: Sequential usage, this option allow to manage a sequance with a restart condition, address
                            and with new data to transfer if the direction change or manage only the new data to transfer
                            if no direction change and with a final stop condition in both cases
      (++) I2C_LAST_FRAME_NO_STOP: Sequential usage (Master only), this option allow to manage a restart condition after several call of the same master sequential
                            interface several times (link with option I2C_FIRST_AND_NEXT_FRAME).
                            Usage can, transfer several bytes one by one using HAL_I2C_Master_Seq_Transmit_IT(option I2C_FIRST_AND_NEXT_FRAME then I2C_NEXT_FRAME)
                              or HAL_I2C_Master_Seq_Receive_IT(option I2C_FIRST_AND_NEXT_FRAME then I2C_NEXT_FRAME)
                              or HAL_I2C_Master_Seq_Transmit_DMA(option I2C_FIRST_AND_NEXT_FRAME then I2C_NEXT_FRAME)
                              or HAL_I2C_Master_Seq_Receive_DMA(option I2C_FIRST_AND_NEXT_FRAME then I2C_NEXT_FRAME).
                            Then usage of this option I2C_LAST_FRAME_NO_STOP at the last Transmit or Receive sequence permit to call the oposite interface Receive or Transmit
                              without stopping the communication and so generate a restart condition.
      (++) I2C_OTHER_FRAME: Sequential usage (Master only), this option allow to manage a restart condition after each call of the same master sequential
                            interface.
                            Usage can, transfer several bytes one by one with a restart with slave address between each bytes using HAL_I2C_Master_Seq_Transmit_IT(option I2C_FIRST_FRAME then I2C_OTHER_FRAME)
                              or HAL_I2C_Master_Seq_Receive_IT(option I2C_FIRST_FRAME then I2C_OTHER_FRAME)
                              or HAL_I2C_Master_Seq_Transmit_DMA(option I2C_FIRST_FRAME then I2C_OTHER_FRAME)
                              or HAL_I2C_Master_Seq_Receive_DMA(option I2C_FIRST_FRAME then I2C_OTHER_FRAME).
                            Then usage of this option I2C_OTHER_AND_LAST_FRAME at the last frame to help automatic generation of STOP condition.

      (+) Differents sequential I2C interfaces are listed below:
      (++) Sequential transmit in master I2C mode an amount of data in non-blocking mode using @ref HAL_I2C_Master_Seq_Transmit_IT()
            or using @ref HAL_I2C_Master_Seq_Transmit_DMA()
      (+++) At transmission end of current frame transfer, @ref HAL_I2C_MasterTxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_MasterTxCpltCallback()
      (++) Sequential receive in master I2C mode an amount of data in non-blocking mode using @ref HAL_I2C_Master_Seq_Receive_IT()
            or using @ref HAL_I2C_Master_Seq_Receive_DMA()
      (+++) At reception end of current frame transfer, @ref HAL_I2C_MasterRxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_MasterRxCpltCallback()
      (++) Abort a master IT or DMA I2C process communication with Interrupt using @ref HAL_I2C_Master_Abort_IT()
      (+++) End of abort process, @ref HAL_I2C_AbortCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_AbortCpltCallback()
      (++) Enable/disable the Address listen mode in slave I2C mode using @ref HAL_I2C_EnableListen_IT() @ref HAL_I2C_DisableListen_IT()
      (+++) When address slave I2C match, @ref HAL_I2C_AddrCallback() is executed and user can
           add his own code to check the Address Match Code and the transmission direction request by master (Write/Read).
      (+++) At Listen mode end @ref HAL_I2C_ListenCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_ListenCpltCallback()
      (++) Sequential transmit in slave I2C mode an amount of data in non-blocking mode using @ref HAL_I2C_Slave_Seq_Transmit_IT()
            or using @ref HAL_I2C_Slave_Seq_Transmit_DMA()
      (+++) At transmission end of current frame transfer, @ref HAL_I2C_SlaveTxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_SlaveTxCpltCallback()
      (++) Sequential receive in slave I2C mode an amount of data in non-blocking mode using @ref HAL_I2C_Slave_Seq_Receive_IT()
            or using @ref HAL_I2C_Slave_Seq_Receive_DMA()
      (+++) At reception end of current frame transfer, @ref HAL_I2C_SlaveRxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_SlaveRxCpltCallback()
      (++) In case of transfer Error, @ref HAL_I2C_ErrorCallback() function is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_ErrorCallback()
      (++) Discard a slave I2C process communication using @ref __HAL_I2C_GENERATE_NACK() macro.
           This action will inform Master to generate a Stop condition to discard the communication.

    *** Interrupt mode IO MEM operation ***
    =======================================
    [..]
      (+) Write an amount of data in non-blocking mode with Interrupt to a specific memory address using
          @ref HAL_I2C_Mem_Write_IT()
      (+) At Memory end of write transfer, @ref HAL_I2C_MemTxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_MemTxCpltCallback()
      (+) Read an amount of data in non-blocking mode with Interrupt from a specific memory address using
          @ref HAL_I2C_Mem_Read_IT()
      (+) At Memory end of read transfer, @ref HAL_I2C_MemRxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_MemRxCpltCallback()
      (+) In case of transfer Error, @ref HAL_I2C_ErrorCallback() function is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_ErrorCallback()

    *** DMA mode IO operation ***
    ==============================
    [..]
      (+) Transmit in master mode an amount of data in non-blocking mode (DMA) using
          @ref HAL_I2C_Master_Transmit_DMA()
      (+) At transmission end of transfer, @ref HAL_I2C_MasterTxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_MasterTxCpltCallback()
      (+) Receive in master mode an amount of data in non-blocking mode (DMA) using
          @ref HAL_I2C_Master_Receive_DMA()
      (+) At reception end of transfer, @ref HAL_I2C_MasterRxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_MasterRxCpltCallback()
      (+) Transmit in slave mode an amount of data in non-blocking mode (DMA) using
          @ref HAL_I2C_Slave_Transmit_DMA()
      (+) At transmission end of transfer, @ref HAL_I2C_SlaveTxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_SlaveTxCpltCallback()
      (+) Receive in slave mode an amount of data in non-blocking mode (DMA) using
          @ref HAL_I2C_Slave_Receive_DMA()
      (+) At reception end of transfer, @ref HAL_I2C_SlaveRxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_SlaveRxCpltCallback()
      (+) In case of transfer Error, @ref HAL_I2C_ErrorCallback() function is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_ErrorCallback()
      (+) Abort a master I2C process communication with Interrupt using @ref HAL_I2C_Master_Abort_IT()
      (+) End of abort process, @ref HAL_I2C_AbortCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_AbortCpltCallback()
      (+) Discard a slave I2C process communication using @ref __HAL_I2C_GENERATE_NACK() macro.
           This action will inform Master to generate a Stop condition to discard the communication.

    *** DMA mode IO MEM operation ***
    =================================
    [..]
      (+) Write an amount of data in non-blocking mode with DMA to a specific memory address using
          @ref HAL_I2C_Mem_Write_DMA()
      (+) At Memory end of write transfer, @ref HAL_I2C_MemTxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_MemTxCpltCallback()
      (+) Read an amount of data in non-blocking mode with DMA from a specific memory address using
          @ref HAL_I2C_Mem_Read_DMA()
      (+) At Memory end of read transfer, @ref HAL_I2C_MemRxCpltCallback() is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_MemRxCpltCallback()
      (+) In case of transfer Error, @ref HAL_I2C_ErrorCallback() function is executed and user can
           add his own code by customization of function pointer @ref HAL_I2C_ErrorCallback()


     *** I2C HAL driver macros list ***
     ==================================
     [..]
       Below the list of most used macros in I2C HAL driver.

      (+) @ref __HAL_I2C_ENABLE: Enable the I2C peripheral
      (+) @ref __HAL_I2C_DISABLE: Disable the I2C peripheral
      (+) @ref __HAL_I2C_GENERATE_NACK: Generate a Non-Acknowledge I2C peripheral in Slave mode
      (+) @ref __HAL_I2C_GET_FLAG: Check whether the specified I2C flag is set or not
      (+) @ref __HAL_I2C_CLEAR_FLAG: Clear the specified I2C pending flag
      (+) @ref __HAL_I2C_ENABLE_IT: Enable the specified I2C interrupt
      (+) @ref __HAL_I2C_DISABLE_IT: Disable the specified I2C interrupt

     *** Callback registration ***
     =============================================
    [..]
     The compilation flag USE_HAL_I2C_REGISTER_CALLBACKS when set to 1
     allows the user to configure dynamically the driver callbacks.
     Use Functions @ref HAL_I2C_RegisterCallback() or @ref HAL_I2C_RegisterAddrCallback()
     to register an interrupt callback.
    [..]
     Function @ref HAL_I2C_RegisterCallback() allows to register following callbacks:
       (+) MasterTxCpltCallback : callback for Master transmission end of transfer.
       (+) MasterRxCpltCallback : callback for Master reception end of transfer.
       (+) SlaveTxCpltCallback  : callback for Slave transmission end of transfer.
       (+) SlaveRxCpltCallback  : callback for Slave reception end of transfer.
       (+) ListenCpltCallback   : callback for end of listen mode.
       (+) MemTxCpltCallback    : callback for Memory transmission end of transfer.
       (+) MemRxCpltCallback    : callback for Memory reception end of transfer.
       (+) ErrorCallback        : callback for error detection.
       (+) AbortCpltCallback    : callback for abort completion process.
       (+) MspInitCallback      : callback for Msp Init.
       (+) MspDeInitCallback    : callback for Msp DeInit.
     This function takes as parameters the HAL peripheral handle, the Callback ID
     and a pointer to the user callback function.
    [..]
     For specific callback AddrCallback use dedicated register callbacks : @ref HAL_I2C_RegisterAddrCallback().
    [..]
     Use function @ref HAL_I2C_UnRegisterCallback to reset a callback to the default
     weak function.
     @ref HAL_I2C_UnRegisterCallback takes as parameters the HAL peripheral handle,
     and the Callback ID.
     This function allows to reset following callbacks:
       (+) MasterTxCpltCallback : callback for Master transmission end of transfer.
       (+) MasterRxCpltCallback : callback for Master reception end of transfer.
       (+) SlaveTxCpltCallback  : callback for Slave transmission end of transfer.
       (+) SlaveRxCpltCallback  : callback for Slave reception end of transfer.
       (+) ListenCpltCallback   : callback for end of listen mode.
       (+) MemTxCpltCallback    : callback for Memory transmission end of transfer.
       (+) MemRxCpltCallback    : callback for Memory reception end of transfer.
       (+) ErrorCallback        : callback for error detection.
       (+) AbortCpltCallback    : callback for abort completion process.
       (+) MspInitCallback      : callback for Msp Init.
       (+) MspDeInitCallback    : callback for Msp DeInit.
    [..]
     For callback AddrCallback use dedicated register callbacks : @ref HAL_I2C_UnRegisterAddrCallback().
    [..]
     By default, after the @ref HAL_I2C_Init() and when the state is @ref HAL_I2C_STATE_RESET
     all callbacks are set to the corresponding weak functions:
     examples @ref HAL_I2C_MasterTxCpltCallback(), @ref HAL_I2C_MasterRxCpltCallback().
     Exception done for MspInit and MspDeInit functions that are
     reset to the legacy weak functions in the @ref HAL_I2C_Init()/ @ref HAL_I2C_DeInit() only when
     these callbacks are null (not registered beforehand).
     If MspInit or MspDeInit are not null, the @ref HAL_I2C_Init()/ @ref HAL_I2C_DeInit()
     keep and use the user MspInit/MspDeInit callbacks (registered beforehand) whatever the state.
    [..]
     Callbacks can be registered/unregistered in @ref HAL_I2C_STATE_READY state only.
     Exception done MspInit/MspDeInit functions that can be registered/unregistered
     in @ref HAL_I2C_STATE_READY or @ref HAL_I2C_STATE_RESET state,
     thus registered (user) MspInit/DeInit callbacks can be used during the Init/DeInit.
     Then, the user first registers the MspInit/MspDeInit user callbacks
     using @ref HAL_I2C_RegisterCallback() before calling @ref HAL_I2C_DeInit()
     or @ref HAL_I2C_Init() function.
    [..]
     When the compilation flag USE_HAL_I2C_REGISTER_CALLBACKS is set to 0 or
     not defined, the callback registration feature is not available and all callbacks
     are set to the corresponding weak functions.

     [..]
       (@) You can refer to the I2C HAL driver header file for more useful macros

  @endverbatim
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2019 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "stm32mp1xx_hal.h"
#include "los_sys.h"

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/** @defgroup I2C_Private_Define I2C Private Define
  * @{
  */
#define TIMING_CLEAR_MASK   (0xF0FFFFFFU)  /*!< I2C TIMING clear register Mask */
#define I2C_TIMEOUT_ADDR    (10000U)       /*!< 10 s  */
#define I2C_TIMEOUT_BUSY    (25U)          /*!< 25 ms */
#define I2C_TIMEOUT_DIR     (25U)          /*!< 25 ms */
#define I2C_TIMEOUT_RXNE    (25U)          /*!< 25 ms */
#define I2C_TIMEOUT_STOPF   (25U)          /*!< 25 ms */
#define I2C_TIMEOUT_TC      (25U)          /*!< 25 ms */
#define I2C_TIMEOUT_TCR     (25U)          /*!< 25 ms */
#define I2C_TIMEOUT_TXIS    (25U)          /*!< 25 ms */
#define I2C_TIMEOUT_FLAG    (25U)          /*!< 25 ms */

#define MAX_NBYTE_SIZE      255U
#define SlaveAddr_SHIFT     7U
#define SlaveAddr_MSK       0x06U

/* Private define for @ref PreviousState usage */
#define I2C_STATE_MSK             ((uint32_t)((uint32_t)((uint32_t)HAL_I2C_STATE_BUSY_TX | (uint32_t)HAL_I2C_STATE_BUSY_RX) & (uint32_t)(~((uint32_t)HAL_I2C_STATE_READY)))) /*!< Mask State define, keep only RX and TX bits            */
#define I2C_STATE_NONE            ((uint32_t)(HAL_I2C_MODE_NONE))                                                        /*!< Default Value                                          */
#define I2C_STATE_MASTER_BUSY_TX  ((uint32_t)(((uint32_t)HAL_I2C_STATE_BUSY_TX & I2C_STATE_MSK) | (uint32_t)HAL_I2C_MODE_MASTER))            /*!< Master Busy TX, combinaison of State LSB and Mode enum */
#define I2C_STATE_MASTER_BUSY_RX  ((uint32_t)(((uint32_t)HAL_I2C_STATE_BUSY_RX & I2C_STATE_MSK) | (uint32_t)HAL_I2C_MODE_MASTER))            /*!< Master Busy RX, combinaison of State LSB and Mode enum */
#define I2C_STATE_SLAVE_BUSY_TX   ((uint32_t)(((uint32_t)HAL_I2C_STATE_BUSY_TX & I2C_STATE_MSK) | (uint32_t)HAL_I2C_MODE_SLAVE))             /*!< Slave Busy TX, combinaison of State LSB and Mode enum  */
#define I2C_STATE_SLAVE_BUSY_RX   ((uint32_t)(((uint32_t)HAL_I2C_STATE_BUSY_RX & I2C_STATE_MSK) | (uint32_t)HAL_I2C_MODE_SLAVE))             /*!< Slave Busy RX, combinaison of State LSB and Mode enum  */
#define I2C_STATE_MEM_BUSY_TX     ((uint32_t)(((uint32_t)HAL_I2C_STATE_BUSY_TX & I2C_STATE_MSK) | (uint32_t)HAL_I2C_MODE_MEM))               /*!< Memory Busy TX, combinaison of State LSB and Mode enum */
#define I2C_STATE_MEM_BUSY_RX     ((uint32_t)(((uint32_t)HAL_I2C_STATE_BUSY_RX & I2C_STATE_MSK) | (uint32_t)HAL_I2C_MODE_MEM))               /*!< Memory Busy RX, combinaison of State LSB and Mode enum */


/* Private define to centralize the enable/disable of Interrupts */
#define I2C_XFER_TX_IT          (0x00000001U)
#define I2C_XFER_RX_IT          (0x00000002U)
#define I2C_XFER_LISTEN_IT      (0x00000004U)

#define I2C_XFER_ERROR_IT       (0x00000011U)
#define I2C_XFER_CPLT_IT        (0x00000012U)
#define I2C_XFER_RELOAD_IT      (0x00000012U)

/* Private define Sequential Transfer Options default/reset value */
#define I2C_NO_OPTION_FRAME     (0xFFFF0000U)
/**
  * @}
  */
/**
  * @brief  Initializes the I2C according to the specified parameters
  *         in the I2C_InitTypeDef and initialize the associated handle.
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_I2C_Init(I2C_HandleTypeDef *hi2c)
{
  /* Check the I2C handle allocation */
  if (hi2c == NULL)
  {
    return HAL_ERROR;
  }


  if (hi2c->State == HAL_I2C_STATE_RESET)
  {
    /* Allocate lock resource and initialize it */
    hi2c->Lock = HAL_UNLOCKED;
  }

  hi2c->State = HAL_I2C_STATE_BUSY;

  /* Disable the selected I2C peripheral */
  __HAL_I2C_DISABLE(hi2c);

  /*---------------------------- I2Cx TIMINGR Configuration ------------------*/
  /* Configure I2Cx: Frequency range */
  hi2c->Instance->TIMINGR = hi2c->Init.Timing & TIMING_CLEAR_MASK;

  /*---------------------------- I2Cx OAR1 Configuration ---------------------*/
  /* Disable Own Address1 before set the Own Address1 configuration */
  hi2c->Instance->OAR1 &= ~I2C_OAR1_OA1EN;

  /* Configure I2Cx: Own Address1 and ack own address1 mode */
  if (hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_7BIT)
  {
    hi2c->Instance->OAR1 = (I2C_OAR1_OA1EN | hi2c->Init.OwnAddress1);
  }
  else /* I2C_ADDRESSINGMODE_10BIT */
  {
    hi2c->Instance->OAR1 = (I2C_OAR1_OA1EN | I2C_OAR1_OA1MODE | hi2c->Init.OwnAddress1);
  }

  /*---------------------------- I2Cx CR2 Configuration ----------------------*/
  /* Configure I2Cx: Addressing Master mode */
  if (hi2c->Init.AddressingMode == I2C_ADDRESSINGMODE_10BIT)
  {
    hi2c->Instance->CR2 = (I2C_CR2_ADD10);
  }
  /* Enable the AUTOEND by default, and enable NACK (should be disable only during Slave process */
  hi2c->Instance->CR2 |= (I2C_CR2_AUTOEND | I2C_CR2_NACK);

  /*---------------------------- I2Cx OAR2 Configuration ---------------------*/
  /* Disable Own Address2 before set the Own Address2 configuration */
  hi2c->Instance->OAR2 &= ~I2C_DUALADDRESS_ENABLE;

  /* Configure I2Cx: Dual mode and Own Address2 */
  hi2c->Instance->OAR2 = (hi2c->Init.DualAddressMode | hi2c->Init.OwnAddress2 | (hi2c->Init.OwnAddress2Masks << 8));

  /*---------------------------- I2Cx CR1 Configuration ----------------------*/
  /* Configure I2Cx: Generalcall and NoStretch mode */
  hi2c->Instance->CR1 = (hi2c->Init.GeneralCallMode | hi2c->Init.NoStretchMode);

  /* Enable the selected I2C peripheral */
  __HAL_I2C_ENABLE(hi2c);

  hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
  hi2c->State = HAL_I2C_STATE_READY;
  hi2c->PreviousState = I2C_STATE_NONE;
  hi2c->Mode = HAL_I2C_MODE_NONE;

  return HAL_OK;
}

/**
  * @brief  DeInitialize the I2C peripheral.
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_I2C_DeInit(I2C_HandleTypeDef *hi2c)
{
  /* Check the I2C handle allocation */
  if (hi2c == NULL)
  {
    return HAL_ERROR;
  }

  hi2c->State = HAL_I2C_STATE_BUSY;

  /* Disable the I2C Peripheral Clock */
  __HAL_I2C_DISABLE(hi2c);

  hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
  hi2c->State = HAL_I2C_STATE_RESET;
  hi2c->PreviousState = I2C_STATE_NONE;
  hi2c->Mode = HAL_I2C_MODE_NONE;

  /* Release Lock */
  __HAL_UNLOCK(hi2c);

  return HAL_OK;
}

/**
  * @brief  I2C Tx data register flush process.
  * @param  hi2c I2C handle.
  * @retval None
  */
static void I2C_Flush_TXDR(I2C_HandleTypeDef *hi2c)
{
  /* If a pending TXIS flag is set */
  /* Write a dummy data in TXDR to clear it */
  if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_TXIS) != RESET)
  {
    hi2c->Instance->TXDR = 0x00U;
  }

  /* Flush TX register if not empty */
  if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_TXE) == RESET)
  {
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_TXE);
  }
}

/**
  * @brief  This function handles Acknowledge failed detection during an I2C Communication.
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @param  Timeout Timeout duration
  * @param  Tickstart Tick start value
  * @retval HAL status
  */
static HAL_StatusTypeDef I2C_IsAcknowledgeFailed(I2C_HandleTypeDef *hi2c, uint32_t Timeout, uint32_t Tickstart)
{
  if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_AF) == SET)
  {
    /* Wait until STOP Flag is reset */
    /* AutoEnd should be initiate after AF */
    while (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_STOPF) == RESET)
    {
      /* Check for the Timeout */
      if (Timeout != HAL_MAX_DELAY)
      {
        if (((LOS_TickCountGet() - Tickstart) > Timeout) || (Timeout == 0U))
        {
          hi2c->ErrorCode |= HAL_I2C_ERROR_TIMEOUT;
          hi2c->State = HAL_I2C_STATE_READY;
          hi2c->Mode = HAL_I2C_MODE_NONE;

          /* Process Unlocked */
          __HAL_UNLOCK(hi2c);

          return HAL_ERROR;
        }
      }
    }

    /* Clear NACKF Flag */
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_AF);

    /* Clear STOP Flag */
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_STOPF);

    /* Flush TX register */
    I2C_Flush_TXDR(hi2c);

    /* Clear Configuration Register 2 */
    I2C_RESET_CR2(hi2c);

    hi2c->ErrorCode |= HAL_I2C_ERROR_AF;
    hi2c->State = HAL_I2C_STATE_READY;
    hi2c->Mode = HAL_I2C_MODE_NONE;

    /* Process Unlocked */
    __HAL_UNLOCK(hi2c);

    return HAL_ERROR;
  }
  return HAL_OK;
}

/**
  * @brief  Handles I2Cx communication when starting transfer or during transfer (TC or TCR flag are set).
  * @param  hi2c I2C handle.
  * @param  DevAddress Specifies the slave address to be programmed.
  * @param  Size Specifies the number of bytes to be programmed.
  *   This parameter must be a value between 0 and 255.
  * @param  Mode New state of the I2C START condition generation.
  *   This parameter can be one of the following values:
  *     @arg @ref I2C_RELOAD_MODE Enable Reload mode .
  *     @arg @ref I2C_AUTOEND_MODE Enable Automatic end mode.
  *     @arg @ref I2C_SOFTEND_MODE Enable Software end mode.
  * @param  Request New state of the I2C START condition generation.
  *   This parameter can be one of the following values:
  *     @arg @ref I2C_NO_STARTSTOP Don't Generate stop and start condition.
  *     @arg @ref I2C_GENERATE_STOP Generate stop condition (Size should be set to 0).
  *     @arg @ref I2C_GENERATE_START_READ Generate Restart for read request.
  *     @arg @ref I2C_GENERATE_START_WRITE Generate Restart for write request.
  * @retval None
  */
static void I2C_TransferConfig(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t Size, uint32_t Mode, uint32_t Request)
{
  /* update CR2 register */
  MODIFY_REG(hi2c->Instance->CR2, ((I2C_CR2_SADD | I2C_CR2_NBYTES | I2C_CR2_RELOAD | I2C_CR2_AUTOEND | (I2C_CR2_RD_WRN & (uint32_t)(Request >> (31U - I2C_CR2_RD_WRN_Pos))) | I2C_CR2_START | I2C_CR2_STOP)), \
             (uint32_t)(((uint32_t)DevAddress & I2C_CR2_SADD) | (((uint32_t)Size << I2C_CR2_NBYTES_Pos) & I2C_CR2_NBYTES) | (uint32_t)Mode | (uint32_t)Request));
}

/**
  * @brief  This function handles I2C Communication Timeout.
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @param  Flag Specifies the I2C flag to check.
  * @param  Status The new Flag status (SET or RESET).
  * @param  Timeout Timeout duration
  * @param  Tickstart Tick start value
  * @retval HAL status
  */
static HAL_StatusTypeDef I2C_WaitOnFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Flag, FlagStatus Status, uint32_t Timeout, uint32_t Tickstart)
{
  while (__HAL_I2C_GET_FLAG(hi2c, Flag) == Status)
  {
    /* Check for the Timeout */
    if (Timeout != HAL_MAX_DELAY)
    {
      if (((LOS_TickCountGet() - Tickstart) > Timeout) || (Timeout == 0U))
      {
        hi2c->ErrorCode |= HAL_I2C_ERROR_TIMEOUT;
        hi2c->State = HAL_I2C_STATE_READY;
        hi2c->Mode = HAL_I2C_MODE_NONE;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);
        return HAL_ERROR;
      }
    }
  }
  return HAL_OK;
}

/**
  * @brief  This function handles I2C Communication Timeout for specific usage of TXIS flag.
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @param  Timeout Timeout duration
  * @param  Tickstart Tick start value
  * @retval HAL status
  */
static HAL_StatusTypeDef I2C_WaitOnTXISFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Timeout, uint32_t Tickstart)
{
  while (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_TXIS) == RESET)
  {
    /* Check if a NACK is detected */
    if (I2C_IsAcknowledgeFailed(hi2c, Timeout, Tickstart) != HAL_OK)
    {
      return HAL_ERROR;
    }

    /* Check for the Timeout */
    if (Timeout != HAL_MAX_DELAY)
    {
      if (((LOS_TickCountGet() - Tickstart) > Timeout) || (Timeout == 0U))
      {
        hi2c->ErrorCode |= HAL_I2C_ERROR_TIMEOUT;
        hi2c->State = HAL_I2C_STATE_READY;
        hi2c->Mode = HAL_I2C_MODE_NONE;
        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_ERROR;
      }
    }
  }
  return HAL_OK;
}

/**
  * @brief  This function handles I2C Communication Timeout for specific usage of STOP flag.
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @param  Timeout Timeout duration
  * @param  Tickstart Tick start value
  * @retval HAL status
  */
static HAL_StatusTypeDef I2C_WaitOnSTOPFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Timeout, uint32_t Tickstart)
{
  while (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_STOPF) == RESET)
  {
    /* Check if a NACK is detected */
    if (I2C_IsAcknowledgeFailed(hi2c, Timeout, Tickstart) != HAL_OK)
    {
      return HAL_ERROR;
    }

    /* Check for the Timeout */
    if (((LOS_TickCountGet() - Tickstart) > Timeout) || (Timeout == 0U))
    {
      hi2c->ErrorCode |= HAL_I2C_ERROR_TIMEOUT;
      hi2c->State = HAL_I2C_STATE_READY;
      hi2c->Mode = HAL_I2C_MODE_NONE;

      /* Process Unlocked */
      __HAL_UNLOCK(hi2c);

      return HAL_ERROR;
    }
  }
  return HAL_OK;
}

/**
  * @brief  This function handles I2C Communication Timeout for specific usage of RXNE flag.
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @param  Timeout Timeout duration
  * @param  Tickstart Tick start value
  * @retval HAL status
  */
static HAL_StatusTypeDef I2C_WaitOnRXNEFlagUntilTimeout(I2C_HandleTypeDef *hi2c, uint32_t Timeout, uint32_t Tickstart)
{
  while (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_RXNE) == RESET)
  {
    /* Check if a NACK is detected */
    if (I2C_IsAcknowledgeFailed(hi2c, Timeout, Tickstart) != HAL_OK)
    {
      return HAL_ERROR;
    }

    /* Check if a STOPF is detected */
    if (__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_STOPF) == SET)
    {
      /* Check if an RXNE is pending */
      /* Store Last receive data if any */
      if ((__HAL_I2C_GET_FLAG(hi2c, I2C_FLAG_RXNE) == SET) && (hi2c->XferSize > 0U))
      {
        /* Return HAL_OK */
        /* The Reading of data from RXDR will be done in caller function */
        return HAL_OK;
      }
      else
      {
        /* Clear STOP Flag */
        __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_STOPF);

        /* Clear Configuration Register 2 */
        I2C_RESET_CR2(hi2c);

        hi2c->ErrorCode = HAL_I2C_ERROR_NONE;
        hi2c->State = HAL_I2C_STATE_READY;
        hi2c->Mode = HAL_I2C_MODE_NONE;

        /* Process Unlocked */
        __HAL_UNLOCK(hi2c);

        return HAL_ERROR;
      }
    }

    /* Check for the Timeout */
    if (((LOS_TickCountGet() - Tickstart) > Timeout) || (Timeout == 0U))
    {
      hi2c->ErrorCode |= HAL_I2C_ERROR_TIMEOUT;
      hi2c->State = HAL_I2C_STATE_READY;

      /* Process Unlocked */
      __HAL_UNLOCK(hi2c);

      return HAL_ERROR;
    }
  }
  return HAL_OK;
}

/**
  * @brief  Transmits in master mode an amount of data in blocking mode.
  * @param  hi2c Pointer to a I2C_HandleTypeDef structure that contains
  *                the configuration information for the specified I2C.
  * @param  DevAddress Target device address: The device 7 bits address value
  *         in datasheet must be shifted to the left before calling the interface
  * @param  pData Pointer to data buffer
  * @param  Size Amount of data to be sent
  * @param  Timeout Timeout duration
  * @retval HAL status
  */
HAL_StatusTypeDef HAL_I2C_Master_Transmit(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
    uint32_t tickstart;

    if (hi2c->State == HAL_I2C_STATE_READY)
    {
      if(hi2c == NULL)
      {
          return -1;
      }
      /* Process Locked */
      __HAL_LOCK(hi2c);

      /* Init tickstart for timeout management*/
      tickstart = LOS_TickCountGet();

      if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_BUSY, SET, I2C_TIMEOUT_BUSY, tickstart) != HAL_OK)
      {
        return HAL_ERROR;
      }

      hi2c->State     = HAL_I2C_STATE_BUSY_TX;
      hi2c->Mode      = HAL_I2C_MODE_MASTER;
      hi2c->ErrorCode = HAL_I2C_ERROR_NONE;

      /* Prepare transfer parameters */
      hi2c->pBuffPtr  = pData;
      hi2c->XferCount = Size;
      hi2c->XferISR   = NULL;

      /* Send Slave Address */
      /* Set NBYTES to write and reload if hi2c->XferCount > MAX_NBYTE_SIZE and generate RESTART */
      if (hi2c->XferCount > MAX_NBYTE_SIZE)
      {
        hi2c->XferSize = MAX_NBYTE_SIZE;
        I2C_TransferConfig(hi2c, DevAddress, (uint8_t)hi2c->XferSize, I2C_RELOAD_MODE, I2C_GENERATE_START_WRITE);
      }
      else
      {
        hi2c->XferSize = hi2c->XferCount;
        I2C_TransferConfig(hi2c, DevAddress, (uint8_t)hi2c->XferSize, I2C_AUTOEND_MODE, I2C_GENERATE_START_WRITE);
      }
      while (hi2c->XferCount > 0U)
      {
          /* Wait until TXIS flag is set */
          if (I2C_WaitOnTXISFlagUntilTimeout(hi2c, Timeout, tickstart) != HAL_OK)
          {
              return HAL_ERROR;
          }
          /* Write data to TXDR */
          hi2c->Instance->TXDR = *hi2c->pBuffPtr;

          /* Increment Buffer pointer */
          hi2c->pBuffPtr++;

          hi2c->XferCount--;
          hi2c->XferSize--;

          if ((hi2c->XferCount != 0U) && (hi2c->XferSize == 0U))
          {
              /* Wait until TCR flag is set */
              if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_TCR, RESET, Timeout, tickstart) != HAL_OK)
              {
                  return HAL_ERROR;
              }

              
              if (hi2c->XferCount > MAX_NBYTE_SIZE)
              {
                  hi2c->XferSize = MAX_NBYTE_SIZE;
                  I2C_TransferConfig(hi2c, DevAddress, (uint8_t)hi2c->XferSize, I2C_RELOAD_MODE, I2C_NO_STARTSTOP);
              }
              else
              {
                  hi2c->XferSize = hi2c->XferCount;
                  I2C_TransferConfig(hi2c, DevAddress, (uint8_t)hi2c->XferSize, I2C_AUTOEND_MODE, I2C_NO_STARTSTOP);
              }
          }
      }
      /* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
      /* Wait until STOPF flag is set */
      if (I2C_WaitOnSTOPFlagUntilTimeout(hi2c, Timeout, tickstart) != HAL_OK)
      {
        return HAL_ERROR;
      }

      /* Clear STOP Flag */
      __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_STOPF);

      /* Clear Configuration Register 2 */
      I2C_RESET_CR2(hi2c);

      hi2c->State = HAL_I2C_STATE_READY;
      hi2c->Mode  = HAL_I2C_MODE_NONE;

      /* Process Unlocked */
      __HAL_UNLOCK(hi2c);
      
      return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

HAL_StatusTypeDef HAL_I2C_Master_Receive(I2C_HandleTypeDef *hi2c, uint16_t DevAddress, uint8_t *pData, uint16_t Size, uint32_t Timeout)
{
  uint32_t tickstart;
  
  if (hi2c->State == HAL_I2C_STATE_READY)
  {    
    /* Process Locked */
    __HAL_LOCK(hi2c);

    /* Init tickstart for timeout management*/
    tickstart = LOS_TickCountGet();

    if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_BUSY, SET, I2C_TIMEOUT_BUSY, tickstart) != HAL_OK)
    {
      return HAL_ERROR;
    }

    hi2c->State     = HAL_I2C_STATE_BUSY_RX;
    hi2c->Mode      = HAL_I2C_MODE_MASTER;
    hi2c->ErrorCode = HAL_I2C_ERROR_NONE;

    /* Prepare transfer parameters */
    hi2c->pBuffPtr  = pData;
    hi2c->XferCount = Size;
    hi2c->XferISR   = NULL;

    /* Send Slave Address */
    /* Set NBYTES to write and reload if hi2c->XferCount > MAX_NBYTE_SIZE and generate RESTART */
    if (hi2c->XferCount > MAX_NBYTE_SIZE)
    {
      hi2c->XferSize = MAX_NBYTE_SIZE;
      I2C_TransferConfig(hi2c, DevAddress, (uint8_t)hi2c->XferSize, I2C_RELOAD_MODE, I2C_GENERATE_START_READ);
    }
    else
    {
      hi2c->XferSize = hi2c->XferCount;
      I2C_TransferConfig(hi2c, DevAddress, (uint8_t)hi2c->XferSize, I2C_AUTOEND_MODE, I2C_GENERATE_START_READ);
    }
    while (hi2c->XferCount > 0U)
    {
        /* Wait until RXNE flag is set */
        if (I2C_WaitOnRXNEFlagUntilTimeout(hi2c, Timeout, tickstart) != HAL_OK)
        {
            return HAL_ERROR;
        }
        
        /* Read data from RXDR */
        *hi2c->pBuffPtr = (uint8_t)hi2c->Instance->RXDR;

        /* Increment Buffer pointer */
        hi2c->pBuffPtr++;

        hi2c->XferSize--;
        hi2c->XferCount--;

        if ((hi2c->XferCount != 0U) && (hi2c->XferSize == 0U))
        {
            /* Wait until TCR flag is set */
            if (I2C_WaitOnFlagUntilTimeout(hi2c, I2C_FLAG_TCR, RESET, Timeout, tickstart) != HAL_OK)
            {
                return HAL_ERROR;
            }

            if (hi2c->XferCount > MAX_NBYTE_SIZE)
            {
                hi2c->XferSize = MAX_NBYTE_SIZE;
                I2C_TransferConfig(hi2c, DevAddress, (uint8_t)hi2c->XferSize, I2C_RELOAD_MODE, I2C_NO_STARTSTOP);
            }
            else
            {
                hi2c->XferSize = hi2c->XferCount;
                I2C_TransferConfig(hi2c, DevAddress, (uint8_t)hi2c->XferSize, I2C_AUTOEND_MODE, I2C_NO_STARTSTOP);
            }
        }
    }

    /* No need to Check TC flag, with AUTOEND mode the stop is automatically generated */
    /* Wait until STOPF flag is set */
    if (I2C_WaitOnSTOPFlagUntilTimeout(hi2c, Timeout, tickstart) != HAL_OK)
    {
      return HAL_ERROR;
    }

    /* Clear STOP Flag */
    __HAL_I2C_CLEAR_FLAG(hi2c, I2C_FLAG_STOPF);

    /* Clear Configuration Register 2 */
    I2C_RESET_CR2(hi2c);

    hi2c->State = HAL_I2C_STATE_READY;
    hi2c->Mode  = HAL_I2C_MODE_NONE;

    /* Process Unlocked */
    __HAL_UNLOCK(hi2c);

    return HAL_OK;
  }
  else
  {
    return HAL_BUSY;
  }
}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
