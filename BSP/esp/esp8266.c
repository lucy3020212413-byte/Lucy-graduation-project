
#include "esp8266.h"
#include "./SYSTEM/delay/delay.h"   // 正点原子自带延时函数
#include "./SYSTEM/usart/usart.h"
/****************************************************************
 * 函数：esp8266_init
 * 功能：ESP8266 初始化，分时间发送 4 段标准 AT 指令
 * 顺序：
 * 1. AT          —— 测试是否在线
 * 2. AT+RST       —— 重启模块
 * 3. AT+CWMODE=1  —— 设置为STA模式
 * 4. AT+CWMODE?   —— 查询模式（确认成功）
 ****************************************************************/

//void esp8266_init()
//{
//	delay_ms(5000);
//Serial_Printf("AT+MQTTUSERCFG=0,1,\"haiyun1\",\"61HUR9Nplm\",\"version=2018-10-31&res=products%%2F61HUR9Nplm%%2Fdevices%%2Fhaiyun1&et=2190525243&method=md5&sign=vZ%%2Fz2KmC%%2F58NUIvrO7ma2A%%3D%%3D\",0,0,\"\"\r\n");
//delay_ms(5000);
//Serial_Printf("AT+MQTTCONN=0,\"mqtts.heclouds.com\",1883,1\r\n");
//	delay_ms(5000);
//Serial_Printf("AT+MQTTSUB=0,\"$sys/61HUR9Nplm/haiyun1/thing/property/post/reply\",0\r\n");
//		delay_ms(5000);
//}

