#ifndef __DHT22_H
#define __DHT22_H

#include "stdint.h"  
#include "stm32f10x.h"

void DHT22_Init(void);
void DHT22_SetOutput(void);
void DHT22_SetInput(void);
uint8_t DHT22_ReadData(float *temp, float *humi);


#endif
