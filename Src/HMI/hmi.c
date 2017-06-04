#include "hmi.h"

UART_HMI_t      UART_HMI = {0};

TIM_HandleTypeDef  Tim6Handle;    //用于对modbus单次传输计timeout
//TIM_OnePulse_InitTypeDef OPConfig;//用于配置单次计时的结构体

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
//    if (HAL_TIM_OnePulse_ConfigChannel(&Tim6Handle,&OPConfig,TIM_CHANNEL_1,TIM_CHANNEL_2) != HAL_OK)
//	    SEGGER_RTT_printf(0,"\r\n[hmi_init]OnePulse Config fail\r\n");
//    if (HAL_TIM_OnePulse_ConfigClockSource(&Tim6Handle,T) != HAL_OK)
//	    SEGGER_RTT_printf(0,"\r\n[hmi_init]ClkSrc Config fail\r\n");

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
    UART_HMI.pRxBuffer_in  = UART_HMI.RxBuffer;
    UART_HMI.pRxBuffer_out = UART_HMI.RxBuffer;
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
    uint16_t crc_received;
    uint16_t addr,num;

    if(UART_HMI.Status & RX_CPLT)
    {
	cnt = UART_HMI.Status &= 0X7F;
	UART_HMI.Status = 0;
	if(2 > cnt)//接收不完整
	{
	    UART_HMI.pRxBuffer_out += cnt;
	    if(UART_HMI.RxBuffer <= UART_HMI.pRxBuffer_out-RXBUFFER_SIZE)
		    UART_HMI.pRxBuffer_out -= RXBUFFER_SIZE;
	    return;
	}

	crc_received = UART_HMI.pRxBuffer_out[cnt-2] << 8 | UART_HMI.pRxBuffer_out[cnt-1];//由于STM32为小端而modbus是大端，所以要自己拼合

	if(crc_received != crc_calculate(UART_HMI.pRxBuffer_out,cnt-2))//校验错
	{
	    UART_HMI.pRxBuffer_out += cnt;
	    if(UART_HMI.RxBuffer <= UART_HMI.pRxBuffer_out-RXBUFFER_SIZE)
		    UART_HMI.pRxBuffer_out -= RXBUFFER_SIZE;
	    return;
	}

	UART_HMI.TxBuffer[0] = UART_HMI.pRxBuffer_out[0];//复制slave地址
	UART_HMI.pRxBuffer_out++;//第一个地址不看
	cnt--;

	UART_HMI.TxBuffer[1] = UART_HMI.pRxBuffer_out[0];//复制操作码
	switch(*UART_HMI.pRxBuffer_out)//查看Function Code
	{
            case MODBUS_RDREGS: addr = UART_HMI.pRxBuffer_out[1] << 8 |
				       UART_HMI.pRxBuffer_out[2];
     				num  = UART_HMI.pRxBuffer_out[3] << 8 |
				       UART_HMI.pRxBuffer_out[4];
				UART_HMI.pRxBuffer_out += 5;
     				cnt -= 5;
     				hmi_r(addr,num);
     				UART_HMI.pRxBuffer_out += cnt;
     				break;
	    default: 	SEGGER_RTT_printf(0,"\r\n[hmi_main]:");
			while(cnt--)//还有没打印完的数据
			{
		    	   SEGGER_RTT_printf(0,"%3x",*UART_HMI.pRxBuffer_out++);
			   if(UART_HMI.RxBuffer <= (UART_HMI.pRxBuffer_out-RXBUFFER_SIZE))		//超出Buffer范围
				   UART_HMI.pRxBuffer_out = UART_HMI.RxBuffer;
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

#ifdef  USE_UART3_485
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);  //PB12输出高电平，使能写
#endif
    SEGGER_RTT_printf(0,"\r\n[hmi_test_resp]%x\r\n",
		  HAL_UART_Transmit(&UART_HMI.Handle,UART_HMI.TxBuffer,7,5000));
#ifdef  USE_UART3_485
    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);//PB12输出低电平，使能读
#endif
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
