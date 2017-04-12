#ifndef __CODER_H
#define __CODER_H

#include "stm32f1xx_hal.h"
#include "SEGGER_RTT.h"

void coder_init(uint16_t div, uint8_t dir);
void coder_on(void);
#endif /* __CODER_H */
