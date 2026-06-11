#ifndef __SERIAL_H
#define __SERIAL_H
#include "stm32f10x.h"  
#include <stdio.h>

void Serial_Init(void);
void Serial_SendByte(uint8_t Byte);
void Serial_SendArray(uint8_t *Array, uint16_t Length);
void Serial_SendString(char *String);
void Serial_SendNumber(uint32_t Number, uint8_t Length);
void Serial_Printf(char *format, ...);
void Serial1_ClearRxBuffer(void);
char* Serial1_GetRxBuffer(void);
uint8_t Serial1_GetRxCompleteFlag(void);

uint8_t Serial_GetRxFlag(void);
uint8_t Serial_GetRxData(void);
uint8_t Serial2_ReadByte(void);



void Serial2_Init(uint32_t BaudRate);
void Serial2_SendByte(uint8_t Byte);
void Serial2_SendArray(uint8_t *Array, uint16_t Length);
uint8_t Serial2_ReceiveArray(uint8_t *buf, uint8_t max_len);
void Serial2_SendString(char *String);

#endif
