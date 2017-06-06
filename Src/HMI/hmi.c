#include "hmi.h"

UART_HMI_t      UART_HMI = {0};

TIM_HandleTypeDef  Tim6Handle;    //用于对modbus单次传输计timeout
//TIM_OnePulse_InitTypeDef OPConfig;//用于配置单次计时的结构体

/**@brief  从接收Buffer中读取n字节的函数
 *
 * @param  pRxBuf_out Buffer开始读的位置
 * @param  n          读取字节数，取1，2，4
 * @param  pval       存放数据的变量地址
 *
 * @notice 请确保pRxBuf_out>=UART_HMI.RxBuffer
 */
static void hmi_rxbuf_out(uint8_t * pRxBuf_out, uint8_t n, void * pval)
{
    uint8_t * pend = pRxBuf_out - n + 1;
    if(UART_HMI.RxBuffer <= pend)//读完数据指针不会溢出
    {
	    switch(n)
	    {
		    case 1 : *(uint8_t*)pval  = pend[0];break;
		    case 2 : *(uint16_t*)pval = *(uint16_t*)pend;break;
		    case 4 : *(uint32_t*)pval = *(uint32_t*)pend;break;
		    default: break;
	    }
    }
    else//if(UART_HMI.RxBuffer > pend)读完数据指针要溢出
    {
	    switch(n)
	    {
		    case 2 : *(uint16_t*)pval 
			     = (UART_HMI.RxBuffer[0] << 8) 
			     | UART_HMI.RxBuffer[RXBUFFER_SIZE-1];
			     break;
		    case 4 : for(*(uint32_t*)pval=0;n;n--)
			     {
				     *(uint32_t*)pval = *(uint32_t*)pval << 8 | *pRxBuf_out;
				     pRxBuf_out--;
				     if(UART_HMI.RxBuffer > pRxBuf_out)//超范围
					     pRxBuf_out = UART_HMI.RxBuffer + RXBUFFER_SIZE - 1;
			     }
			     break;
		    default: break;
	    }
    }//if(UART_HMI.RxBuffer<=pend)
}

/**@brief 用于初始化与触摸屏通信的函数
 *
 * @detail 
 */
void hmi_init(void)
{

//    TIM_OnePulse_InitTypeDef OPConfig;
    /* Configure TIM  ------------------------------------------------------*/

    /* -1- Set TIM6 instance */
    Tim6Handle.Instance = TIM6;

    /* -2- Initialize TIM6 peripheral as follows: //modbus要求1.5char以上
         + Period = 4 - 1                         //的间隔认为一次传输结束
         + Prescaler = 50000 - 1                  //9600baud时，320Hz-3char
         + ClockDivision = 0                      //480Hz-2char 72MHz时钟
         + Counter direction = Up                 //故需200,000分频
    */
    Tim6Handle.Init.Period            = 4 - 1;
    Tim6Handle.Init.Prescaler         = 50000 - 1;
    Tim6Handle.Init.ClockDivision     = 0;
    Tim6Handle.Init.CounterMode       = TIM_COUNTERMODE_UP;
    Tim6Handle.Init.RepetitionCounter = 0;

//    OPConfig.OCMode = TIM_OCMODE_TIMING;           //不做实际的输出

    if (HAL_TIM_OnePulse_Init(&Tim6Handle,TIM_OPMODE_SINGLE) != HAL_OK)
            SEGGER_RTT_printf(0,"\r\n[hmi_init]TIM Init fail\r\n");

    UART_HMI.Handle.Instance        = USART_HMI;

    UART_HMI.Handle.Init.BaudRate   = 9600;
    UART_HMI.Handle.Init.WordLength = UART_WORDLENGTH_8B;
    UART_HMI.Handle.Init.Parity     = UART_PARITY_NONE;
    UART_HMI.Handle.Init.StopBits   = UART_STOPBITS_1;
    UART_HMI.Handle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UART_HMI.Handle.Init.Mode       = UART_MODE_TX_RX;

    if(HAL_UART_DeInit(&UART_HMI.Handle) != HAL_OK)
	SEGGER_RTT_printf(0,"\r\n[HMI_Init]DeInit err\r\n");
    if(HAL_UART_Init(&UART_HMI.Handle) != HAL_OK)
	SEGGER_RTT_printf(0,"\r\n[HMI_Init]Init err\r\n");

    UART_HMI.Status = 0;
    UART_HMI.pRxBuffer_in  = UART_HMI.RxBuffer + RXBUFFER_SIZE - 1;
    UART_HMI.pRxBuffer_out = UART_HMI.RxBuffer + RXBUFFER_SIZE - 1;
    HAL_UART_Receive_IT(&UART_HMI.Handle, UART_HMI.pRxBuffer_in, 1);
}

