#include "stm32f10x.h"
#include "lora.h"
#include "Delay.h"

uint8_t g_lora_state = LORA_STATE_DISCONNECT;

// 只初始化 PA0 PA1，写死透传模式，不碰串口！
void LoRa_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;

    // 开 GPIOA 时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);

    // PA0 M0 + PA1 M1 推挽输出
    GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOA, &GPIO_InitStruct);

    // ================= 写死透传 =================
    GPIO_ResetBits(GPIOA, GPIO_Pin_0); // M0=低
    GPIO_ResetBits(GPIOA, GPIO_Pin_1); // M1=低

    g_lora_state = LORA_STATE_CONNECT;
	
}

