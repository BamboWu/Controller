#ifndef __CRC_H
#define __CRC_H

#include "stm32f1xx_hal.h"

uint16_t crc_calculate(uint8_t* pdata, uint8_t len);

#endif
