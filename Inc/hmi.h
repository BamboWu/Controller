#ifndef  __HMI_H
#define  __HMI_H

#include "stm32f1xx_hal.h"
#include "SEGGER_RTT.h"

#define  USE_UART1_232

//给UARTx_STA用的标注状态的宏
#define  RX_CPLT   0x20
#define  TX_CPLT   0x40

#define  RXBUFFER_SIZE 16
#define  TXBUFFER_SIZE 256

void hmi_init(void);
void hmi_main(void);

#ifdef  USE_UART1_232
void hmi_test(void);
#endif //USE_UART1_232

#endif /* __HMI_H */
