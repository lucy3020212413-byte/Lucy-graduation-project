#include "freertos_demo.h"
#include "dht22.h"
#include "Serial.h"
#include "OLED.h"   
#include "lora.h"
#include "Delay.h"
#include "hongwaimenjin.h"

TaskHandle_t StartTask_Handler;      // 开始任务句柄
TaskHandle_t DisplayTask_Handler;    // 显示任务句柄

uint8_t g_oled_page = 1;             
float g_temp = 25.5f;
float g_humi = 50.5f;

static void start_task(void *pvParameters);
static void display_task(void *pvParameters);

void freertos_demo(void)
{
    // 创建开始任务
    xTaskCreate((TaskFunction_t)start_task,
                (const char*)"start_task",
                (uint16_t)START_TASK_STK_SIZE,
                (void*)NULL,
                (UBaseType_t)START_TASK_PRIO,
                (TaskHandle_t*)&StartTask_Handler);
                
    // 启动调度器
    vTaskStartScheduler();
}

static void start_task(void *pvParameters)
{
    taskENTER_CRITICAL(); 
    
    // 只创建 OLED 任务！！！
    xTaskCreate((TaskFunction_t)display_task,
                (const char*)"display_task",
                (uint16_t)DISPLAY_TASK_STK_SIZE,
                (void*)NULL,
                (UBaseType_t)DISPLAY_TASK_PRIO,
                (TaskHandle_t*)&DisplayTask_Handler);
                
    vTaskDelete(StartTask_Handler); 
    taskEXIT_CRITICAL();            
}

// ========== 只运行 OLED 显示任务 ==========
static void display_task(void *pvParameters)
{
    // 任务刚启动，强制重新初始化一次！
    OLED_Init();
    OLED_Clear();
    
    // 直接写死测试
    OLED_ShowString(1,1,"RTOS OK");
    OLED_ShowString(2,1,"OLED IS ON");
    OLED_ShowString(3,1,"HELLO WORLD");
    OLED_ShowString(4,1,"SUCCESS");
    
    while(1)
    {
        // 只延时，不刷新
        vTaskDelay(pdMS_TO_TICKS(500));
    }
}