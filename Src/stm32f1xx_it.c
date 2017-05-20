/** 
  ******************************************************************************
  * @file    GPIO/GPIO_IOToggle/Src/stm32f1xx_it.c
  * @author  MCD Application Team
  * @version V1.4.0
  * @date    29-April-2016
  * @brief   Main Interrupt Service Routines.
  *          This file provides template for all exceptions handler and
  *          peripherals interrupt service routine.
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
#include "main.h"
#include "stm32f1xx_it.h"

/** @addtogroup STM32F1xx_HAL_Examples
  * @{
  */

/** @addtogroup GPIO_IOToggle
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
extern TIM_HandleTypeDef Tim7Handle;
extern TIM_HandleTypeDef Tim8Handle;
#ifdef USE_UART1_232
extern UART_HandleTypeDef UART1Handle;
extern unsigned char UART1_STA;
extern unsigned char RxBuffer1[RXBUFFER_SIZE];
#endif
extern UART_HandleTypeDef UART3Handle;
extern unsigned char UART3_STA;
extern unsigned char RxBuffer[RXBUFFER_SIZE];

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

/******************************************************************************/
/*            Cortex-M3 Processor Exceptions Handlers                         */
/******************************************************************************/

/**
  * @brief  This function handles NMI exception.
  * @param  None
  * @retval None
  */
void NMI_Handler(void)
{
}

/**
  * @brief  This function handles Hard Fault exception.
  * @param  None
  * @retval None
  */
void HardFault_Handler(void)
{
  /* Go to infinite loop when Hard Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Memory Manage exception.
  * @param  None
  * @retval None
  */
void MemManage_Handler(void)
{
  /* Go to infinite loop when Memory Manage exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Bus Fault exception.
  * @param  None
  * @retval None
  */
void BusFault_Handler(void)
{
  /* Go to infinite loop when Bus Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles Usage Fault exception.
  * @param  None
  * @retval None
  */
void UsageFault_Handler(void)
{
  /* Go to infinite loop when Usage Fault exception occurs */
  while (1)
  {
  }
}

/**
  * @brief  This function handles SVCall exception.
  * @param  None
  * @retval None
  */
void SVC_Handler(void)
{
}

/**
  * @brief  This function handles Debug Monitor exception.
  * @param  None
  * @retval None
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  This function handles PendSVC exception.
  * @param  None
  * @retval None
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  This function handles SysTick Handler.
  * @param  None
  * @retval None
  */
void SysTick_Handler(void)
{
  HAL_IncTick();
}

/******************************************************************************/
/*                 STM32F1xx Peripherals Interrupt Handlers                   */
/*  Add here the Interrupt Handler for the used peripheral(s) (PPP), for the  */
/*  available peripheral interrupt handler's name please refer to the startup */
/*  file (startup_stm32f1xx.s).                                               */
/******************************************************************************/

/**
  * @brief  This function handles PPP interrupt request.
  * @param  None
  * @retval None
  */
/*void PPP_IRQHandler(void)
{
}*/

/**
  * @brief  Period elapsed callback in non blocking mode
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /*if (htim == &Tim8Handle)
  {
    SEGGER_RTT_printf(0,"[Tim8]%d arrive\r\n",Tim8Handle.Instance.CNT);
  }//if (htim == &Tim8Handle)
  else */if (htim == &Tim7Handle)
  {
    static char hsecond = 0;
    static char minute = 0;
    static char hour = 0;
    HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_15);//翻转PA15电平，使L6交替亮灭
    if(hsecond < 119)
	hsecond++;
    else
    {
	hsecond = 0;
	if(minute < 59)
	    minute++;
	else
	{
            minute = 0;
	    hour++;
	}
	SEGGER_RTT_printf(0,"\r\n%2dh%2dm\r\n",hour,minute);
    }
  }//else if (htim == &Tim7Handle)
}

/**
  * @brief  Input Capture callback in non blocking mode
  * @param  htim : TIM IC handle
  * @retval None
  */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
  /* Prevent unused argument(s) compilation warning */
  UNUSED(htim);
  /* NOTE : This function Should not be modified, when the callback is needed,
            the __HAL_TIM_OC_DelayElapsedCallback could be implemented in the user file
   */
  SEGGER_RTT_printf(0,"\r\n[IC_CaptureCallback]\r\n");
}

/**
  * @brief  Output Compare callback in non blocking mode 
  * @param  htim : TIM OC handle
  * @retval None
  */
