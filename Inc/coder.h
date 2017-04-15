#ifndef __CODER_H
#define __CODER_H

#include "stm32f1xx_hal.h"
#include "SEGGER_RTT.h"
#include "valve.h"

#define TIM_CHANNEL_12  ((uint32_t)0x1212)//让Encoder的Start Stop函数，进default
#define CODER_EVT_MAX   (ON_OFF_MAX*3)*12 //最多可能有的事件数=开关数*3*12个通道

typedef enum
{
    channel_keep,
    channel_on,
    channel_off_H,
    channel_off_L,
}coder_evt_type;

typedef struct coder_evt_s coder_evt_t;//提前先做typedef，结构体中可包含同类指针
struct coder_evt_s
{
    uint16_t coder_count;
    uint8_t  evt_channel;
    coder_evt_type evt_type;
    coder_evt_t * next;
};

void coder_init(uint16_t div, uint8_t dir);
void coder_Z(void);
void coder_evt_insert(coder_evt_t evt);
void coder_evt_remove(coder_evt_t evt);
void coder_evt_gather(void);

#endif /* __CODER_H */
