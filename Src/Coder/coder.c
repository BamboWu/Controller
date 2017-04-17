#include "coder.h"

TIM_HandleTypeDef Tim8Handle;             //TIM8的结构体

coder_evt_t   coder_evts[CODER_EVT_MAX];
coder_evt_t * pcoder_evt_now   = NULL;
coder_evt_t * pcoder_evt_free  = NULL;
coder_evt_t * pcoder_evt_first = NULL;

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

  coder_evt_gather();
}

/**@brief 用于启用编码器接口的函数
 */
void coder_Z(void)
{
  HAL_TIM_Encoder_Stop(&Tim8Handle,TIM_CHANNEL_12);
  Tim8Handle.Instance->CNT = 0;//清零重启
  if(pcoder_evt_first != NULL)//设置第一个事件点
	Tim8Handle.Instance->CCR3 = pcoder_evt_first->coder_count;
  pcoder_evt_now = pcoder_evt_first;
  HAL_TIM_OC_Start_IT(&Tim8Handle,TIM_CHANNEL_3);
  HAL_TIM_Encoder_Start(&Tim8Handle,TIM_CHANNEL_12);
}

/**@breif 用于插入一个coder event到合适位置的函数
 */
void coder_evt_insert(coder_evt_t evt)
{
  coder_evt_t * p_coder_evt = pcoder_evt_first;
  coder_evt_t * p_coder_evt_tmp;

  if(p_coder_evt == NULL)//一个事件都还没有
  {
      pcoder_evt_first = pcoder_evt_free;//记录第一个事件
      pcoder_evt_free = pcoder_evt_free -> next;//把第一个空闲的evt结构体挪用
      pcoder_evt_first -> next = NULL;
      pcoder_evt_first -> coder_count = evt.coder_count;
      pcoder_evt_first -> evt_channel = evt.evt_channel;
      pcoder_evt_first -> evt_type = evt.evt_type;
  }
  else                   //至少存在一个事件
  {
      if(pcoder_evt_free == NULL)//达到事件上限，无法插入新事件
      {
	 //SEGGER_RTT_printf(0,"\r\n[coder_evt_insert]pcoder_evt_free==NULL\r\n");
	 return;
      }
      else
      {
	 p_coder_evt_tmp = pcoder_evt_free;
	 pcoder_evt_free = pcoder_evt_free -> next;//挪用第一个空闲的evt结构体
	 p_coder_evt_tmp -> coder_count = evt.coder_count;
	 p_coder_evt_tmp -> evt_channel = evt.evt_channel;
	 p_coder_evt_tmp -> evt_type    = evt.evt_type;
      }

      if(evt.coder_count<(p_coder_evt->coder_count))//新事件比原第一个事件早
      {
	  p_coder_evt_tmp -> next = p_coder_evt;
	  pcoder_evt_first = p_coder_evt_tmp;//提前第一个事件
      }
      else//新事件不比原第一个事件早
      {
	  while(p_coder_evt->next)//后面还有事件
	  {
	      if(evt.coder_count<(p_coder_evt->next->coder_count))
	      //新事件比下一事件早
	          break;
	      else
		  p_coder_evt = p_coder_evt -> next;
	  }
	  //p_coder_evt指向最后一个早于新事件的事件，后一事件或没有或晚于新事件

	  p_coder_evt_tmp -> next = p_coder_evt -> next;
	  p_coder_evt -> next = p_coder_evt_tmp;
      }//else(evt.coder_count>=(pcoder_evt_first->coder_count))
  }//else(pcoder_evt_first!=NULL)
  //SEGGER_RTT_printf(0,"\r\n[evt_insert]p_first=%x\r\n",pcoder_evt_first);
  //SEGGER_RTT_printf(0,"\r\n[evt_insert]p_coder=%x\r\n",p_coder_evt);
}

/**@breif 用于移除一个指定内容的coder event的函数
 */
void coder_evt_remove(coder_evt_t evt)
{
  coder_evt_t * p_coder_evt = pcoder_evt_first;
  coder_evt_t * p_coder_evt_tmp = NULL;

  if(p_coder_evt == NULL)//一个事件都还没有
  {
      //SEGGER_RTT_printf(0,"\r\n[coder_evt_remove]pcoder_evt_first==NULL\r\n");
      return;
  }
  //至少存在一个事件
  if((p_coder_evt->coder_count==evt.coder_count)&&
     (p_coder_evt->evt_channel==evt.evt_channel)&&
     (p_coder_evt->evt_type   ==evt.evt_type))//要移除第一个事件
  {
     p_coder_evt_tmp = p_coder_evt;
     pcoder_evt_first = pcoder_evt_first -> next;
  }
  else//不是移除第一个事件
  {
     while(p_coder_evt->next)//后面还有事件
     {
	 p_coder_evt_tmp = p_coder_evt->next;
         if((p_coder_evt_tmp->coder_count==evt.coder_count)&&
	    (p_coder_evt_tmp->evt_channel==evt.evt_channel)&&
	    (p_coder_evt_tmp->evt_type   ==evt.evt_type))//找到指定事件
		 break;
	 else
		 p_coder_evt = p_coder_evt_tmp;//移动到下一个事件
	 if((p_coder_evt->coder_count)>evt.coder_count)//之后事件一定都更迟
		 break;
     }

     if(p_coder_evt_tmp==NULL)//第一个事件不是，后面又没有第二个事件了
     {
	 //SEGGER_RTT_printf(0,"[coder_evt_remove]p_coder_evt_tmp==NULL\r\n");
	 return;
     }
     else if(p_coder_evt==p_coder_evt_tmp)//说明不是找到指定事件而break出来的
     {
	 //SEGGER_RTT_printf(0,"[coder_evt_remove]p_coder_evt==p_coder_evt_tmp\r\n");
	 return;
     }
     else
	 p_coder_evt->next = p_coder_evt_tmp->next;//bypass the event to remove
  }//else 不是移除第一个事件
  p_coder_evt_tmp -> next = pcoder_evt_free;
  pcoder_evt_free = p_coder_evt_tmp;//归还一个evt结构体到free链表
}

/**@brief 用于从valve_params生成coder_evts的函数
 */
void coder_evt_gather(void)
{
  extern valve_param_t valve_params[13];
  uint8_t channel,i;
  coder_evt_t evt;

  //organize a free coder events chain
  for(i=0;i<CODER_EVT_MAX;i++)
  {
      coder_evts[i].coder_count = 0;
      coder_evts[i].evt_channel = 0;
      coder_evts[i].evt_type    = channel_keep;
      coder_evts[i].next        = &coder_evts[i+1];
  }
  coder_evts[CODER_EVT_MAX-1].next = NULL;
  pcoder_evt_free = &coder_evts[0];
  pcoder_evt_first = NULL;

  for(channel=1;channel<=12;channel++)
  {
      evt.evt_channel = channel;
      for(i=0;i<ON_OFF_MAX;i++)
      {
	  if(valve_params[channel].on_offs_mask & (0x0001<<i))//该对开关参数有效
	  {
	      evt.evt_type = channel_on;
	      evt.coder_count = valve_params[channel].on_offs[i][0];
	      coder_evt_insert(evt);
	      evt.evt_type = channel_off_H;
	      evt.coder_count += valve_params[channel].high_duration;
	      coder_evt_insert(evt);
	      evt.evt_type = channel_off_L;
	      evt.coder_count = valve_params[channel].on_offs[i][1];
	      coder_evt_insert(evt);
	  }//if on_offs_mask & (0x0001<<i)
      }//for i=0:ON_OFF_MAX-1
  }//for channel=1:12
}
