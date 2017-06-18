#ifndef __CMD_H
#define __CMD_H

#include "stm32f1xx_hal.h"
#include "SEGGER_RTT.h"
#include "valve.h"
#include "coder.h"

void cmd_h(void);
void cmd_l(void);
void cmd_s(void);
void cmd_p(void);
void cmd_m(void);
void cmd_c(void);
void cmd_i(void);
void cmd_r(void);
void cmd_g(void);
void cmd_d(void);
void cmd_e(void);

void cmd_main(void);

#endif /* __CMD_H */
