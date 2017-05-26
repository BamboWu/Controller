#include "coder.h"

TIM_HandleTypeDef Tim8Handle;             //TIM8的结构体
TIM_HandleTypeDef Tim5Handle;             //TIM5的结构体

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
  HAL_TIM_OC_ConfigChannel(&Tim8Handle,&OCConfig,TIM_CHANNEL_4);

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

  Tim5Handle.Instance = TIM5;
  Tim5Handle.Init.Period            = 2 - 1;//每1ms计时溢出一次
  Tim5Handle.Init.Prescaler         = 36000 - 1;//72MHz系统时钟分频
  Tim5Handle.Init.ClockDivision     = 0;
  Tim5Handle.Init.CounterMode       = TIM_COUNTERMODE_UP;

  if (HAL_TIM_Base_Init(&Tim5Handle) != HAL_OK)
	  SEGGER_RTT_printf(0,"\r\n[coder_init]TIM5 Init fail\r\n");
  if (HAL_TIM_Base_Start_IT(&Tim5Handle) != HAL_OK)
	  SEGGER_RTT_printf(0,"\r\n[coder_init]TIM5 IT Init fail\r\n");

  coder_evt_gather();
}

/**@brief 用于启用编码器接口的函数
 */
void coder_Z(void)
{
  HAL_TIM_Encoder_Stop(&Tim8Handle,TIM_CHANNEL_12);
  Tim8Handle.Instance->CNT = 0;//清零重启
  if(pcoder_evt_first)//设置第一个事件点
  {
	Tim8Handle.Instance->CCR3 = pcoder_evt_first->coder_count;
	SEGGER_RTT_printf(0,"\r\n[coder_Z]first count:%4d\r\n",pcoder_evt_first->coder_count);
      	if(pcoder_evt_first->next)//第一个事件点不是最后一个事件点
	    Tim8Handle.Instance->CCR4 = pcoder_evt_first->prev->coder_count - EVT_RETURN_DIFF;//为反转设置最后一个事件点
	SEGGER_RTT_printf(0,"\r\n[coder_Z]last count:%4d\r\n",pcoder_evt_first->prev->coder_count);
  }
  pcoder_evt_now = pcoder_evt_first;
  SEGGER_RTT_printf(0,"\r\n[coder_Z]pcoder_evt_now:%x\r\n",pcoder_evt_now);
  HAL_TIM_OC_Start_IT(&Tim8Handle,TIM_CHANNEL_3);
  HAL_TIM_OC_Start_IT(&Tim8Handle,TIM_CHANNEL_4);
  HAL_TIM_Encoder_Start(&Tim8Handle,TIM_CHANNEL_12);
}

/**@breif 用于插入一个coder event到合适位置的函数
 */
void coder_evt_insert(coder_evt_t evt)
{
  coder_evt_t * p_coder_evt = pcoder_evt_first;

  if(NULL == pcoder_evt_free)//达到事件上限，无法插入新事件
	  return;
  else
  {
      pcoder_evt_free -> coder_count = evt.coder_count;
      pcoder_evt_free -> evt_channel = evt.evt_channel;
      pcoder_evt_free -> evt_type    = evt.evt_type;

      if(NULL == pcoder_evt_first)//一个事件都还没有
      {
	  pcoder_evt_first = pcoder_evt_free;//记录及一个事件
	  pcoder_evt_free  = pcoder_evt_free -> next;//挪用第一个空闲的evt结构体
	  pcoder_evt_first -> prev = pcoder_evt_first;//自构成循环
	  pcoder_evt_first -> next = NULL;
      }
      else
      {
	  while(evt.coder_count>=(p_coder_evt->coder_count))
	  //新事件不早于当前比较事件
	  {
		p_coder_evt = p_coder_evt -> next;
		if(NULL == p_coder_evt)//已过了最后一个事件
			break;
	  }
	  if(p_coder_evt)//有找到晚于新事件的事件
	  {
		pcoder_evt_free -> prev = p_coder_evt -> prev;
		if(pcoder_evt_first == p_coder_evt)//插在了第一个事件前
		    pcoder_evt_first = pcoder_evt_free;//前挪一个事件
		else
		    pcoder_evt_free -> prev -> next = pcoder_evt_free;
		p_coder_evt -> prev = pcoder_evt_free;
		pcoder_evt_free = pcoder_evt_free -> next;//挪用first free evt
		p_coder_evt -> prev -> next = p_coder_evt;
	  }
	  else//添加到事件链末尾
	  {
		pcoder_evt_free -> prev = pcoder_evt_first -> prev;
		pcoder_evt_free -> prev -> next = pcoder_evt_free;
		pcoder_evt_first -> prev = pcoder_evt_free;
		pcoder_evt_free = pcoder_evt_free -> next;//挪用first free evt
		pcoder_evt_first -> prev -> next = NULL;
	  }
      }
  }
  //SEGGER_RTT_printf(0,"\r\n[evt_insert]p_first=%x\r\n",pcoder_evt_first);
  //SEGGER_RTT_printf(0,"\r\n[evt_insert]p_coder=%x\r\n",p_coder_evt);
}

