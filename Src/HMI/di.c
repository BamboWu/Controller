#include "di.h"

/**@brief  初始化数字输入控制端的函数
 */
void di_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct = {0}; //GPIO初始化用到的结构体

  __HAL_RCC_GPIOC_CLK_ENABLE();  //打开GPIOC口的时钟

  GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;      //输入
  GPIO_InitStruct.Pull  = GPIO_PULLUP;          //上拉
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; //高速IO 50MHz
  GPIO_InitStruct.Pin   = GPIO_PIN_10|
	                  GPIO_PIN_11|
			  GPIO_PIN_12|
			  GPIO_PIN_13|
			  GPIO_PIN_14|
			  GPIO_PIN_15;          //引脚号

  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);       //初始化DI引脚
}

/**@brief  处理数字输入的主函数
 */
void di_main(void)
{
  uint8_t di;
  uint8_t channel;
  extern uint8_t  prog_status; //程序运行状态字
  extern uint8_t  test_channel;//进行测试的通道
  extern valve_param_t valve_params[CHANNEL_NUM+1];
  extern uint16_t high_left[CHANNEL_NUM+1]; //高压剩余ms数，第0元素弃用
  uint8_t test_chan_old = 0;   //之前在测试的通道

  di = ~(GPIOC->IDR & 0x0000FC00) >> 10;

  //紧急停止的循环
  do{
	if(di & PROG_DI_STOP)
	{
	    HAL_NVIC_DisableIRQ(EXTI15_10_IRQn); //关闭编码器Z相中断
	    HAL_NVIC_DisableIRQ(TIM8_CC_IRQn);   //关闭编码器AB相中断
	    
	    //关闭所有通道输出
	    for(channel=1;channel<=CHANNEL_NUM;channel++)
	    {
		valve_channel_off(channel,1);
    		valve_channel_off(channel,0);
	    }

	    prog_status |= PROG_STOP_MSK;
	    di = ~(GPIOC->IDR & 0x0000FC00) >> 10;
	}
	else if(di & PROG_DI_TEST)
	{
	    HAL_NVIC_DisableIRQ(EXTI15_10_IRQn); //关闭编码器Z相中断
	    HAL_NVIC_DisableIRQ(TIM8_CC_IRQn);   //关闭编码器AB相中断

	    test_channel = (di & 0X3C)>>2;
	    if(test_chan_old != test_channel) //第一次进入测试或测试通道改变
	    {
		//关闭所有通道输出
	        for(channel=1;channel<=CHANNEL_NUM;channel++)
	        {
	            valve_channel_off(channel,1);
    	            valve_channel_off(channel,0);
	        }
		valve_channel_on(test_channel);
		high_left[test_channel] = valve_params[test_channel].high_duration;
		test_chan_old = test_channel;//更新测试通道
	    }

	    prog_status &= ~PROG_STOP_MSK;//清除停止位
	    prog_status |= PROG_TEST_MSK;
	    di = ~(GPIOC->IDR & 0x0000FC00) >> 10;
	}
	else if(prog_status & (PROG_STOP_MSK|PROG_TEST_MSK))//退出前
	{
	    prog_status &= ~(PROG_STOP_MSK|PROG_TEST_MSK);//清除停止测试位
            HAL_NVIC_EnableIRQ(EXTI15_10_IRQn); //开启编码器Z相中断
            HAL_NVIC_EnableIRQ(TIM8_CC_IRQn);   //开启编码器AB相中断
            //关闭所有通道输出
            for(channel=1;channel<=CHANNEL_NUM;channel++)
            {
                valve_channel_off(channel,1);
              	valve_channel_off(channel,0);
            }
	}
  }while(prog_status & (PROG_STOP_MSK|PROG_TEST_MSK));

}
