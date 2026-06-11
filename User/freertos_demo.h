#ifndef __FREERTOS_DEMO_H
#define __FREERTOS_DEMO_H

#include "FreeRTOS.h"
#include "task.h"
#include "timers.h"
#include "lora.h"  // 引入LoRa头文件，用于地址宏引用

// ================== 任务优先级配置（按合理逻辑调整） ==================
// 优先级数字越大，优先级越高（遵循：红外>按键>显示>传感器>LoRa>启动）
#define START_TASK_PRIO         1     // 开始任务（最低，创建完任务即删除）
#define LORA_TASK_PRIO          2     // LoRa通信任务
#define SENSOR_TASK_PRIO        3     // 传感器采集任务（DHT22）
#define DISPLAY_TASK_PRIO       4     // OLED显示任务
#define KEY_TASK_PRIO           5     // 按键任务（高优先级，响应及时）
#define COUNT_TASK_PRIO         6     // 人数统计任务（双红外，最高优先级）

// ================== 任务栈大小配置 ==================
#define START_TASK_STK_SIZE     128   // 开始任务栈大小
#define KEY_TASK_STK_SIZE       128   // 按键任务栈大小
#define DISPLAY_TASK_STK_SIZE   1024   // 显示任务栈大小（OLED刷屏需更大栈）
#define SENSOR_TASK_STK_SIZE    128   // 传感器采集任务栈大小
#define LORA_TASK_STK_SIZE      256   // LoRa任务栈大小（收发数据需更大栈）
#define COUNT_TASK_STK_SIZE     128   // 人数统计任务栈大小

// ================== 任务句柄声明 ==================
extern TaskHandle_t StartTask_Handler;      // 开始任务句柄
extern TaskHandle_t KeyTask_Handler;        // 按键任务句柄（新增）
extern TaskHandle_t DisplayTask_Handler;    // OLED显示任务句柄
extern TaskHandle_t SensorTask_Handler;     // 传感器采集任务句柄
extern TaskHandle_t LoRaTask_Handler;       // LoRa通信任务句柄（新增）
extern TaskHandle_t CountTask_Handler;      // 人数统计任务句柄（新增）

// ================== 全局变量声明 ==================
extern uint8_t g_oled_page;          // OLED当前页面（1-4，新增人数页）
extern float g_temp;                 // 温度值（保留1位小数）
extern float g_humi;                 // 湿度值（保留1位小数）
extern uint8_t g_lora_state;         // LoRa状态（0=未连接，1=已连接）
extern uint16_t g_people_count;      // 当前人数统计（新增，0-999）
extern uint8_t g_ir1_last, g_ir2_last; // 红外状态缓存（新增）

// ================== 宏定义补充 ==================
#define OLED_PAGE_MAX          4     // OLED最大页面数（1-4）
#define LORA_REPORT_INTERVAL   5     // LoRa自动上报间隔（秒）
#define IR_DETECT_INTERVAL     100   // 红外检测间隔（ms）

// ================== 函数声明 ==================
void freertos_demo(void);            // FreeRTOS入口函数

// 新增：按键/红外/LoRa相关辅助函数声明（按需）
void Key_Task(void *pvParameters);   // 按键任务函数
void Count_Task(void *pvParameters); // 人数统计任务函数
void LoRa_Task(void *pvParameters);  // LoRa通信任务函数

#endif
