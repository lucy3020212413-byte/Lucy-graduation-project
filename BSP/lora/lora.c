#include "lora.h"
#include "stm32f1xx_hal.h"

uint8_t g_lora_state = LORA_STATE_DISCONNECT;

// HAL库版本 LoRa 初始化（只设置 PA0 PA1 为低电平 → 透传模式）
void LoRa_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 开 GPIOA 时钟
    __HAL_RCC_GPIOA_CLK_ENABLE();

    // PA0, PA1 推挽输出
    GPIO_InitStruct.Pin   = GPIO_PIN_0 | GPIO_PIN_1;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // M0=0, M1=0 → 透传模式
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET);
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET);

    g_lora_state = LORA_STATE_CONNECT;
}