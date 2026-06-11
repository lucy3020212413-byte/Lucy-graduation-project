#ifndef __USART_H
#define __USART_H

#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef g_uart1_handle;
extern UART_HandleTypeDef g_uart2_handle;

extern uint8_t  g_uart1_rx_buf[];
extern uint16_t g_uart1_rx_sta;

extern uint8_t  g_uart2_rx_buf[];
extern uint16_t g_uart2_rx_sta;

void uart1_init(uint32_t baudrate);
void uart2_init(uint32_t baudrate);
void Serial_Printf(char *format, ...);
	
void Serial_Init(void);
void Serial3_Init(uint32_t BaudRate);
void Serial3_SendString(char *String);

extern char Serial3_RxPacket[50];
extern uint8_t Serial3_RxFlag;

// 队列操作函数
extern uint8_t Serial3_Dequeue(char *data);


#endif
