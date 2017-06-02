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
  //uint8_t i=0;
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
  coder_init(1000,1);//1000P/R的编码器，编码器露出的轴朝自己，顺时针转计数增加
  hmi_init();

  //cmd_h();//打印一遍命令行交互界面的帮助信息
  /* -3- Toggle IO in an infinite loop */
  while (1)
  {
    /* Insert delay 100 ms */
    //HAL_Delay(100);
    //SEGGER_RTT_printf(0,"\r\n[loop%3d]\r\n",i++);
    //cmd_main();
    hmi_main();
    //hmi_test();
    //hmi_test_485();
  }  //while(1)
}  //main()

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
  oscinitstruct.HSIState        = RCC_HSI_ON;//要操作FLASH，必须开启内部时钟
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
  //flash操作延时两个cycle，以便适应频率高达72MHz的CPU时钟
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
