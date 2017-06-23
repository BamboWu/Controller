#include "valve.h"

static uint32_t m_valve_state = 0;   //内部记录电磁阀通路输出状态的变量

valve_param_t valve_params[CHANNEL_NUM+1] = {0};//记录电磁阀通路通断参数的变量，第0元素弃用

uint8_t valve_params_flash = 0; //标记参数在flash中的状态的变量

/**@brief 用于设置电磁阀输出的函数
 *
 * @param[in] bank      指定根据374芯片划分的区间，0对应芯片U14，1为U58，2是U912
 * @param[in] mask      指定要掩去哪几位，置1的那几位会保持原状态
 * @param[in] state     指定输出状态，1为输出
 */
static void do_valve_set(uint8_t bank, uint8_t mask, uint8_t state)
{
  uint32_t mask32 = mask;
  uint32_t state32 = state;
  uint16_t toset16,toclr16;
  uint8_t  i;

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
    case BANK3: mask32 <<= 24;
		mask32 |= 0X00FFFFFF;  //BANK3对应才是真正最高8个通路
		state32 <<= 24;
		break;
    default   : mask32 = 0XFFFFFFFF;
  }
		state32 &= ~mask32;    //掩去所选BANK以外的新状态数据
  m_valve_state &= mask32;  //清零要修改的那几个通路的记录
  m_valve_state |= state32; //要修改的那几个通路记录新的状态
  
  //SEGGER_RTT_printf(0,"\r\n[do_valve_set]m_valve_state=0X%8x\r\n",m_valve_state);

  switch(bank)
  {
    case BANK0: toset16 = ~m_valve_state & 0X000000FF;    //BANK0对应最低8个通路
		toclr16 =  m_valve_state & 0X000000FF;
		break;
    case BANK1: toset16 = (~m_valve_state & 0X0000FF00)>>8;  //BANK1对应中间8个通路
		toclr16 = ( m_valve_state & 0X0000FF00)>>8;
		break;
    case BANK2: toset16 = (~m_valve_state & 0X00FF0000)>>16; //BANK2对应最高8个通路
		toclr16 = ( m_valve_state & 0X00FF0000)>>16;
		break;
    case BANK3: toset16 = (~m_valve_state & 0XFF000000)>>24; //BANK3对应才是真正最高8个通路
		toclr16 = ( m_valve_state & 0XFF000000)>>24;
		break;
    default   : toset16 = toclr16 = 0X0000;
  }

  //SEGGER_RTT_printf(0,"\r\n[do_valve_set]toset16=0X%4x,toclr16=0X%4x\r\n",toset16,toclr16);

  HAL_GPIO_WritePin(GPIOA,toclr16,GPIO_PIN_RESET); //将要变低电平的脚复位
  HAL_GPIO_WritePin(GPIOA,toset16,GPIO_PIN_SET);   //将要变高电平的脚置位

  switch(bank)
  {
    case BANK0: HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0,GPIO_PIN_SET);//上升沿锁数据到U14
		break;
    case BANK1: HAL_GPIO_WritePin(GPIOB,GPIO_PIN_1,GPIO_PIN_SET);//上升沿锁数据到U58
		break;
    case BANK2: HAL_GPIO_WritePin(GPIOB,GPIO_PIN_2,GPIO_PIN_SET);//上升沿锁数据到U912
		break;
    case BANK3: //因为当前硬件设计总共12通道，才用了3片374锁存，BANK3不存在的
    default   : HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2,GPIO_PIN_RESET);
  }
		//HAL_Delay(1);//延时1ms
  for(i=0;i<255;i++);//软件延时，这样中断中也调用到该函数不会死机
  HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2,GPIO_PIN_RESET); //确保CP0～CP2的信号平时保持低电平。
}

/**@brief 用于初始化电磁阀控制的函数
 */
