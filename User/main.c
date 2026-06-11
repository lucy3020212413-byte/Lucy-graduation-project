#include "stm32f10x.h"                  // Device header
#include  "dht22.h"
#include "Serial.h"
#include "mq2.h"
#include <OLED.h>
#include "freertos_demo.h"
#include "Delay.h"
#include "hongwaimenjin.h"
#include "collect.h"

// 声明USART2接收变量
extern uint8_t Serial2_RxData;
extern uint8_t Serial2_RxFlag;

// 人流计数变量
int people_cnt = 0;   // 初始0
static uint8_t first_trigger = 0;    // 第一个触发的传感器 (1=ir1, 2=ir2)
static uint32_t trigger_timeout = 0; // 触发超时计时器

// 定时器初始化（用于时间戳）
void TIM2_Init(void)
{
    TIM_TimeBaseInitTypeDef TIM_TimeBaseInitStruct;
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
    TIM_TimeBaseInitStruct.TIM_Prescaler = 72 - 1; // 72MHz / 72 = 1MHz
    TIM_TimeBaseInitStruct.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInitStruct.TIM_Period = 0xFFFF; // 最大计数
    TIM_TimeBaseInitStruct.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_TimeBaseInit(TIM2, &TIM_TimeBaseInitStruct);
    TIM_Cmd(TIM2, ENABLE);
}



void FanLight_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    // 1. �� GPIOB ʱ�ӣ���������Ҫ��һ����
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

    // 2. ���� PB12 PB13 �������
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);

    // 3. Ĭ�Ϲر�
    GPIO_ResetBits(GPIOB, GPIO_Pin_12);
    GPIO_ResetBits(GPIOB, GPIO_Pin_13);
}



int main(void)
	{

		char buf[24]; 
			 float temp, humi;
		 float smoke;
		
		
		
		
		Delay_Init();
		HongWaiMenJin_Init();
		FanLight_GPIO_Init();
		TIM2_Init();  // 初始化定时器用于时间戳
			DHT22_Init();
		
			MQ2_Init();
		Serial2_Init(115200);
			OLED_Init();
			Serial_Init();
		LoRa_Init();
		Delay_ms(1500);
		Collect_Init();
			OLED_ShowString(1, 1, "Temp:");
			OLED_ShowString(2, 1, "Humi:");
			OLED_ShowString(3, 1, "gas:");
		OLED_ShowString(4, 1, "person:"); 

          

			while(1)
	{
		DHT22_ReadData(&temp, &humi);
		smoke = MQ2_GetPercent();
		
		// ========== 使用中断方式接收数据 ==========
		if(Serial2_RxFlag == 1)
		{
			static char cmd_buf[10];
			static uint8_t cmd_len = 0;
			
			uint8_t rx = Serial2_RxData;
			Serial2_RxFlag = 0;  // 清除标志
			
			// 存储接收到的字符
			if(rx != '\n' && rx != '\r' && cmd_len < 9)
			{
				cmd_buf[cmd_len++] = rx;
				cmd_buf[cmd_len] = '\0';
			}
			else
			{
				// 接收到完整指令，开始解析
				// 只响应帧头为66的指令（设备2）
				if(cmd_len >= 3 && cmd_buf[0] == '8' && cmd_buf[1] == '8')
				{
					if(cmd_buf[2] == '1')      GPIO_SetBits(GPIOB, GPIO_Pin_12);  // 风扇开
					else if(cmd_buf[2] == '0') GPIO_ResetBits(GPIOB, GPIO_Pin_12);  // 风扇关
					else if(cmd_buf[2] == '2') GPIO_SetBits(GPIOB, GPIO_Pin_13);   // 灯光开
					else if(cmd_buf[2] == '3') GPIO_ResetBits(GPIOB, GPIO_Pin_13); // 灯光关
				}
				cmd_len = 0;  // 重置缓冲区
			}
		}

		// ===== 红外检测处理 =====
		// IR1触发处理
		if(ir1_triggered)
		{
			if(first_trigger == 0)
			{
				first_trigger = 1;
				trigger_timeout = TIM2->CNT;
			}
			else if(first_trigger == 2)
			{
				// ir2先触发，现在ir1触发 = 离开
				if(people_cnt > 0) people_cnt--;
				first_trigger = 0;
			}
			ir1_triggered = 0;
		}
		
		// IR2触发处理
		if(ir2_triggered)
		{
			if(first_trigger == 0)
			{
				first_trigger = 2;
				trigger_timeout = TIM2->CNT;
			}
			else if(first_trigger == 1)
			{
				// ir1先触发，现在ir2触发 = 进入
				people_cnt++;
				first_trigger = 0;
			}
			ir2_triggered = 0;
		}
		
		// 超时重置（2秒内没有第二个触发则丢弃）
		if(first_trigger != 0 && (TIM2->CNT - trigger_timeout) > 2000000)  // 2秒（1MHz时钟）
		{
			first_trigger = 0;
		}
		
		OLED_ShowNum(1, 6, temp, 2);
		OLED_ShowString(1, 8, ".");
		OLED_ShowNum(1, 9, (int)(temp*10)%10, 1);
		OLED_ShowString(1,10, "C");
			
		OLED_ShowNum(2, 6, humi, 2);
		OLED_ShowString(2, 8, ".");
		OLED_ShowNum(2, 9, (int)(humi*10)%10, 1);
		OLED_ShowString(2,10, "%");
		
		OLED_ShowNum(3,7, smoke, 2);
		OLED_ShowString(3,11,"%");
		
		OLED_ShowNum(4,10, people_cnt, 2);
		
		// ========== 自动控制逻辑 ==========
		// 当温度>40、湿度>70、气体>30时自动打开风扇
		if(temp > 40.0 || humi > 70.0 || smoke > 30.0)
		{
			GPIO_SetBits(GPIOB, GPIO_Pin_12);  // 自动打开风扇
		}
		// 当温度<20、湿度<30、气体<5时自动关闭风扇
		else if(temp < 20.0 && humi < 30.0 && smoke < 5.0)
		{
			GPIO_ResetBits(GPIOB, GPIO_Pin_12);  // 自动关闭风扇
		}
		
		// 修改帧结构，添加设备ID并使用浮点数格式，符合接收端解析要求
		#define DEVICE_ID 1  // 设备编号
		sprintf(buf, "%d:T:%.1f H:%.1f G:%.1f P:%d\n", DEVICE_ID, temp, humi, smoke, people_cnt);

		Serial2_SendString(buf);
		Delay_ms(3500);   // 降低主循环延迟到100ms
	}
}