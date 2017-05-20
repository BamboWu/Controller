#include "hmi.h"

UART_HandleTypeDef UART3Handle;

unsigned char UART3_STA = 0;

unsigned char RxBuffer[RXBUFFER_SIZE];
unsigned char TxBuffer[TXBUFFER_SIZE];

#ifdef USE_UART1_232
UART_HandleTypeDef UART1Handle;
unsigned char UART1_STA = 0;
unsigned char RxBuffer1[RXBUFFER_SIZE];
unsigned char TxBuffer1[TXBUFFER_SIZE];
#endif

/**@brief 用于初始化与触摸屏通信的函数
 *
 * @detail 
 */
void hmi_init(void)
{

#ifdef USE_UART1_232
    UART1Handle.Instance        = USART1;

    UART1Handle.Init.BaudRate   = 1200;
    UART1Handle.Init.WordLength = UART_WORDLENGTH_8B;
    UART1Handle.Init.Parity     = UART_PARITY_NONE;
    UART1Handle.Init.StopBits   = UART_STOPBITS_1;
    UART1Handle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UART1Handle.Init.Mode       = UART_MODE_TX_RX;

    if(HAL_UART_DeInit(&UART1Handle) != HAL_OK)
	SEGGER_RTT_printf(0,"\r\n[HMI_Init]DeInit err\r\n");
    if(HAL_UART_Init(&UART1Handle) != HAL_OK)
	SEGGER_RTT_printf(0,"\r\n[HMI_Init]Init err\r\n");

    UART1_STA = 0;
    HAL_UART_Receive_IT(&UART1Handle, (uint8_t *)RxBuffer1, 1);
#endif

    UART3Handle.Instance        = USART3;

    UART3Handle.Init.BaudRate   = 9600;
    UART3Handle.Init.WordLength = UART_WORDLENGTH_8B;
    UART3Handle.Init.Parity     = UART_PARITY_NONE;
    UART3Handle.Init.StopBits   = UART_STOPBITS_1;
    UART3Handle.Init.HwFlowCtl  = UART_HWCONTROL_NONE;
    UART3Handle.Init.Mode       = UART_MODE_TX_RX;

    if(HAL_UART_DeInit(&UART3Handle) != HAL_OK)
	SEGGER_RTT_printf(0,"\r\n[HMI_Init]DeInit err\r\n");
    if(HAL_UART_Init(&UART3Handle) != HAL_OK)
	SEGGER_RTT_printf(0,"\r\n[HMI_Init]Init err\r\n");

	SEGGER_RTT_printf(0,"\r\n[HMI_Init]Init OK\r\n");
    UART3_STA = 0;
    HAL_UART_Receive_IT(&UART3Handle, (uint8_t *)RxBuffer, 1);
}

/**@brief  图形交互界面通信数据处理后台程序
 *
 * @detail 通过UASRT1与触摸屏交换信息，根据触摸屏的指令配置参数、控制运转
 *
 */
void hmi_main(void)
{
    //SEGGER_RTT_printf(0,"\r\n[hmi_main]UART3_STA=%x\r\n",UART3_STA);
    //SEGGER_RTT_printf(0,"\r\nRxBuffer=%x\r\n",RxBuffer[0]);
    //HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_RESET);  
    if(UART3_STA & RX_CPLT)
    {
	SEGGER_RTT_printf(0,"%2x",RxBuffer[0]);
	UART3_STA &= ~RX_CPLT;
    	HAL_UART_Receive_IT(&UART3Handle, (uint8_t *)RxBuffer, 1);
    }
    //else
    //{
    //    SEGGER_RTT_printf(0,"\r\n[hmi_main]%x\r\n",UART3Handle.State);
    //    HAL_Delay(100);
    //}
}

#ifdef  USE_UART1_232
/**@brief  UART1调试程序
 *
 */
void hmi_test(void)
{
    unsigned char char_num=0;
    int key_val;
    while(1)
    {
	if(UART1_STA & RX_CPLT)
    	{
		UART1_STA &= ~RX_CPLT;
	        SEGGER_RTT_printf(0,"%c",RxBuffer1[0]);
		HAL_UART_Receive_IT(&UART1Handle, (uint8_t *)RxBuffer1, 1);
	}
	if(UART1_STA & TX_CPLT)
	{
		UART1_STA &= ~TX_CPLT;
		SEGGER_RTT_printf(0,"\r\n[hmi_test]Tx cplt\r\n");
		HAL_UART_Receive_IT(&UART1Handle, (uint8_t *)RxBuffer1, 1);
	}

	key_val = SEGGER_RTT_GetKey();
	if(0 > key_val)
		continue;
	else if('\n' == key_val)
	{
		TxBuffer1[char_num] = '\r';
		++char_num;
		TxBuffer1[char_num] = '\n';
		++char_num;
		//SEGGER_RTT_printf(0,"\r\n[hmi_test]to Tx\r\n");
		HAL_UART_Transmit_IT(&UART1Handle,(uint8_t*)TxBuffer1,char_num);
		char_num = 0;
	}
	else
	{
		TxBuffer1[char_num] = key_val;
		//SEGGER_RTT_printf(0,"%x",key_val);
		++char_num;
	}
    }
}
#endif

#ifdef  USE_UART3_485
/**@brief  485调试程序
 *
 */
void hmi_test_485(void)
{
    unsigned char char_num=0;

    HAL_GPIO_WritePin(GPIOB,GPIO_PIN_12,GPIO_PIN_SET);  //PB12输出高电平，使能写
    TxBuffer[0] = ':';
    HAL_UART_Transmit_IT(&UART3Handle, (uint8_t *)TxBuffer, 1);
    while(1)
    {
	if(UART3_STA & TX_CPLT)
	{
		UART3_STA &= ~TX_CPLT;
		SEGGER_RTT_printf(0,"\r\n[hmi_test]Tx cplt\r\n");
		if(9 <= char_num)
			char_num = 0;
		else
			char_num++;
		TxBuffer[0] = 'H';
		TxBuffer[1] = 'e';
		TxBuffer[2] = 'l';
		TxBuffer[3] = 'l';
		TxBuffer[4] = 'o';
		TxBuffer[5] = '0'+char_num;
		TxBuffer[6] = '\r';
		TxBuffer[7] = '\n';
		HAL_UART_Transmit_IT(&UART3Handle, (uint8_t *)TxBuffer, 8);
	}
	else
	{
		SEGGER_RTT_printf(0,"\r\n[hmi_test]wait Tx cplt\r\n");
		HAL_Delay(1000);
	}
    }
}
#endif