void valve_init(void)
{
  GPIO_InitTypeDef  GPIO_InitStruct = {0}; //GPIO初始化用到的结构体

  __HAL_RCC_GPIOA_CLK_ENABLE();  //打开GPIOA口的时钟
  __HAL_RCC_GPIOB_CLK_ENABLE();  //打开GPIOB口的时钟
  __HAL_RCC_GPIOC_CLK_ENABLE();  //打开GPIOC口的时钟

  GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;  //推挽输出
  GPIO_InitStruct.Pull  = GPIO_PULLUP;          //上拉
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH; //高速IO 50MHz

  // Initialize CP0~CP2
  GPIO_InitStruct.Pin   = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2;//引脚号
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct); //初始化PB0,PB1,PB2引脚
  HAL_GPIO_WritePin(GPIOB,GPIO_InitStruct.Pin,GPIO_PIN_RESET);  //PB0,PB1,PB2输出低电平

  // Initialize D0~D7
  GPIO_InitStruct.Pin   |= GPIO_PIN_3|GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;//引脚号
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct); //初始化PA0~PA7引脚

  // Initialize 245 OE
  GPIO_InitStruct.Pin   = GPIO_PIN_4;//引脚号
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct); //初始化PC4引脚
  HAL_GPIO_WritePin(GPIOC,GPIO_InitStruct.Pin,GPIO_PIN_RESET);  //PC4输出低电平
  
  // 245 输出使能后，374 输出使能前， 清零
  do_valve_set(BANK0,0X00,0X00);
  do_valve_set(BANK1,0X00,0X00);
  do_valve_set(BANK2,0X00,0X00);
  m_valve_state = 0;

  // Initialize 374 OE
  GPIO_InitStruct.Pin   = GPIO_PIN_5;//引脚号
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct); //初始化PC5引脚
  HAL_GPIO_WritePin(GPIOC,GPIO_InitStruct.Pin,GPIO_PIN_RESET);  //PC5输出低电平

  // 从flash中读取电磁阀通断的参数
  valve_params_load();
}


/**@brief 用于打开单通道电磁阀输出的函数
 *
 * @param[in] channel    指定通道
 *
 * @note  无论所指定的通道原来状态是输出或不输出，该函数保证执行后，该通道的
 * 高低压两路一定都是输出。
 */
void valve_channel_on(uint8_t channel)
{
  uint8_t mask8 = 0X03;
  //uint8_t state8 = 0X03;//之后对state8的操作及使用都是与mask8一致的，故简并
  switch(channel)
  {
    case 12: mask8 <<= 2;
    case 11: mask8 <<= 2;
    case 10: mask8 <<= 2;
    case  9: do_valve_set(BANK2,~mask8,mask8);
	     break;
    case  8: mask8 <<= 2;
    case  7: mask8 <<= 2;
    case  6: mask8 <<= 2;
    case  5: do_valve_set(BANK1,~mask8,mask8);
	     break;
    case  4: mask8 <<= 2;
    case  3: mask8 <<= 2;
    case  2: mask8 <<= 2;
    case  1: do_valve_set(BANK0,~mask8,mask8);
	     break;
    default: break;
  }
}

/**@brief 用于关闭单通道单路电磁阀输出的函数
 *
 * @param[in] channel    指定通道
 * @param[in] Hi_Lo      指定高压通路还是低压通路，1为高压
 *
 * @note  无论所指定的通路原来状态是输出或不输出，该函数保证执行后，该通路的
 * 状态一定是不输出。
 */
void valve_channel_off(uint8_t channel, uint8_t Hi_Lo)
{
  uint8_t mask8 = 0X01;
  //uint8_t state8 = 0X01;//之后对state8的操作及使用都是与mask8一致的，故简并
  switch(channel)
  {
    case 12: mask8 <<= 2;
    case 11: mask8 <<= 2;
    case 10: mask8 <<= 2;
    case  9: mask8 <<= Hi_Lo; //高压通路总比同通道的低压通路高一位
	     do_valve_set(BANK2,~mask8,~mask8);
	     break;
    case  8: mask8 <<= 2;
    case  7: mask8 <<= 2;
    case  6: mask8 <<= 2;
    case  5: mask8 <<= Hi_Lo; //高压通路总比同通道的低压通路高一位
             do_valve_set(BANK1,~mask8,~mask8);
	     break;
    case  4: mask8 <<= 2;
    case  3: mask8 <<= 2;
    case  2: mask8 <<= 2;
    case  1: mask8 <<= Hi_Lo; //高压通路总比同通道的低压通路高一位
             do_valve_set(BANK0,~mask8,~mask8);
	     break;
    default: break;
  }
}

/**@brief 查询电磁阀通路的状态
 */
