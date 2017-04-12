#include "coder.h"

//static uint8_t m_coder_state = 0;       //内部记录是否启动TIM8 encoder接口的量
TIM_HandleTypeDef Tim8Handle;             //TIM8的结构体

/**@brief 用于设置编码器的函数
 *
 * @param[in] bank      指定根据374芯片划分的区间，0对应芯片U14，1为U58，2是U912
 * @param[in] mask      指定要掩去哪几位，置1的那几位会保持原状态
 * @param[in] state     指定输出状态，1为输出
 */
/*
static void do_coder_set(uint8_t bank, uint8_t mask, uint8_t state)
{
  uint32_t mask32 = mask;
  uint32_t state32 = state;
  uint16_t toset16,toclr16;

  switch(bank)
  {
    case BANK0: mask32 |= 0XFFFFFF00;  //BANK0对应最低8个通路
		break;
    case BANK1: mask32 <<= 8;
		mask32 |= 0XFFFF00FF;  //BANK1对应中间8个通路
		state32 <<= 8;
		break;
    case BANK2: mask32 <<= 16;
		mask32 |= 0XFF00FFFF;  //BANK2对应最高8个通路
		state32 <<= 16;
		break;
    default   : mask32 = 0XFFFFFFFF;
  }
		state32 &= ~mask32;    //掩去所选BANK以外的新状态数据
  m_coder_state &= mask32;  //清零要修改的那几个通路的记录
  m_coder_state |= state32; //要修改的那几个通路记录新的状态
  
  SEGGER_RTT_printf(0,"\r\n[do_coder_set]m_coder_state=0X%8x\r\n",m_coder_state);

  switch(bank)
  {
    case BANK0: toset16 = m_coder_state & 0X000000FF;  //BANK0对应最低8个通路
		toclr16 = ~m_coder_state & 0X000000FF;
		break;
    case BANK1: toset16 = (m_coder_state & 0X0000FF00)>>8;  //BANK1对应中间8个通路
		toclr16 = (~m_coder_state & 0X0000FF00)>>8;
		break;
    case BANK2: toset16 = (m_coder_state & 0X00FF0000)>>16;  //BANK2对应最高8个通路
		toclr16 = (~m_coder_state & 0X00FF0000)>>16;
		break;
    default   : toset16 = toclr16 = 0X0000;
  }

  SEGGER_RTT_printf(0,"\r\n[do_coder_set]toset16=0X%4x,toclr16=0X%4x\r\n",toset16,toclr16);

  HAL_GPIO_WritePin(GPIOA,toclr16,GPIO_PIN_RESET); //将低电平的脚复位
  HAL_GPIO_WritePin(GPIOA,toset16,GPIO_PIN_SET);   //将高电平的脚置位

  switch(bank)
  {
    case BANK0: HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);//上升沿锁数据到U14
		break;
    case BANK1: HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);//上升沿锁数据到U58
		break;
    case BANK2: HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_SET);//上升沿锁数据到U912
		break;
    default   : HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2,GPIO_PIN_RESET);
  }
		HAL_Delay(1);//延时1ms
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2,GPIO_PIN_RESET); //确保CP0～CP2的信号平时保持低电平。
}
*/

/**@brief 用于初始化编码器交互的函数
 * 
 * @param[in]  div        用于适配编码器的分度；
 * @param[in]  dir        用于适配转动方向的参数，改变真值后对角度的测量反向；
 */
void coder_init(uint16_t div, uint8_t dir)
{
  GPIO_InitTypeDef  GPIO_InitStruct = {0}; //GPIO初始化用到的结构体
  TIM_Encoder_InitTypeDef  CoderConfig;    //用于配置编码器接口的结构体

  Tim8Handle.Instance = TIM8;
  Tim8Handle.Init.Period            = div - 1;
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

  HAL_TIM_Encoder_Init(&Tim8Handle, &CoderConfig);

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

}

/**@brief 用于启用编码器接口的函数
 */
void coder_on(void)
{
  HAL_TIM_Encoder_Stop_IT(&Tim8Handle,TIM_CHANNEL_1);
  HAL_TIM_Encoder_Stop_IT(&Tim8Handle,TIM_CHANNEL_2);
  Tim8Handle.Instance->CNT = 0;//清零重启
  HAL_TIM_Encoder_Start_IT(&Tim8Handle,TIM_CHANNEL_1);
  HAL_TIM_Encoder_Start_IT(&Tim8Handle,TIM_CHANNEL_2);
  //SEGGER_RTT_printf(0,"\r\n[coder_on]%d\r\n",Tim8Handle.Instance->CNT);
}

