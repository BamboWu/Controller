#ifndef __CODER_H
#define __CODER_H

#include "stm32f1xx_hal.h"
#include "SEGGER_RTT.h"

#define TIM_CHANNEL_12  ((uint32_t)0x1212)//让Encoder的Start Stop函数，进default

void coder_init(uint16_t div, uint8_t dir);
void coder_Z(void);
#endif /* __CODER_H */