/**@brief  图形交互界面通信数据处理后台程序
 *
 * @detail 通过UASRT1与触摸屏交换信息，根据触摸屏的指令配置参数、控制运转
 *
 */
void hmi_main(void)
{
    uint8_t cnt;
    uint8_t * pcrc;
    uint16_t crc_received;
    uint16_t addr,num;

    if(UART_HMI.Status & RX_CPLT)
    {
	cnt = UART_HMI.Status &= 0X7F;
	UART_HMI.Status = 0;
	if(2 > cnt)//接收不完整
	{
	   UART_HMI_PRXBUF_(cnt);//指针前移cnt字节
	   SEGGER_RTT_printf(0,"\r\n[hmi_main]cnt=%x\r\n",cnt);
	   return;
	}

	pcrc = UART_HMI.pRxBuffer_out - cnt + 2;
	//SEGGER_RTT_printf(0,"\r\n[hmi_main]pcrc=%x\r\n",pcrc);
	if(UART_HMI.RxBuffer > pcrc)//全在溢出后
	    hmi_rxbuf_out(pcrc+RXBUFFER_SIZE,2,&crc_received);
	else
	    hmi_rxbuf_out(pcrc,2,&crc_received);

	if(crc_received != crc_calculate(UART_HMI.pRxBuffer_out,cnt-2))//校验错
	{
	    UART_HMI_PRXBUF_(cnt);//指针前移cnt字节
	    SEGGER_RTT_printf(0,"\r\n[hmi_main]crc=%4x\r\n",crc_received);
	    return;
	}

	UART_HMI.TxBuffer[0] = UART_HMI.pRxBuffer_out[0];//复制slave地址
	UART_HMI_PRXBUF_(1);//slave地址自动满足，不做判断
	cnt--;

	UART_HMI.TxBuffer[1] = UART_HMI.pRxBuffer_out[0];//复制操作码
	UART_HMI_PRXBUF_(1);
	cnt--;
	switch(UART_HMI.TxBuffer[1])//查看Function Code
	{
            case MODBUS_RDREGS: hmi_rxbuf_out(UART_HMI.pRxBuffer_out,2,&addr);
				UART_HMI_PRXBUF_(2);
				hmi_rxbuf_out(UART_HMI.pRxBuffer_out,2,&num);
				UART_HMI_PRXBUF_(2);
				cnt -= 4;
				hmi_r(addr,num);
				UART_HMI_PRXBUF_(cnt);
     				break;
            case MODBUS_WRREG : hmi_rxbuf_out(UART_HMI.pRxBuffer_out,2,&addr);
				UART_HMI_PRXBUF_(2);
				hmi_w(addr,1);
				UART_HMI_PRXBUF_(cnt);
     				break;
            case MODBUS_WRREGS: hmi_rxbuf_out(UART_HMI.pRxBuffer_out,2,&addr);
				UART_HMI_PRXBUF_(2);
				hmi_rxbuf_out(UART_HMI.pRxBuffer_out,2,&num);
				UART_HMI_PRXBUF_(3);//Omit Byte Count
				cnt -= 5;
				hmi_w(addr,num);
				UART_HMI_PRXBUF_(cnt);
     				break;
	    default: 	SEGGER_RTT_printf(0,"\r\n[hmi_main]:");
			while(cnt--)//还有没打印完的数据
			{
		    	   SEGGER_RTT_printf(0,"%3x",*UART_HMI.pRxBuffer_out);
			   UART_HMI_PRXBUF_(1);
			}
			SEGGER_RTT_printf(0,"\r\n");
	}
    }
}

/**@brief  处理modbus读操作的函数
 *
 * @param  addr   起始地址
 * @param  num    寄存器数
 */
