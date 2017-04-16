#ifndef __VALVE_H
#define __VALVE_H

#include "stm32f1xx_hal.h"
#include "SEGGER_RTT.h"

#define BANK0 0X01
#define BANK1 0X02
#define BANK2 0X04

#define ON_OFF_MAX 4
//最多可设置的开关对数，受on_offs_mask制约，应小于16
#define FLASH_ADDR ((uint32_t)0x0801FC00)
//flash地址从0x08000000开始，使用第128个1KB的page存储电磁阀的参数
#define DATA_MARK  ((uint32_t)0x5050A0A0)
//用来标记flash中的数据存在，不可取为0xFFFFFFFF

typedef uint16_t on_off[2]; //电磁阀开启关闭时刻count的一对设定值的结构
typedef struct valve_param_s
{
  on_off    on_offs[ON_OFF_MAX];
  uint16_t  on_offs_mask;   //有效的开关的掩码，为1的bit所对应的on_offs有效
  uint16_t  high_duration;  //高压通路的持续时间
} valve_param_t;

typedef struct valve_modify_s
{
  uint16_t  on_degree;    //打开角度的整数部分
  uint16_t  on_fraction;  //打开角度的小数部分
  uint16_t  off_degree;   //关闭角度的整数部分
  uint16_t  off_fraction; //关闭角度的小数部分
  uint16_t  high_degree;  //高压持续角度的整数部分
  uint16_t  high_fraction;//高压持续角度的小数部分
  uint8_t   on_off_valid; //开关参数是否有效状态位设置，1为有效，0无效
  uint8_t   high_valid;   //是否更新高压持续值，1为有效，0无效
  uint8_t   on_off_index; //设置第on_offs_index对开关的参数
} valve_modify_t;

typedef struct valve_display_s
{
  uint16_t  on_degree[ON_OFF_MAX];    //打开角度的整数部分
  uint16_t  on_fraction[ON_OFF_MAX];  //打开角度的小数部分
  uint16_t  off_degree[ON_OFF_MAX];   //关闭角度的整数部分
  uint16_t  off_fraction[ON_OFF_MAX]; //关闭角度的小数部分
  uint16_t  high_degree;              //高压持续角度的整数部分
  uint16_t  high_fraction;            //高压持续角度的小数部分
  uint16_t  on_offs_mask; //开关参数是否有效状态位设置，1为有效，0无效
} valve_display_t;

void valve_init(void);
void valve_channel_on(uint8_t channel);
void valve_channel_off(uint8_t channel, uint8_t Hi_Lo);

void valve_params_load(void);
void valve_params_store(void);
void valve_params_modify(uint8_t channel, valve_modify_t new_param);
void valve_params_display(uint8_t channel, valve_display_t * p_param);
#endif /* __VALVE_H */
