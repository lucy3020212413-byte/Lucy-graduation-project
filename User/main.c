#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "./SYSTEM/delay/delay.h"
#include "./BSP/LED/led.h"
#include "./BSP/LCD/lcd.h"
#include "./BSP/KEY/key.h"
#include "./BSP/SRAM/sram.h"
#include "./MALLOC/malloc.h"
#include "./BSP/TOUCH/touch.h"
#include "lvgl_demo.h"
#include "lora.h"
//#include "esp8266.h"

int main(void)
{
    HAL_Init();                         /* 初始化HAL库 */
    sys_stm32_clock_init(RCC_PLL_MUL9); /* 设置时钟, 72Mhz */
    delay_init(72);                     /* 延时初始化 */
    Serial_Init();					// LoRa     → 波特率 9600
    led_init();                         /* 初始化LED */
    lcd_init();                        /* 初始化LCD */
    key_init();                         /* 初始化按键 */
    tp_dev.init();                      /* 触摸屏初始化 */
    my_mem_init(SRAMIN);                /* 初始化内部SRAM内存池 */
 delay_ms(3000);
Serial_Printf("AT+MQTTUSERCFG=0,1,\"1\",\"2\",\"3\",0,0,\"\"\r\n");
delay_ms(3000);
Serial_Printf("AT+MQTTCONN=0,\"mqtts..com\",1883,1\r\n");
	delay_ms(3000);
Serial_Printf("AT+MQTTSUB=0,\"$sys/2/1/thing/property/post/reply\",0\r\n");
		delay_ms(1000);
	
	Serial3_Init(115200);
	
	LoRa_Init();
	
	
   lvgl_demo();                   
}