uint32_t valve_state_query(void)
{
    uint8_t  i;
    uint32_t state = 0;
    uint32_t tmp;
    uint32_t msk;

    //先记录高压通道在低16bit
    for(i=0,tmp=m_valve_state,msk=0x0001;i<CHANNEL_NUM;i++)
    {
	tmp >>= 1;//高压通道之间有第压间隔，所以靠拢一位
	state |= (tmp&msk);//记录一位状态
	msk <<= 1;//往state记录的位置每次左移一位
    }
    state <<= 16;//把高压通道的状态放到高16bit
    //记录低压通道在低16bit
    for(i=0,tmp=m_valve_state,msk=0x0001;i<CHANNEL_NUM;i++)
    {
	state |= (tmp&msk);//记录一位状态
	tmp >>= 1;//低压通道之间有高压间隔，所以靠拢一位
	msk <<= 1;//往state记录的位置每次左移一位
    }

    return (state << 1);//因为现在valve系列的代码中，把通道1做了通道0
}

/**@brief 用于从flash中读取电磁阀通路参数的函数
 */
void valve_params_load(void)
{
    uint8_t channel,on_off_id;
    uint32_t flash_addr;
    valve_param_t *p_valve_param;

    if(DATA_MARK != *(uint32_t*)FLASH_ADDR)//数据有损
    {
	valve_params_flash = 0;
	SEGGER_RTT_printf(0,"\r\n[valve_params_load]DATA MARK err\r\n");
	HAL_Delay(1);//不知道为什么，此处必须延时，否则编码器计数不正常
	return;//退出
    }
    //遍历填入12个通道参数
    for(channel=1,flash_addr=FLASH_ADDR+4;channel<=CHANNEL_NUM;channel++)
    {
	p_valve_param = &valve_params[channel];
	//为每个通道遍历填入(ON_OFF_MAX)对输出关闭设定
	for(on_off_id=0;on_off_id<ON_OFF_MAX;on_off_id++)
	{
	    p_valve_param->on_offs[on_off_id][0] = *(uint16_t*)flash_addr;
	    //给第channel通道的第on_off_id对设定的开启时间载入参数
	    flash_addr += 2;
	    p_valve_param->on_offs[on_off_id][1] = *(uint16_t*)flash_addr;
	    //给第channel通道的第on_off_id对设定的关闭时间载入参数
	    flash_addr += 2;
	}
	p_valve_param->on_offs_mask = *(uint16_t*)flash_addr;
	//给第channel通道的开关对数载入参数
	flash_addr += 2; //
	p_valve_param->high_duration = *(uint16_t*)flash_addr;
	//给第channel通道的高压输出持续时间载入参数
	flash_addr += 2; //
    }
    valve_params_flash = 1;
}

/**@brief 用于将电磁阀通路参数存入flash的函数
 */
void valve_params_store(void)
{
    uint8_t channel,on_off_id;
    uint32_t flash_addr,PAGEError;
    FLASH_EraseInitTypeDef EraseInitStruct;
    valve_param_t *p_valve_param;
    extern UART_HMI_t        UART_HMI;

    valve_params_flash = 0;//假定flash失败

#if defined(USE_UART3_485)
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);//PB12置高电平，使能写
    UART_HMI = UART_HMI;
#elif defined(USE_UART1_232)
    if(HAL_OK != HAL_UART_DeInit(&UART_HMI.Handle))
	SEGGER_RTT_printf(0,"\r\n[valve_params_store]UART DeInit err\r\n");
    else//停止串口中断成功
