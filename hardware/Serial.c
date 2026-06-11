#include "stm32f10x.h"                  // Device header
#include <stdio.h>
#include <stdarg.h>
#include <string.h>

uint8_t Serial_RxData;
uint8_t Serial_RxFlag;
uint8_t Serial1_RxBuffer[128];
uint16_t Serial1_RxLen = 0; 
uint8_t Serial1_RxCompleteFlag = 0;

// 串口2指令缓冲区和状态机
static char Serial2_cmd_buf[10];
static uint8_t Serial2_cmd_len = 0;
static uint8_t Serial2_state = 0;  // 0=等待帧头, 1=等待第二个帧头, 2=等待指令



void Serial_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
	
	GPIO_InitTypeDef GPIO_InitStructure;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);
	
	USART_InitTypeDef USART_InitStructure;
	USART_InitStructure.USART_BaudRate = 115200;
	USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
	USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
	USART_InitStructure.USART_Parity = USART_Parity_No;
	USART_InitStructure.USART_StopBits = USART_StopBits_1;
	USART_InitStructure.USART_WordLength = USART_WordLength_8b;
	USART_Init(USART1, &USART_InitStructure);
	
	USART_ITConfig(USART1, USART_IT_RXNE, ENABLE);
	
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_4);
	
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 6;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_Init(&NVIC_InitStructure);
	
	USART_Cmd(USART1, ENABLE);
}

uint8_t Serial2_RxData;
uint8_t Serial2_RxFlag;

void Serial2_Init(uint32_t BaudRate)
{
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE);
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
    
    // USART2_TX -> PA2�����츴�������
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // USART2_RX -> PA3���������룩
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStructure);
    
    // USART2���ã���LoRaģ�鲨����һ�£�����9600��
    USART_InitTypeDef USART_InitStructure;
    USART_InitStructure.USART_BaudRate = BaudRate;
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None;
    USART_InitStructure.USART_Mode = USART_Mode_Tx | USART_Mode_Rx;
    USART_InitStructure.USART_Parity = USART_Parity_No;
    USART_InitStructure.USART_StopBits = USART_StopBits_1;
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;
    USART_Init(USART2, &USART_InitStructure);
    
    // ========== 启用接收中断 ==========
    USART_ITConfig(USART2, USART_IT_RXNE, ENABLE);
    
    // 配置NVIC
    NVIC_InitTypeDef NVIC_InitStructure;
    NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 5;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_Init(&NVIC_InitStructure);
    
    USART_Cmd(USART2, ENABLE);
}

// USART2中断服务函数
void USART2_IRQHandler(void)
{
    if (USART_GetITStatus(USART2, USART_IT_RXNE) == SET)
    {
        uint8_t rx = USART_ReceiveData(USART2);
        Serial2_RxData = rx;
        Serial2_RxFlag = 1;  // 设置接收标志
        
        // 使用状态机解析指令：收到"88"+指令后立即执行
        switch(Serial2_state)
        {
            case 0: // 等待第一个'8'
                if(rx == '8')
                {
                    Serial2_state = 1;
                }
                break;
                
            case 1: // 等待第二个'8'
                if(rx == '8')
                {
                    Serial2_state = 2;
                }
                else
                {
                    Serial2_state = 0; // 不是'8'，重新等待
                }
                break;
                
            case 2: // 收到指令字节，立即执行
                if(rx == '1')      GPIO_SetBits(GPIOB, GPIO_Pin_12);  // 风扇开
                else if(rx == '0') GPIO_ResetBits(GPIOB, GPIO_Pin_12);  // 风扇关
                else if(rx == '2') GPIO_SetBits(GPIOB, GPIO_Pin_13);   // 灯光开
                else if(rx == '3') GPIO_ResetBits(GPIOB, GPIO_Pin_13); // 灯光关
                Serial2_state = 0; // 回到初始状态
                break;
        }
        
        USART_ClearITPendingBit(USART2, USART_IT_RXNE);
    }
}

