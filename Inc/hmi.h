#ifndef  __HMI_H
#define  __HMI_H

#include "stm32f1xx_hal.h"
#include "SEGGER_RTT.h"
#include "crc.h"
#include "valve.h"
#include "coder.h"

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

#define MODBUS_RDREGS  0X03  //读寄存器的操作码
#define MODBUS_WRREG   0X06  //写寄存器的操作码
#define MODBUS_WRREGS  0X10  //写多个寄存器的操作码

#define MODBUS_SYS_MSK 0XF000//特殊用途地址掩码
#define MODBUS_CHA_MSK 0X0F00//通道索引掩码
#define MODBUS_TYP_MSK 0X0030//参数类型掩码
#define MODBUS_IDX_MSK 0X000F//开关参数对索引掩码

#define MODBUS_SYS_HOUR  0X2000//运行小时数记录地址
#define MODBUS_SYS_MIN   0X1000//运行分钟数记录地址
#define MODBUS_SYS_SAVE  0X3000//保存参数到flash
#define MODBUS_SYS_APPLY 0X4000//应用当前参数
#define MODBUS_TYP_OFF   0X0000//关闭角参数类型代码
#define MODBUS_TYP_ON    0X0010//开启角参数类型代码
#define MODBUS_TYP_VALID 0X0020//开关对有效值参数类型代码
#define MODBUS_TYP_HIGH  0X0030//高压时间参数类型代码

#if defined(USE_UART3_485)
#define USART_HMI  USART3
#elif defined(USE_UART1_232)
#define USART_HMI  USART1
#endif

//接收缓冲输出指针移位的函数块宏定义
#define UART_HMI_PRXBUF_(cnt) {\
UART_HMI.pRxBuffer_out-=cnt;\
if(UART_HMI.RxBuffer>UART_HMI.pRxBuffer_out)\
UART_HMI.pRxBuffer_out+=RXBUFFER_SIZE;\
}

void hmi_init(void);
void hmi_main(void);
void hmi_r(uint16_t addr, uint16_t num);
void hmi_w(uint16_t addr, uint16_t num);
void hmi_test_resp(void);

#ifdef USE_UART3_485
void hmi_test_485(void);
#endif
#endif /* __HMI_H */
