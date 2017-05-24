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
#include "SEGGER_RTT.h"

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
  
  if(TIM7 == htim->Instance)
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

}
/**
  * @brief Initializes the TIM Encoder Interface MSP
  * @param htim: TIM handle pointer
  * @retval None
  */
void HAL_TIM_Encoder_MspInit(TIM_HandleTypeDef *htim)
{
  GPIO_InitTypeDef  GPIO_InitStruct = {0}; //GPIO初始化用到的结构体
  
  if(TIM8 == htim->Instance)
  {
      /*##-1- Enable peripheral clock #############################*/
      /* TIM8 Peripheral clock enable */
      __HAL_RCC_AFIO_CLK_ENABLE();   //打开AFIO引脚功能复用的时钟
      __HAL_RCC_TIM8_CLK_ENABLE();   //打开TIM8的时钟
      __HAL_RCC_GPIOC_CLK_ENABLE();  //打开GPIOC口的时钟
  
      /*##-2- Configure the NVIC for TIM8 ####################################*/
      /* Set the TIM8 priority */
      //HAL_NVIC_SetPriority(TIM8_CC_IRQn, 2, 3); 

      /* Enable the TIM8 Capture/Compare Interrupt */
      //HAL_NVIC_EnableIRQ(TIM8_CC_IRQn); 

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
  * @brief  Initializes the TIM Output Compare MSP.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_OC_MspInit(TIM_HandleTypeDef *htim)
{
  if(TIM8 == htim->Instance)
  {
      /*##-1- Enable peripheral clock #############################*/
      /* TIM8 Peripheral clock enable */
      __HAL_RCC_AFIO_CLK_ENABLE();   //打开AFIO引脚功能复用的时钟
      __HAL_RCC_TIM8_CLK_ENABLE();   //打开TIM8的时钟
  
      /*##-2- Configure the NVIC for TIM8 ####################################*/
      /* Set the TIM8 priority */
      HAL_NVIC_SetPriority(TIM8_CC_IRQn, 2, 3); 

      /* Enable the TIM8 Capture/Compare Interrupt */
      HAL_NVIC_EnableIRQ(TIM8_CC_IRQn); 

  }
}
/**
  * @brief  Initializes the TIM One Pulse MSP.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_OnePulse_MspInit(TIM_HandleTypeDef *htim)
{
  if(TIM6 == htim->Instance)
  {
      /*##-1- Enable peripheral clock #############################*/
      /* TIM6 Peripheral clock enable */
      __HAL_RCC_TIM6_CLK_ENABLE();   //打开TIM6的时钟
  
      /*##-2- Configure the NVIC for TIM6 ####################################*/
      /* Set the TIM6 priority */
      HAL_NVIC_SetPriority(TIM6_IRQn, 0, 1); 

      /* Enable the TIM6 Global Interrupt */
      HAL_NVIC_EnableIRQ(TIM6_IRQn); 
  }
}
/**
  * @brief UART MSP Initialization 
  *        This function configures the hardware resources used in this example: 
  *           - Peripheral's clock enable
  *           - Peripheral's GPIO Configuration  
  *           - NVIC configuration for UART interrupt request enable
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{  
  GPIO_InitTypeDef  GPIO_InitStruct;

  if(USART1 == huart->Instance)
  {
      /*##-1- Enable peripherals and GPIO Clocks ############################*/
      /* Enable GPIO TX/RX clock */
      __HAL_RCC_GPIOA_CLK_ENABLE();

      /* Enable AFIO clock */
      __HAL_RCC_AFIO_CLK_ENABLE();

      /* Enable USARTx clock */
      __HAL_RCC_USART1_CLK_ENABLE(); 
      
      /*##-2- Configure peripheral GPIO #####################################*/  
      /* UART TX GPIO pin configuration  */
      GPIO_InitStruct.Pin       = GPIO_PIN_9;
      GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull      = GPIO_PULLUP;
      GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;

      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);//把PA9设为复用推挽

      /* UART RX GPIO pin configuration  */
      GPIO_InitStruct.Pin       = GPIO_PIN_10;
      GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;

      HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);//把PA10设为上拉输入

      /*##-3- Configure the NVIC for UART ###################################*/
      /* NVIC for USART */
      HAL_NVIC_SetPriority(USART1_IRQn, 0, 3);
      HAL_NVIC_EnableIRQ(USART1_IRQn);
  }
  if(USART3 == huart->Instance)
  {
      /*##-1- Enable peripherals and GPIO Clocks ############################*/
      /* Enable GPIO TX/RX clock */
      __HAL_RCC_GPIOB_CLK_ENABLE();

      /* Enable AFIO clock */
      __HAL_RCC_AFIO_CLK_ENABLE();

      /* Enable USARTx clock */
      __HAL_RCC_USART3_CLK_ENABLE(); 
      
      /*##-2- Configure peripheral GPIO #####################################*/  
      GPIO_InitStruct.Pin       = GPIO_PIN_10;
      GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
      GPIO_InitStruct.Pull      = GPIO_PULLUP;
      GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;

      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);//把PB10设为复用推挽
        
      GPIO_InitStruct.Pin       = GPIO_PIN_11;
      GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;

      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);//把PB11设为上拉输入

      /* 485 RDDE GPIO pin configuration  */
      GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;  //推挽输出
      GPIO_InitStruct.Pull  = GPIO_PULLUP;          //上拉
      GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; //高速IO 50MHz
      GPIO_InitStruct.Pin   = GPIO_PIN_12;          //引脚号

      HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); //初始化PB12引脚

      HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);  //PB12输出低电平，使能读

      /*##-3- Configure the NVIC for UART ###################################*/
      /* NVIC for USART */
      HAL_NVIC_SetPriority(USART3_IRQn, 0, 3);
      HAL_NVIC_EnableIRQ(USART3_IRQn);
  }
}

