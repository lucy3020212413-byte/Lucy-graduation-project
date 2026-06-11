#ifndef __HONGWAIMENJIN_H
#define __HONGWAIMENJIN_H

#include "stm32f10x.h"

#define IR1_PIN     GPIO_Pin_4
#define IR1_PORT    GPIOB
#define IR2_PIN     GPIO_Pin_5
#define IR2_PORT    GPIOB

// 中断触发标志和时间戳
extern volatile uint8_t ir1_triggered;
extern volatile uint8_t ir2_triggered;
extern volatile uint32_t ir1_timestamp;
extern volatile uint32_t ir2_timestamp;

void HongWaiMenJin_Init(void);
uint8_t Infrared_GetState(uint8_t ir_num);
void Infrared_ClearTrigger(void);  // 清除触发标志

#endif