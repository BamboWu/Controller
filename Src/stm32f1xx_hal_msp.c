/**
  ******************************************************************************
  * @file    stm32f1xx_hal_msp_template.c
  * @author  MCD Application Team
  * @version V1.0.4
  * @date    29-April-2016
  * @brief   HAL BSP module.
  *          This file template is located in the HAL folder and should be copied 
  *          to the user folder.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; COPYRIGHT(c) 2016 STMicroelectronics</center></h2>
  *
  * Redistribution and use in source and binary forms, with or without modification,
  * are permitted provided that the following conditions are met:
  *   1. Redistributions of source code must retain the above copyright notice,
  *      this list of conditions and the following disclaimer.
  *   2. Redistributions in binary form must reproduce the above copyright notice,
  *      this list of conditions and the following disclaimer in the documentation
  *      and/or other materials provided with the distribution.
  *   3. Neither the name of STMicroelectronics nor the names of its contributors
  *      may be used to endorse or promote products derived from this software
  *      without specific prior written permission.
  *
  * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
  * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
  * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
  * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
  * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
  * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
  * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
  * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
  * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
  * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
  *
  ******************************************************************************  
  */ 

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/** @addtogroup STM32F1xx_HAL_Driver
  * @{
  */

/** @defgroup HAL_MSP HAL_MSP
  * @brief HAL MSP module.
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/** @defgroup HAL_MSP_Exported_Functions HAL MSP Exported Functions
  * @{
  */

/**
  * @brief  Initializes the Global MSP.
  * @retval None
  */
void HAL_MspInit(void)
{

}

/**
  * @brief  DeInitializes the Global MSP.
  * @retval None
  */
void HAL_MspDeInit(void)
{

}

/**
  * @brief  Initializes the PPP MSP.
  * @retval None
  */
void HAL_PPP_MspInit(void)
{

}

/**
  * @brief  DeInitializes the PPP MSP.
  * @retval None
  */
void HAL_PPP_MspDeInit(void)
{

}

/**
  * @brief TIM MSP Initialization
  *        This function configures the hardware resources used in this example:
  *           - Peripheral's clock enable
  * @param htim: TIM handle pointer
  * @retval None
  */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
  GPIO_InitTypeDef  GPIO_InitStruct = {0}; //GPIO初始化用到的结构体
  
  if(htim->Instance == TIM7)
  { 
      /*##-1- Enable peripheral clock #############################*/
      /* TIM7 Peripheral clock enable */
      __HAL_RCC_TIM7_CLK_ENABLE();
      __HAL_RCC_GPIOA_CLK_ENABLE();  //打开GPIOA口的时钟
      __HAL_RCC_AFIO_CLK_ENABLE();   //打开AFIO复用功能管理器的时钟
      __HAL_AFIO_REMAP_SWJ_NOJTAG(); //禁用JTAG，这样PA15方可作为GPIO使用
  
      /*##-2- Configure the NVIC for TIM7 ####################################*/
      /* Set the TIM7 priority */
      HAL_NVIC_SetPriority(TIM7_IRQn, 3, 3);

      /* Enable the TIMx global Interrupt */
      HAL_NVIC_EnableIRQ(TIM7_IRQn);

      /*##-3- Configure GPIO #################################################*/
      /* Configure IO in output push-pull mode to drive external LEDs */
      GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;  //推挽输出
      GPIO_InitStruct.Pull  = GPIO_PULLUP;          //上拉
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; //高速IO 50MHz
      GPIO_InitStruct.Pin   = GPIO_PIN_15;          //引脚号

      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); //初始化PA15引脚

      HAL_GPIO_WritePin(GPIOA,GPIO_PIN_15,GPIO_PIN_RESET);  //PA15输出低电平，点亮L6
  }
  //else if(htim->Instance == TIM8)
  //{
  //    /*##-1- Enable peripheral clock #############################*/
  //    /* TIM8 Peripheral clock enable */
  //    __HAL_RCC_GPIOB_CLK_ENABLE();  //打开GPIOB口的时钟
  //    __HAL_RCC_AFIO_CLK_ENABLE();   //打开AFIO引脚功能复用的时钟
  //    __HAL_RCC_TIM8_CLK_ENABLE();   //打开TIM8的时钟
  //    __HAL_RCC_GPIOC_CLK_ENABLE();  //打开GPIOC口的时钟
  //
  //    /*##-2- Configure the NVIC for TIM8 ####################################*/
  //    /* Set the TIM8 priority */
  //    //HAL_NVIC_SetPriority(TIM8_IRQn, 0, 3); //TIM8是编码器模式工作的，无中断
  //    /* Set the EXTI15 priority */
  //    HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 3); 

  //    /* Enable the TIMx global Interrupt */
  //    //HAL_NVIC_EnableIRQ(TIM8_IRQn); //TIM8是编码器模式工作的，无中断
  //    /* Enable the EXTI15 Interrupt */
  //    HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); 
  //}

}
/**
  * @brief Initializes the TIM Encoder Interface MSP
  * @param htim: TIM handle pointer
  * @retval None
  */
void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef *htim)
{
  GPIO_InitTypeDef  GPIO_InitStruct = {0}; //GPIO初始化用到的结构体
  
  if(htim->Instance == TIM8)
  {
      /*##-1- Enable peripheral clock #############################*/
      /* TIM8 Peripheral clock enable */
      __HAL_RCC_AFIO_CLK_ENABLE();   //打开AFIO引脚功能复用的时钟
      __HAL_RCC_TIM8_CLK_ENABLE();   //打开TIM8的时钟
      __HAL_RCC_GPIOC_CLK_ENABLE();  //打开GPIOC口的时钟
  
      /*##-2- Configure the NVIC for TIM8 ####################################*/
      /* Set the TIM8 priority */
      HAL_NVIC_SetPriority(TIM8_CC_IRQn, 2, 3); 

      /* Enable the TIM8 Capture/Compare Interrupt */
      HAL_NVIC_EnableIRQ(TIM8_CC_IRQn); 

      /*##-3- Configure GPIO #################################################*/
      /* Configure IO in output push-pull mode to drive external LEDs */
      GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;      //输入
      GPIO_InitStruct.Pull  = GPIO_PULLDOWN;        //下拉
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; //高速IO 50MHz
      GPIO_InitStruct.Pin   = GPIO_PIN_6|GPIO_PIN_7;//引脚号

      HAL_GPIO_Init(GPIOC, &GPIO_InitStruct); //初始化PC15引脚
  }

}
/**
  * @}
  */

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
