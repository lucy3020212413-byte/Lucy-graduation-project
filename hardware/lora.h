#ifndef __LORA_H
#define __LORA_H

#include "stm32f10x.h"
#include "Serial.h"

#define LORA_UART       USART2    // LoRa接的串口
//#define LORA_AUX_PIN    GPIO_Pin_15
//#define LORA_AUX_PORT   GPIOC
#define LORA_M0_PIN     GPIO_Pin_0
#define LORA_M0_PORT    GPIOA
#define LORA_M1_PIN     GPIO_Pin_1
#define LORA_M1_PORT    GPIOA

// ========== 地址配置（和软件里的模块地址对应！） ==========
#define LORA_MY_ADDR    0x02      // 从机1的模块地址（软件里设的2）
#define LORA_HOST_ADDR  0x01      // 主机的模块地址（软件里设的1）

// ========== 状态宏 ==========
#define LORA_STATE_DISCONNECT 0
#define LORA_STATE_CONNECT    1

// ========== 函数声明 ==========
void LoRa_Init(void);





#endif