/**
  * @brief UART MSP De-Initialization 
  *        This function frees the hardware resources used in this example:
  *          - Disable the Peripheral's clock
  *          - Revert GPIO and NVIC configuration to their default state
  * @param huart: UART handle pointer
  * @retval None
  */
void HAL_UART_MspDeInit(UART_HandleTypeDef *huart)
{
  if(USART1 == huart->Instance)
  {
      /*##-1- Reset peripherals #############################################*/
      __HAL_RCC_USART1_FORCE_RESET();
      __HAL_RCC_USART1_RELEASE_RESET();

      /*##-2- Disable peripherals and GPIO Clocks ###########################*/
      /* Configure UART Tx as alternate function  */
      HAL_GPIO_DeInit(GPIOA, GPIO_PIN_9);
      /* Configure UART Rx as alternate function  */
      HAL_GPIO_DeInit(GPIOA, GPIO_PIN_10);
      
      /*##-3- Disable the NVIC for UART #####################################*/
      HAL_NVIC_DisableIRQ(USART1_IRQn);
  }
  if(USART3 == huart->Instance)
  {
      /*##-1- Reset peripherals #############################################*/
      __HAL_RCC_USART3_FORCE_RESET();
      __HAL_RCC_USART3_RELEASE_RESET();

      /*##-2- Disable peripherals and GPIO Clocks ###########################*/
      /* Configure UART Tx as alternate function  */
      HAL_GPIO_DeInit(GPIOB, GPIO_PIN_10);
      /* Configure UART Rx as alternate function  */
      HAL_GPIO_DeInit(GPIOB, GPIO_PIN_11);
      /* Configure RE/DE as alternate function  */
      HAL_GPIO_DeInit(GPIOB, GPIO_PIN_12);
      
      /*##-3- Disable the NVIC for UART #####################################*/
      HAL_NVIC_DisableIRQ(USART3_IRQn);
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
