#ifndef  __HMI_H
#define  __HMI_H

#include "stm32f1xx_hal.h"
#include "SEGGER_RTT.h"

#define  USE_UART3_485
//#define  USE_UART1_232
#ifndef  USE_UART3_485
#ifndef  USE_UART1_232
#error   至少定义USE_UART3_485和USE_UART1_232中的一个！！！
#endif //USE_UART1_232
#endif //USE_UART3_485

//给UART_HMI_s.Status用的标注状态的宏
#define  RX_CPLT   0x80
#define  TX_CPLT   0x80

#define  RXBUFFER_SIZE 256
#define  TXBUFFER_SIZE 128

typedef struct UART_HMI_s
{
    UART_HandleTypeDef Handle;
    uint8_t Status;//最高位指示完成状况，后7位是当前接收字符数
    uint8_t RxBuffer[RXBUFFER_SIZE];
    uint8_t TxBuffer[TXBUFFER_SIZE];
    uint8_t * pRxBuffer_in;//FIFO缓冲中第一个未读数据的指针
    uint8_t * pRxBuffer_out;//FIFO缓冲中第一个可存数据的指针
} UART_HMI_t;

#if defined(USE_UART3_485)
#define USART_HMI  USART3
#elif defined(USE_UART1_232)
#define USART_HMI  USART1
#endif

void hmi_init(void);
void hmi_main(void);

#ifdef USE_UART3_485
void hmi_test_485(void);
#endif
#endif /* __HMI_H */