void Serial_SendByte(uint8_t Byte)
{
	USART_SendData(USART1, Byte);
	while (USART_GetFlagStatus(USART1, USART_FLAG_TXE) == RESET);
}

void Serial_SendArray(uint8_t *Array, uint16_t Length)
{
	uint16_t i;
	for (i = 0; i < Length; i ++)
	{
		Serial_SendByte(Array[i]);
	}
}

void Serial_SendString(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)
	{
		Serial_SendByte(String[i]);
	}
}

uint32_t Serial_Pow(uint32_t X, uint32_t Y)
{
	uint32_t Result = 1;
	while (Y --)
	{
		Result *= X;
	}
	return Result;
}

void Serial_SendNumber(uint32_t Number, uint8_t Length)
{
	uint8_t i;
	for (i = 0; i < Length; i ++)
	{
		Serial_SendByte(Number / Serial_Pow(10, Length - i - 1) % 10 + '0');
	}
}

int fputc(int ch, FILE *f)
{
	Serial_SendByte(ch);
	return ch;
}

void Serial_Printf(char *format, ...)
{
	char String[100];
	va_list arg;
	va_start(arg, format);
	vsprintf(String, format, arg);
	va_end(arg);
	Serial_SendString(String);
}

uint8_t Serial1_GetRxCompleteFlag(void)
{
	if (Serial1_RxCompleteFlag == 1)
	{
		Serial1_RxCompleteFlag = 0;
		return 1;
	}
	return 0;
}

char* Serial1_GetRxBuffer(void)
{
	return (char*)Serial1_RxBuffer;
}

void Serial1_ClearRxBuffer(void)
{
	__disable_irq(); //
	Serial1_RxLen = 0;
	Serial1_RxCompleteFlag = 0;
	memset(Serial1_RxBuffer, 0, sizeof(Serial1_RxBuffer));
	__enable_irq(); // 
}

void USART1_IRQHandler(void)
{
	if (USART_GetITStatus(USART1, USART_IT_RXNE) == SET)
	{
		uint8_t rx_byte = USART_ReceiveData(USART1); // 
		
		if (Serial1_RxLen < 127) // 
		{
			Serial1_RxBuffer[Serial1_RxLen++] = rx_byte;
		}
		
		// 2. 
		if (Serial1_RxLen >= 2)
		{
			if (Serial1_RxBuffer[Serial1_RxLen-2] == '\r' && Serial1_RxBuffer[Serial1_RxLen-1] == '\n')
			{
				Serial1_RxBuffer[Serial1_RxLen] = '\0'; // 
				Serial1_RxCompleteFlag = 1; // 
				Serial1_RxLen = 0; //
			}
		}
		
		USART_ClearITPendingBit(USART1, USART_IT_RXNE);
	}
}

//////////////////////////////////////////////
void Serial2_SendByte(uint8_t Byte)
{
    USART_SendData(USART2, Byte);
    while (USART_GetFlagStatus(USART2, USART_FLAG_TXE) == RESET);
}

// USART2�������飨�������Serial_SendArray�߼���
void Serial2_SendArray(uint8_t *Array, uint16_t Length)
{
    uint16_t i;
    for (i = 0; i < Length; i ++)
    {
        Serial2_SendByte(Array[i]);
    }
}

// USART2�������飨��ѯʽ������LoRa��
uint8_t Serial2_ReceiveArray(uint8_t *buf, uint8_t max_len)
{
    uint8_t recv_len = 0;
    while(recv_len < max_len)
    {
        if(USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET) // �����ݽ���
        {
            buf[recv_len++] = USART_ReceiveData(USART2);
        }
        else
        {
            break; // ���������˳�
        }
    }
    return recv_len;
}
uint8_t Serial2_ReadByte(void)
{
    if (USART_GetFlagStatus(USART2, USART_FLAG_RXNE) == SET)
    {
        return USART_ReceiveData(USART2);
    }
    return 0;
}
void Serial2_SendString(char *String)
{
	uint8_t i;
	for (i = 0; String[i] != '\0'; i ++)
	{
		Serial2_SendByte(String[i]);
	}
}