#include "coder.h"

TIM_HandleTypeDef Tim8Handle;             //TIM8的结构体

uint16_t coder_division;

/**@brief 用于初始化编码器交互的函数
 * 
 * @param[in]  div        用于适配编码器的分度，即AB相每转各有几次脉冲；
 * @param[in]  dir        用于适配转动方向的参数，改变真值后对角度的测量反向；
 */
void coder_init(uint16_t div, uint8_t dir)
{
  GPIO_InitTypeDef  GPIO_InitStruct = {0}; //GPIO初始化用到的结构体
  TIM_Encoder_InitTypeDef  CoderConfig;    //用于配置编码器接口的结构体
  TIM_OC_InitTypeDef OCConfig;             //用于配置比较输出的结构体

  coder_division = div << 2; //4*div 因为在AB相脉冲的上升下降沿都计数，故为四倍

  Tim8Handle.Instance = TIM8;
  Tim8Handle.Init.Period            = (div << 2);//4*div
  Tim8Handle.Init.Prescaler         = 0;
  Tim8Handle.Init.ClockDivision     = 0;
  Tim8Handle.Init.CounterMode       = TIM_COUNTERMODE_UP;
  Tim8Handle.Init.RepetitionCounter = 0;

  CoderConfig.EncoderMode  = TIM_ENCODERMODE_TI12;             //双通道计数
  CoderConfig.IC1Polarity  = TIM_INPUTCHANNELPOLARITY_RISING;  //上升沿计数
  CoderConfig.IC1Selection = TIM_ICSELECTION_DIRECTTI;         //TI1和TI2不交换
  CoderConfig.IC1Prescaler = TIM_ICPSC_DIV1;                   //无预分频
  CoderConfig.IC1Filter    = 0x0;                              //不做滤波
  CoderConfig.IC2Polarity  = dir?                              //适配转动方向
			     TIM_INPUTCHANNELPOLARITY_RISING:  //上升沿计数
			     TIM_INPUTCHANNELPOLARITY_FALLING; //下降沿计数
  CoderConfig.IC2Selection = TIM_ICSELECTION_DIRECTTI;         //TI1和TI2不交换
  CoderConfig.IC2Prescaler = TIM_ICPSC_DIV1;                   //无预分频
  CoderConfig.IC2Filter    = 0x0;                              //不做滤波

  OCConfig.OCMode       = TIM_OCMODE_TIMING;                   //不做实际的输出
  OCConfig.Pulse        = div << 2;

  HAL_TIM_Encoder_Init(&Tim8Handle, &CoderConfig);
  Tim8Handle.State = HAL_TIM_STATE_RESET;//这样可以让做OC_Init时再进OC_Msp函数
  HAL_TIM_OC_Init(&Tim8Handle);
  HAL_TIM_OC_ConfigChannel(&Tim8Handle,&OCConfig,TIM_CHANNEL_3);

  __HAL_RCC_GPIOB_CLK_ENABLE();  //打开GPIOB口的时钟

  // Initialize Z
  GPIO_InitStruct.Mode  = GPIO_MODE_IT_RISING;  //上升沿触发外部中断
  GPIO_InitStruct.Pull  = GPIO_NOPULL;          //无上拉下拉
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; //高速IO 50MHz
  GPIO_InitStruct.Pin   = GPIO_PIN_15;          //引脚号
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);       //初始化PB15引脚
  /* Set the EXTI15 priority */
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 1, 3); 
  /* Enable the EXTI15 Interrupt */
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); 

  //HAL_TIM_Encoder_Start(&Tim8Handle,TIM_CHANNEL_1);
  //HAL_TIM_Encoder_Start(&Tim8Handle,TIM_CHANNEL_2);
}

/**@brief 用于启用编码器接口的函数
 */
void coder_Z(void)
{
  HAL_TIM_Encoder_Stop(&Tim8Handle,TIM_CHANNEL_12);
  Tim8Handle.Instance->CNT = 0;//清零重启
  Tim8Handle.Instance->CCR3 = 3900;//设置第一个事件点
  HAL_TIM_OC_Start_IT(&Tim8Handle,TIM_CHANNEL_3);
  HAL_TIM_Encoder_Start(&Tim8Handle,TIM_CHANNEL_12);
}

