#ifndef __MQ2_H
#define __MQ2_H

#include "stm32f10x.h"



#define MQ2_ADC     ADC1
#define MQ2_CHANNEL ADC_Channel_4
#define MQ2_PIN    GPIO_Pin_4
#define MQ2_PORT   GPIOA

uint8_t MQ2_CheckAlarm(uint16_t threshold);
void MQ2_Init(void);
uint16_t MQ2_GetValue(void);

float MQ2_GetVoltage(void);
float MQ2_GetPercent(void);

#endif

