#ifndef  __DI_H
#define  __DI_H

#include "stm32f1xx_hal.h"
#include "SEGGER_RTT.h"
#include "main.h"
#include "valve.h"

#define DI_STOP_MSK  0X20
#define DI_TEST_MSK  0X10
#define DI_CHAN_MSK  0X0F

void di_init(void);
void di_main(void);

#endif /* __DI_H */
