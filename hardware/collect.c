#include "stm32f10x.h"
#include "collect.h"
#include <string.h>
#include "DHT22.h"
#include "mq2.h"
#include "hongwaimenjin.h"


#define RX_BUF_SIZE 11
uint8_t rx_buf[RX_BUF_SIZE];  // 大数据组载体
uint16_t rx_index = 0;
uint8_t Frame_Ready = 0; 

void Collect_Init(void)
{
    rx_buf[0] = 0x66;
    rx_buf[1] = 0x88;
    rx_buf[2] = 0x22;   // DHT22 设备号
    rx_buf[7] = 0x33;   // MQ2 设备号
    rx_buf[9] = 0x99;  // 红外设备号
    rx_index = 11;      // 固定一帧12字节
}


void Collect_BuildFrame(void)
{
	  Frame_Ready = 0; 
    float temp, humi;
    uint16_t temp_val, humi_val;
    uint8_t  percent_val;
    // ===================== 读取 DHT22 数据 =====================
    if(DHT22_ReadData(&temp, &humi) == 0)  // 成功读到数据
    {
        temp_val = (uint16_t)(temp * 10);   // 28.5℃ → 285
        humi_val = (uint16_t)(humi * 10);   // 45.0% → 450
        rx_buf[3] = (temp_val >> 8) & 0xFF;  // 温度高8位
        rx_buf[4] = temp_val & 0xFF;         // 温度低8位
        rx_buf[5] = (humi_val >> 8) & 0xFF;  // 湿度高8位
        rx_buf[6] = humi_val & 0xFF;         // 湿度低8位
			
			Frame_Ready = 1;
    }
		else
    {
		 Frame_Ready=0;
	   }

    // ===================== MQ2 占位 =====================
		
      percent_val = MQ2_GetPercent();
      uint8_t mq2_val = percent_val;  
      rx_buf[8] = mq2_val; 


    // ===================== 红外占位 =====================
    rx_buf[10] = 0x01;   // 
		
		
}

uint8_t* MY__BUF(void)
{

	Collect_BuildFrame();
  return rx_buf;
}



uint8_t Is_Frame_Ready(void)
{
	
    return Frame_Ready;
}