#endif
    {   
	if(HAL_OK != HAL_FLASH_Unlock())   //写flash之前，必须先解锁flash
	    SEGGER_RTT_printf(0,"[valve_params_store]flash unlock err\r\n");
	else//flash解锁成功
	{
	    EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	    EraseInitStruct.PageAddress = FLASH_ADDR;
	    EraseInitStruct.NbPages     = 1;
	    if(HAL_OK != HAL_FLASHEx_Erase(&EraseInitStruct,&PAGEError))
		SEGGER_RTT_printf(0,"\r\n[valve_params_store]Erase err:%x\r\n",PAGEError);
	    else//页擦除成功
	    {
		//遍历存入12个通道参数
		for(channel=1,flash_addr=FLASH_ADDR+4;channel<=CHANNEL_NUM;channel++)
	     	{
		    p_valve_param = &valve_params[channel];
	    	    //为每个通道遍历存入(ON_OFF_MAX)对 输出关闭时间
		    for(on_off_id=0;on_off_id<ON_OFF_MAX;on_off_id++)
	    	    {
			if(HAL_OK!=HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
			      flash_addr,
			      p_valve_param->on_offs[on_off_id][0]))
		    	//将第channel通道的第on_off_id对设定的开启时间参数存入
			{
	    		    SEGGER_RTT_printf(0,"\r\n[valve_params_store]valve_params[%x].on_offs[%x][0] err:%x",channel,on_off_id,HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,flash_addr,p_valve_param->on_offs[on_off_id][0]));
			 SEGGER_RTT_WaitKey();
			    channel = CHANNEL_NUM+1;
			    break;
			}
			flash_addr += 2;
    			if(HAL_OK!=HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
			      flash_addr,
			      p_valve_param->on_offs[on_off_id][1]))
		    	//将第channel通道的第on_off_id对设定的关闭时间参数存入
			{
	    		    SEGGER_RTT_printf(0,"\r\n[valve_params_store]valve_params[%x].on_offs[%x][1] err:%x",channel,on_off_id,HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,flash_addr,p_valve_param->on_offs[on_off_id][1]));
			 SEGGER_RTT_WaitKey();
			    channel = CHANNEL_NUM+1;
		    	    break;
			}
			flash_addr += 2;
	    	    }//for(on_off_id=0;on_off_id<ON_OFF_MAX;on_off_id++)
	    	    if(HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
			  flash_addr,
			  p_valve_param->on_offs_mask))
	    	    //将第channel通道的开关对数参数存入
	    	    {
			  SEGGER_RTT_printf(0,"\r\n[valve_params_store]valve_params[%x].on_offs_mask err:%x",channel,HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,flash_addr,p_valve_param->on_offs_mask));
			 SEGGER_RTT_WaitKey();
			  break;
		    }
	    	    flash_addr += 2; 
	    	    if(HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
			  flash_addr,
			  p_valve_param->high_duration))
		    //将第channel通道的高压输出持续时间参数存入
		    {
			  SEGGER_RTT_printf(0,"\r\n[valve_params_store]valve_params[%x].high_dration err:%x",channel,HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,flash_addr,p_valve_param->high_duration));
			 SEGGER_RTT_WaitKey();
			  break;
		    }
	    	    flash_addr += 2;
	    	}//for(channel=1,flash_addr=FLASH_ADDR+4;channel<=12;channel++)

		if(FLASH_ADDR+12*(4*ON_OFF_MAX+4)+4 <= flash_addr)//写完整了参数
		{
		    if(HAL_OK != HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
			           FLASH_ADDR,
				   DATA_MARK))//写标识，记录flash中数据确实存在
		    {
			 SEGGER_RTT_printf(0,"\r\n[valve_params_store]DATA MARK err:%x\r\n",HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,FLASH_ADDR,DATA_MARK));
			 SEGGER_RTT_WaitKey();
		    }
		    else
		    {
			 //SEGGER_RTT_printf(0,"\r\n[valve_params_store]DATA MARK OK\r\n");
			 valve_params_flash = 1;//参数完整，标记也在
		    }
		}//else 参数未写完整,之前从双重for中跳出时必有输出
	    }
	    HAL_FLASH_Lock();                //写flash之后，必须再锁上flash
	    //valve_params_flash = 1;          //flash中的参数完整
	}//else if(HAL_OK == HAL_FLASH_Unlock())
#if defined(USE_UART1_232)
	if(HAL_UART_Init(&UART_HMI.Handle) != HAL_OK)
	    SEGGER_RTT_printf(0,"\r\n[valve_params_store]UART ReInit err\r\n");
	HAL_UART_Receive_IT(&UART_HMI.Handle,UART_HMI.pRxBuffer_in,1);
#endif
    }//else if(HAL_OK == HAL_UART_DeInit(&UART_HMI.Handle))
#if defined(USE_UART3_485)
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);//PB12置低电平，使能读
#endif
}

/**@brief 修改开关参数的函数
 *
 * @param[in] channel 指定通道
 * @param[in] new_param 参数配置
 *
 * @detail new_param中打开、关闭、高压持续时间均是角度值
 * 分整数和小数两部分表示，范围为0度～359.999度，最小精度为0.001度
 *
 * @note channel的取值范围为1～12
 * @note new_param 中的on_degree、off_degree和high_degree取值不应超过359
 * on_fraction、off_fraction和high_fraction取值不应超过999
 * @note on_off_valid 只应取0和1两种值，前者表示无效，后者表示有效
 * @note high_valid 只有在取非0值时，high_duration的值才会被修改
 * 而不论on_off_valid 取什么值，on_offs[on_off_index]的值总会被修改
 */
