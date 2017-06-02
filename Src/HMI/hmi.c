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
    if(UART_HMI.Status & RX_CPLT)
    {
	uint8_t cnt;
	cnt = UART_HMI.Status &= 0X7F;
	UART_HMI.Status = 0;
	if(2 < cnt)
	    SEGGER_RTT_printf(0,"\r\nCRC:%4x",
			      crc_calculate(UART_HMI.pRxBuffer_out,cnt-2));
        SEGGER_RTT_printf(0,"\r\n%3d:",cnt);
	while(cnt--)//还有没打印完的数据
	{
	    SEGGER_RTT_printf(0,"%3x",*UART_HMI.pRxBuffer_out++);
	    if(UART_HMI.RxBuffer <= (UART_HMI.pRxBuffer_out-RXBUFFER_SIZE))
		//超出Buffer范围
		UART_HMI.pRxBuffer_out = UART_HMI.RxBuffer;
	}
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
