#include "stm32f10x.h"
#include "hongwaimenjin.h"
#include "Delay.h"

// 中断触发标志和时间戳
volatile uint8_t ir1_triggered = 0;
volatile uint8_t ir2_triggered = 0;
volatile uint32_t ir1_timestamp = 0;
volatile uint32_t ir2_timestamp = 0;

void HongWaiMenJin_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct;
    EXTI_InitTypeDef EXTI_InitStruct;
    NVIC_InitTypeDef NVIC_InitStruct;
    
    // 使能GPIOB时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB | RCC_APB2Periph_AFIO, ENABLE);
    
    // 配置IR1 (PB4) 为上拉输入
    GPIO_InitStruct.GPIO_Pin = IR1_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU; // 上拉输入
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IR1_PORT, &GPIO_InitStruct);
    
    // 配置IR2 (PB5) 为上拉输入
    GPIO_InitStruct.GPIO_Pin = IR2_PIN;
    GPIO_InitStruct.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_InitStruct.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(IR2_PORT, &GPIO_InitStruct);
    
    // 配置EXTI线4 (PB4)
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource4);
    EXTI_InitStruct.EXTI_Line = EXTI_Line4;
    EXTI_InitStruct.EXTI_Mode = EXTI_Mode_Interrupt;
    EXTI_InitStruct.EXTI_Trigger = EXTI_Trigger_Falling; // 下降沿触发（红外检测到物体时）
    EXTI_InitStruct.EXTI_LineCmd = ENABLE;
    EXTI_Init(&EXTI_InitStruct);
    
    // 配置EXTI线5 (PB5)
    GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource5);
    EXTI_InitStruct.EXTI_Line = EXTI_Line5;
    EXTI_Init(&EXTI_InitStruct);
    
    // 配置NVIC - EXTI4中断
    NVIC_InitStruct.NVIC_IRQChannel = EXTI4_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStruct.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStruct);
    
    // 配置NVIC - EXTI9_5中断（包含EXTI5）
    NVIC_InitStruct.NVIC_IRQChannel = EXTI9_5_IRQn;
    NVIC_InitStruct.NVIC_IRQChannelPreemptionPriority = 1;
    NVIC_InitStruct.NVIC_IRQChannelSubPriority = 1;
    NVIC_Init(&NVIC_InitStruct);
}

uint8_t Infrared_GetState(uint8_t ir_num)
{
    uint8_t state = 0;
    if(ir_num == 1)
    {
        state = (GPIO_ReadInputDataBit(IR1_PORT, IR1_PIN) == RESET) ? 1 : 0;
    }
    else if(ir_num == 2)
    {
        state = (GPIO_ReadInputDataBit(IR2_PORT, IR2_PIN) == RESET) ? 1 : 0;
    }
    return state;
}

void Infrared_ClearTrigger(void)
{
    ir1_triggered = 0;
    ir2_triggered = 0;
}

// EXTI4中断服务函数 (IR1) - 只设置标志，不在中断中延时
void EXTI4_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line4) != RESET)
    {
        ir1_triggered = 1;
        ir1_timestamp = TIM2->CNT; // 记录时间戳
        EXTI_ClearITPendingBit(EXTI_Line4);
    }
}

// EXTI9_5中断服务函数 (IR2) - 只设置标志，不在中断中延时
void EXTI9_5_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line5) != RESET)
    {
        ir2_triggered = 1;
        ir2_timestamp = TIM2->CNT; // 记录时间戳
        EXTI_ClearITPendingBit(EXTI_Line5);
    }
}