void valve_params_modify(uint8_t channel, valve_modify_t new_param)
{
    extern uint16_t coder_division;
    valve_param_t * p_valve_param = &valve_params[channel];
    uint8_t index = new_param.on_off_index;
    uint16_t count;
    
    count = (new_param.on_degree*1000 + new_param.on_fraction)//整数小数拼合
	    * coder_division                                  //先乘编码器分度
	    / 360000;                                         //再除最大量程
    p_valve_param->on_offs[index][0] = count;
    count = (new_param.off_degree*1000 + new_param.off_fraction)//整数小数拼合
	    * coder_division                                  //先乘编码器分度
	    / 360000;                                         //再除最大量程
    p_valve_param->on_offs[index][1] = count;
    p_valve_param->on_offs_mask &= ~(0x0001<<index);          //修改相应位的掩码
    p_valve_param->on_offs_mask |= new_param.on_off_valid << index;

    if(new_param.high_valid)
    {
        //count = (new_param.high_degree*1000 + new_param.high_fraction)//整数小数拼合
        //        * coder_division                              //先乘编码器分度
        //        / 360000;                                     //再除最大量程
	count = new_param.high_duration;
        p_valve_param->high_duration = count;
    }
}

/**@brief 将指定通道的参数转换成用于显示的数据的函数
 *
 * @param[in] channel 指定通道
 * @param[in] p_param 指向存放显示格式参数数据的指针
 *
 * @detail p_param中打开、关闭、高压持续时间均是角度值
 * 分整数和小数两部分表示，范围为0度～359.999度，最小精度为0.001度
 *
 * @note channel的取值范围为1～12
 * @note p_param 中的on_degree、off_degree和high_degree取值不应超过359
 * on_fraction、off_fraction和high_fraction取值不应超过999
 */
void valve_params_display(uint8_t channel, valve_display_t * p_param)
{
    extern uint16_t coder_division;
    valve_param_t * p_valve_param = &valve_params[channel];
    uint8_t index;
    uint32_t degree;
    
    for(index=0;index<ON_OFF_MAX;index++)
    {
	    degree = p_valve_param->on_offs[index][0] * 360000 //先乘最大量程
		     / coder_division;                         //再除编码器分度
	    p_param->on_degree[index] = degree / 1000;         //取整数部分
	    p_param->on_fraction[index] = degree % 1000;       //取小数部分

	    degree = p_valve_param->on_offs[index][1] * 360000 //先乘最大量程
		     / coder_division;                         //再除编码器分度
	    p_param->off_degree[index] = degree / 1000;        //取整数部分
	    p_param->off_fraction[index] = degree % 1000;      //取小数部分
    }
    p_param->on_offs_mask = p_valve_param->on_offs_mask;       //开关参数有效位

    //degree = p_valve_param->high_duration * 360000             //先乘最大量程
    //         / coder_division;                                 //再除编码器分度
    //p_param->high_degree = degree / 1000;                      //取整数部分
    //p_param->high_fraction = degree % 1000;                    //取小数部分
    p_param->high_duration = p_valve_param->high_duration;
}

/**@brief 将flash中电磁阀通路参数擦除的函数
 */
void valve_params_erase(void)
{
    uint32_t PAGEError;
    FLASH_EraseInitTypeDef EraseInitStruct;

    if(HAL_OK != HAL_FLASH_Unlock())   //写flash之前，必须先解锁flash
	SEGGER_RTT_printf(0,"[valve_params_erase]flash unlock err\r\n");
    else//flash解锁成功
    {
	EraseInitStruct.TypeErase   = FLASH_TYPEERASE_PAGES;
	EraseInitStruct.PageAddress = FLASH_ADDR;
	EraseInitStruct.NbPages     = 1;
	if(HAL_OK != HAL_FLASHEx_Erase(&EraseInitStruct,&PAGEError))
	    SEGGER_RTT_printf(0,"\r\n[valve_params_erase]Erase err:%x\r\n",PAGEError);
	else//页擦除成功
	{
	    valve_params_flash = 0;
	}
	HAL_FLASH_Lock();                //写flash之后，必须再锁上flash
    }//else if(HAL_OK == HAL_FLASH_Unlock())
}
