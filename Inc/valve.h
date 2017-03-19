#ifndef __VALVE_H
#define __VALVE_H

#include "stm32f1xx_hal.h"
#include "SEGGER_RTT.h"

#define BANK0 0X01
#define BANK1 0X02
#define BANK2 0X04

void valve_init(void);
void valve_channel_on(uint8_t channel);
void valve_channel_off(uint8_t channel, uint8_t Hi_Lo);
#endif /* __VALVE_H */