void HAL_TIM_OC_DelayElapsedCallback(TIM_HandleTypeDef *htim)
{
  if (htim == &Tim8Handle)
  {
    //SEGGER_RTT_printf(0,"[Tim8]CNT = %d\r\n",htim->Instance->CNT);
    //htim->Instance->CCR3 += 1;
    extern coder_evt_t * pcoder_evt_now;
    if(pcoder_evt_now != NULL)
    {
	//SEGGER_RTT_printf(0,"[OC_handler]to do one evt\r\n");
	//SEGGER_RTT_WaitKey();
	switch(pcoder_evt_now->evt_type)
	{
	    case channel_on   :valve_channel_on(pcoder_evt_now->evt_channel);
			       break;
	    case channel_off_H:valve_channel_off(pcoder_evt_now->evt_channel,1);
				break;
	    case channel_off_L:valve_channel_off(pcoder_evt_now->evt_channel,0);
				break;
	    default:            break;
	}
	//SEGGER_RTT_printf(0,"[OC_handler]one evt OK\r\n");
	//SEGGER_RTT_WaitKey();
	while(pcoder_evt_now->next!=NULL) //还不是最后一个事件
	{
	    //SEGGER_RTT_printf(0,"[OC_handler]to do one more evt?\r\n");
	    //SEGGER_RTT_WaitKey();
	    if(pcoder_evt_now->coder_count<pcoder_evt_now->next->coder_count)
		break;//下一事件时间点更晚，就跳出
	    //下一个事件的时间点与当前事件相同
	    pcoder_evt_now = pcoder_evt_now->next;
	    switch(pcoder_evt_now->evt_type)
	    {
		case channel_on   :valve_channel_on(pcoder_evt_now->evt_channel);
				   break;
		case channel_off_H:valve_channel_off(pcoder_evt_now->evt_channel,1);
				   break;
		case channel_off_L:valve_channel_off(pcoder_evt_now->evt_channel,0);
				   break;
		default:           break;
	    }
	    //SEGGER_RTT_printf(0,"[OC_handler]one more evt OK\r\n");
	    //SEGGER_RTT_WaitKey();
	}
	//当前count点的事件已经全部执行完
	if(pcoder_evt_now->next!=NULL)//还不是最后一个事件
	    pcoder_evt_now = pcoder_evt_now->next;
	Tim8Handle.Instance->CCR3 = pcoder_evt_now->coder_count;
	//如果是最后一个事件，则该设置的中断count在达到之前先被coder_Z中覆盖
    }//if(pcoder_evt_now != NULL)
  }
}

/**
  * @brief  This function handles TIM7 interrupt request.
  * @param  None
  * @retval None
  */
void TIM7_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&Tim7Handle);
}

/**
  * @brief  This function handles TIM8 Capture/Compare interrupt request.
  * @param  None
  * @retval None
  */
void TIM8_CC_IRQHandler(void)
{
  HAL_TIM_IRQHandler(&Tim8Handle);
}

/**
  * @brief EXTI line detection callbacks
  * @param GPIO_Pin: Specifies the pins connected EXTI line
  * @retval None
  */
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
  if(GPIO_Pin == GPIO_PIN_15)
  {
    //SEGGER_RTT_printf(0,"\r\n[EXTI15]coder Z\r\n");
    coder_Z();
  }
}

/**
  * @brief  This function handles external lines 10 to 15 interrupt request.
  * @param  None
  * @retval None
  */
void EXTI15_10_IRQHandler(void)
{
  HAL_GPIO_EXTI_IRQHandler(GPIO_PIN_15);
}

/**
  * @brief  Tx Transfer completed callback
  * @param  UartHandle: UART handle. 
  * @note   This example shows a simple way to report end of IT Tx transfer, and 
  *         you can add your own implementation. 
  * @retval None
  */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *UartHandle)
{
#ifdef USE_UART1_232
  if(UartHandle == &UART1Handle)
  {
	  /* Set transmission flag: transfer complete */
	  UART1_STA |= TX_CPLT;
  }
#endif
  if(UartHandle == &UART3Handle)
  {
	  /* Set transmission flag: transfer complete */
	  UART3_STA |= TX_CPLT;
  }
  
}

/**
  * @brief  Rx Transfer completed callback
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report end of DMA Rx transfer, and 
  *         you can add your own implementation.
  * @retval None
  */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *UartHandle)
{
  //SEGGER_RTT_printf(0,"\r\n[RxCpltCallback]\r\n");
  
#ifdef USE_UART1_232
  if(UartHandle == &UART1Handle)
  {
	  /* Set transmission flag: receive complete */
	  UART1_STA |= RX_CPLT;
  }
#endif
  if(UartHandle == &UART3Handle)
  {
	  /* Set transmission flag: receive complete */
	  UART3_STA |= RX_CPLT;
  }
  
}

/**
  * @brief  UART error callbacks
  * @param  UartHandle: UART handle
  * @note   This example shows a simple way to report transfer error, and you can
  *         add your own implementation.
  * @retval None
  */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *UartHandle)
{
  SEGGER_RTT_printf(0,"\r\nUART_Error\r\n");
    //Error_Handler();
}
/**
  * @brief  This function handles UART interrupt request.  
  * @param  None
  * @retval None
  */
void USART1_IRQHandler(void)
{
  //SEGGER_RTT_printf(0,"\r\n[USART1_IRQ]\r\n");
#ifdef USE_UART1_232
  HAL_UART_IRQHandler(&UART1Handle);
#endif
}
/**
  * @brief  This function handles UART interrupt request.  
  * @param  None
  * @retval None
  */
void USART3_IRQHandler(void)
{
  //SEGGER_RTT_printf(0,"\r\n[USART3_IRQ]\r\n");
  HAL_UART_IRQHandler(&UART3Handle);
}
/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