void hmi_r(uint16_t addr, uint16_t num)
{
    uint8_t index = 3;
    uint8_t i = 0;
    uint16_t crc;
    HAL_StatusTypeDef status;
    extern  valve_param_t valve_params[];

    SEGGER_RTT_printf(0,"\r\n[hmi_r]addr=%4x,num=%4x\r\n",addr,num);
    UART_HMI.TxBuffer[2] = num << 1;//字节数是寄存器数的两倍

    while(num--)
    {
	UART_HMI.TxBuffer[index++] = (valve_params[1].high_duration&0xFF00)>>8;
	UART_HMI.TxBuffer[index++] = (valve_params[1].high_duration&0x00FF);
    }

    crc = crc_calculate(UART_HMI.TxBuffer,index);
    SEGGER_RTT_printf(0,"\r\n[hmi_r]");
    while(i != index)
    {
	SEGGER_RTT_printf(0,"%3x",UART_HMI.TxBuffer[i++]);
    }
    SEGGER_RTT_printf(0,"%5x\r\n",crc);

    UART_HMI.TxBuffer[index++] = (crc & 0xFF00) >> 8;
    UART_HMI.TxBuffer[index++] = (crc & 0x00FF);

#ifdef  USE_UART3_485
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);//PB12置高电平，使能写
#endif
    status = HAL_UART_Transmit(&UART_HMI.Handle,UART_HMI.TxBuffer,index,5000);
    SEGGER_RTT_printf(0,"\r\n[hmi_r]status:%x\r\n",status);
#ifdef  USE_UART3_485
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);//PB12置低电平，使能读
#endif

}

/**@brief  处理modbus写操作的函数
 *
 * @param  addr   起始地址
 * @param  num    寄存器数
 */
void hmi_w(uint16_t addr, uint16_t num)
{
    uint8_t index = 3;
    uint16_t crc;
    HAL_StatusTypeDef status;
    extern  valve_param_t valve_params[];

    SEGGER_RTT_printf(0,"\r\n[hmi_w]addr=%4x,num=%4x\r\n",addr,num);

    if(MODBUS_WRREGS == UART_HMI.TxBuffer[1])//
    {
    }
    else
    {
    }

    crc = crc_calculate(UART_HMI.TxBuffer,index);

    UART_HMI.TxBuffer[index++] = (crc & 0xFF00) >> 8;
    UART_HMI.TxBuffer[index++] = (crc & 0x00FF);

#ifdef  USE_UART3_485
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);//PB12置高电平，使能写
#endif
    status = HAL_UART_Transmit(&UART_HMI.Handle,UART_HMI.TxBuffer,index,5000);
    SEGGER_RTT_printf(0,"\r\n[hmi_r]status:%x\r\n",status);
#ifdef  USE_UART3_485
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);//PB12置低电平，使能读
#endif

}
/**@brief  modbus响应调试程序
 *
 */
void hmi_test_resp(void)
{
    uint16_t crc;
    extern  valve_param_t valve_params[];

    UART_HMI.TxBuffer[0] = 0x01;
    UART_HMI.TxBuffer[1] = 0x03;
    UART_HMI.TxBuffer[2] = 0x02;
    UART_HMI.TxBuffer[3] = (valve_params[1].high_duration&0xFF00)>>8;
    UART_HMI.TxBuffer[4] = (valve_params[1].high_duration&0x00FF);
    crc = crc_calculate(UART_HMI.TxBuffer,5);
    UART_HMI.TxBuffer[5] = (crc & 0xFF00) >> 8;
    UART_HMI.TxBuffer[6] = (crc & 0X00FF);

    if(UART_HMI.Status & RX_CPLT)
    {
	UART_HMI.Status = 0;
#ifdef  USE_UART3_485
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);  //PB12输出高电平，使能写
#endif
    SEGGER_RTT_printf(0,"\r\n[hmi_test_resp]%x\r\n",
		  HAL_UART_Transmit(&UART_HMI.Handle,UART_HMI.TxBuffer,7,5000));
#ifdef  USE_UART3_485
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);//PB12输出低电平，使能读
#endif
    }
}

#ifdef  USE_UART3_485
/**@brief  485调试程序
 *
 */
void hmi_test_485(void)
{
    unsigned char char_num=0;

    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);  //PB12输出高电平，使能写
    UART_HMI.TxBuffer[0] = ':';
    HAL_UART_Transmit_IT(&UART_HMI.Handle, UART_HMI.TxBuffer, 1);
    while(1)
    {
		if(9 <= char_num)
			char_num = 0;
		else
			char_num++;
		UART_HMI.TxBuffer[0] = 'H';
		UART_HMI.TxBuffer[1] = 'e';
		UART_HMI.TxBuffer[2] = 'l';
		UART_HMI.TxBuffer[3] = 'l';
		UART_HMI.TxBuffer[4] = 'o';
		UART_HMI.TxBuffer[5] = '0'+char_num;
		UART_HMI.TxBuffer[6] = '\r';
		UART_HMI.TxBuffer[7] = '\n';
		HAL_UART_Transmit_IT(&UART_HMI.Handle, UART_HMI.TxBuffer, 8);
		SEGGER_RTT_printf(0,"\r\n[hmi_test]wait Tx cplt\r\n");
		HAL_Delay(3000);
    }
}
#endif
