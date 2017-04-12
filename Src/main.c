/**
  ******************************************************************************
  * @file    GPIO/GPIO_IOToggle/Src/main.c
  * @author  MCD Application Team
  * @version V1.4.0
  * @date    29-April-2016
  * @brief   This example describes how to configure and use GPIOs through
  *          the STM32F1xx HAL API.
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
TIM_HandleTypeDef Tim7Handle;             //TIM7的结构体

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);            //系统时钟配置函数
void Indication_Config(void);             //程序运行指示灯配置函数

/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  uint8_t i=0;
  /* This sample code shows how to use GPIO HAL API to toggle LED2 IO
    in an infinite loop. */

  /* STM32F103xB HAL library initialization:
       - Configure the Flash prefetch
       - Systick timer is configured by default as source of time base, but user 
         can eventually implement his proper time base source (a general purpose 
         timer for example or other time source), keeping in mind that Time base 
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and 
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
  if(HAL_Init()!=HAL_OK)
	  SEGGER_RTT_printf(0,"\r\n[HAL_Init]error\r\n");//HAL初始化失败打印信息

  /* Configure the system clock to 72 MHz */
  SystemClock_Config();

  Indication_Config();

  valve_init();
  coder_init(2000,0);

  SEGGER_RTT_printf(0,"\r\n[init]OK\r\n");//打印信息提示初始化完成
  /* -3- Toggle IO in an infinite loop */
  while (1)
  {
    char key;
    uint8_t channel = 0;
    /* Insert delay 100 ms */
    //HAL_Delay(100);
    SEGGER_RTT_printf(0,"\r\n[loop%d]\r\nChannel:",i++);
    key = SEGGER_RTT_WaitKey();  //等待输入
    if('1'<=key&&key<='9')
	    channel = key - '0';
    else if('a'<=key&&key<='c')
	    channel = key - 'a' + 10;
    else if('A'<=key&&key<='C')
	    channel = key - 'A' + 10;
    else
	    continue;  //error to choose channel
    SEGGER_RTT_printf(0,"\r\nOn press O, Off press X:");
    key = SEGGER_RTT_WaitKey();  //等待输入
    if(key == 'o' || key == 'O')
	    valve_channel_on(channel);
    else if(key == 'x' || key == 'X')
    {
	    SEGGER_RTT_printf(0,"\r\nHigh press H, Low press L:");
	    key = SEGGER_RTT_WaitKey();
	    if(key == 'h' || key == 'H')
		    valve_channel_off(channel,1);
	    else if(key == 'l' || key == 'L')
		    valve_channel_off(channel,0);
	    else
		    continue;  //error to choose High or Low
    }
    else
	    continue;  //error to choose On or Off

  }
}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow : 
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 72000000
  *            HCLK(Hz)                       = 72000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 2
  *            APB2 Prescaler                 = 1
  *            PLLMUL                         = 9
  *            Flash Latency(WS)              = 2
  * @param  None
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef clkinitstruct = {0};
  RCC_OscInitTypeDef oscinitstruct = {0};
  
  /* Configure PLL ------------------------------------------------------*/
  /* PLL configuration: PLLCLK = HSE * PLLMUL = 8  * 9 = 72 MHz */
  /* PREDIV1 configuration: PREDIV1CLK = PLLCLK / HSEPredivValue = 72 / 1 = 72 MHz */
  /* Enable HSE and activate PLL with HSE_DIV1 as source */
  oscinitstruct.OscillatorType  = RCC_OSCILLATORTYPE_HSE;
  oscinitstruct.HSEState        = RCC_HSE_ON;
  oscinitstruct.LSEState        = RCC_LSE_OFF;
  oscinitstruct.HSIState        = RCC_HSI_OFF;
  oscinitstruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  oscinitstruct.HSEPredivValue  = RCC_HSE_PREDIV_DIV1;
  oscinitstruct.PLL.PLLState    = RCC_PLL_ON;
  oscinitstruct.PLL.PLLSource   = RCC_PLLSOURCE_HSE;
  oscinitstruct.PLL.PLLMUL      = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&oscinitstruct)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 
     clocks dividers */
  clkinitstruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  clkinitstruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  clkinitstruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  clkinitstruct.APB2CLKDivider = RCC_HCLK_DIV1;
  clkinitstruct.APB1CLKDivider = RCC_HCLK_DIV2;  
  if (HAL_RCC_ClockConfig(&clkinitstruct, FLASH_LATENCY_2)!= HAL_OK)
  {
    /* Initialization Error */
    while(1); 
  }
}

/**
  * @brief  运行指示灯配置
  *         运行指示灯（L6）以1Hz的频率闪烁，指示程序正在运行。
  *         使用的资源如下 : 
  *            PA15：配置推挽输出
  *            TIM7：预分频   72000000/60000 = 1200Hz
  *                  定时上溢 600
  *                  上溢中断频率 1200/600   = 2Hz
  *                  中断优先级  3，3（最低）
  * @param  None
  * @retval None
  */
void Indication_Config(void)
{
  /* Configure TIM  ------------------------------------------------------*/

  /* -1- Set TIM7 instance */
  Tim7Handle.Instance = TIM7;

  /* -2- Initialize TIM7 peripheral as follows:
       + Period = 600 - 1
       + Prescaler = 60000) - 1
       + ClockDivision = 0
       + Counter direction = Up
  */
  Tim7Handle.Init.Period            = 600 - 1;
  Tim7Handle.Init.Prescaler         = 60000 - 1;
  Tim7Handle.Init.ClockDivision     = 0;
  Tim7Handle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  Tim7Handle.Init.RepetitionCounter = 0;

  if (HAL_TIM_Base_Init(&Tim7Handle) != HAL_OK)
	  SEGGER_RTT_printf(0,"\r\n[Indication_Config]TIM Init fail\r\n");

  /* -3- Start the TIM Base generation in interrupt mode */
  if (HAL_TIM_Base_Start_IT(&Tim7Handle) != HAL_OK)
	  SEGGER_RTT_printf(0,"\r\n[Indication_Config]TIM IT Init fail\r\n");

}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {
  }
}
#endif

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