/**@breif 用于移除一个指定内容的coder event的函数
 */
void coder_evt_remove(coder_evt_t evt)
{
  coder_evt_t * p_coder_evt = pcoder_evt_first;

  if(p_coder_evt == NULL)//一个事件都还没有
  {
      //SEGGER_RTT_printf(0,"\r\n[coder_evt_remove]pcoder_evt_first==NULL\r\n");
      return;
  }
  else//至少存在一个事件
  {
      while(evt.coder_count > (p_coder_evt->coder_count))
      {
	   p_coder_evt = p_coder_evt ->next;
	   if(NULL == p_coder_evt)//移到最后了也没找到不早于移除事件的
		   return;
      }
      while(evt.coder_count == (p_coder_evt->coder_count))
      {
	   if(evt.evt_channel != (p_coder_evt->evt_channel))//通道不符
	   {
		   p_coder_evt = p_coder_evt -> next;
		   if(NULL == p_coder_evt)//移到了最后也没有找到移除事件
			   return;
	   }
	   else if(evt.evt_type != (p_coder_evt->evt_type))//事件类型不符
	   {
		   p_coder_evt = p_coder_evt -> next;
		   if(NULL == p_coder_evt)//移到了最后也没有找到移除事件
			   return;
	   }
	   else//当前p_coder_evt指向事件为要删除的
	   {
		//SEGGER_RTT_printf(0,"\r\n[evt_remove]p_coder=%x\r\n",p_coder_evt);
		if(pcoder_evt_first == p_coder_evt)//要移除第一个
		{
		        pcoder_evt_first = p_coder_evt -> next;//事件头后移
		        if(p_coder_evt -> next)//不是要移除最后一个
			    p_coder_evt -> next -> prev = p_coder_evt -> prev;
		        p_coder_evt -> next = pcoder_evt_free;
		        pcoder_evt_free = p_coder_evt;//归还了一个evt结构体
		}
		else//要移除的不是第一个
		{
		        if(NULL == p_coder_evt -> next)//要移除最后一个
		     	    pcoder_evt_first -> prev = p_coder_evt -> prev;
		        else//不是要移除最后一个
		     	    p_coder_evt -> next -> prev = p_coder_evt -> prev;
		        p_coder_evt -> prev -> next = p_coder_evt -> next;
		        p_coder_evt -> next = pcoder_evt_free;
		        pcoder_evt_free = p_coder_evt;//归还了一个evt结构体
		}
		//SEGGER_RTT_printf(0,"\r\n[evt_remove]p_first=%x\r\n",pcoder_evt_first);
		break;
	   }
      }
  }
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
      coder_evts[i].prev        = &coder_evts[i-1];
      coder_evts[i].next        = &coder_evts[i+1];
  }
  coder_evts[0].prev = NULL;
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
	      //evt.evt_type = channel_off_H;//高压关闭不依赖于编码器
	      //evt.coder_count += valve_params[channel].high_duration;
	      //coder_evt_insert(evt);//改用定时器来关闭
	      evt.evt_type = channel_off_L;
	      evt.coder_count = valve_params[channel].on_offs[i][1];
	      coder_evt_insert(evt);
	  }//if on_offs_mask & (0x0001<<i)
      }//for i=0:ON_OFF_MAX-1
  }//for channel=1:12
}
