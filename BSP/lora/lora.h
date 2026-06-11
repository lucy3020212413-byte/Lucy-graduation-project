#ifndef __LORA_H
#define __LORA_H

#include "stm32f1xx_hal.h"

#define LORA_STATE_DISCONNECT 0
#define LORA_STATE_CONNECT    1

extern uint8_t g_lora_state;

void LoRa_Init(void);

#endif