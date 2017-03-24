#include "valve.h"

static uint32_t m_valve_state = 0;  //内部记录电磁阀通路输出状态的变量

valve_param_t valve_params[12];     //记录电磁阀通路通断参数的变量
valve_param_t valve_param;     //记录电磁阀通路通断参数的变量
valve_t valve_test[12];     //记录电磁阀通路通断参数的变量

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
  m_valve_state &= mask32;  //清零要修改的那几个通路的记录
  m_valve_state |= state32; //要修改的那几个通路记录新的状态
  
  SEGGER_RTT_printf(0,"\r\n[do_valve_set]m_valve_state=0X%8x\r\n",m_valve_state);

  switch(bank)
  {
    case BANK0: toset16 = ~m_valve_state & 0X000000FF;    //BANK0对应最低8个通路
		toclr16 =  m_valve_state & 0X000000FF;
		break;
    case BANK1: toset16 = (~m_valve_state & 0X0000FF00)>>8;  //BANK1对应中间8个通路
		toclr16 =  (m_valve_state & 0X0000FF00)>>8;
		break;
    case BANK2: toset16 = (~m_valve_state & 0X00FF0000)>>16; //BANK2对应最高8个通路
		toclr16 =  (m_valve_state & 0X00FF0000)>>16;
		break;
    default   : toset16 = toclr16 = 0X0000;
  }

  SEGGER_RTT_printf(0,"\r\n[do_valve_set]toset16=0X%4x,toclr16=0X%4x\r\n",toset16,toclr16);

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
    default   : HAL_GPIO_WritePin(GPIOB,GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2,GPIO_PIN_RESET);
  }
		HAL_Delay(1);//延时1ms
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
  }
}

/**@brief 用于从flash中读取电磁阀通路参数的函数
 */
void valve_params_load(void)
{
    uint8_t channel,on_off_id;
    uint32_t flash_addr;
    valve_param_t *p_valve_param;

    //遍历填入12个通道参数
    for(channel=1,flash_addr=FLASH_ADDR;channel<=12;channel++)
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
	p_valve_param->high_duration = *(uint16_t*)flash_addr;
	//给第channel通道的高压输出持续时间载入参数
	flash_addr += 2; //多留半字(16bit/2byte)作为每个通道的分隔
	flash_addr += 2; //多留半字(16bit/2byte)作为每个通道的分隔
    }
}

/**@brief 用于将电磁阀通路参数存入flash的函数
 */
void valve_params_store(void)
{
    uint8_t channel,on_off_id;
    uint32_t flash_addr;
    valve_param_t *p_valve_param;

    HAL_FLASH_Unlock();              //写flash之前，必须先解锁flash
    FLASH_PageErase(FLASH_ADDR);     //存入新数据前，必先擦除原有数据所在的页
    //遍历存入12个通道参数
    for(channel=1,flash_addr=FLASH_ADDR;channel<=12;channel++)
    {
	p_valve_param = &valve_params[channel];
	//为每个通道遍历存入(ON_OFF_MAX)对 输出关闭时间
	for(on_off_id=0;on_off_id<ON_OFF_MAX;on_off_id++)
	{
	    //HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
	    //    	      flash_addr,
	    //    	      (uint64_t)(*(uint32_t*)(&valve_params[channel].on_offs[on_off_id][0])));
	    ////将第channel通道的第on_off_id对设定的开启关闭时间参数存入
	    //flash_addr += 4;
	    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
			      flash_addr,
			      p_valve_param->on_offs[on_off_id][0]);
	    //将第channel通道的第on_off_id对设定的开启时间参数存入
	    flash_addr += 2;
	    HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD,
			      flash_addr,
			      p_valve_param->on_offs[on_off_id][1]);
	    //将第channel通道的第on_off_id对设定的关闭时间参数存入
	    flash_addr += 2;
	}
	HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD,
			  flash_addr,
			  p_valve_param->high_duration);
	//将第channel通道的高压输出持续时间参数存入
	flash_addr += 4; 
    }
    HAL_FLASH_Lock();                //写flash之后，必须再锁上flash
}
