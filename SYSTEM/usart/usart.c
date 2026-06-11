#include "./SYSTEM/sys/sys.h"
#include "./SYSTEM/usart/usart.h"
#include "stdio.h"
#include "string.h"
#include <stdarg.h>

// �����׼��ȫ�ֱ��� һģһ��������
char Serial_RxPacket[100];
uint8_t Serial_RxFlag;

// LoRa接收缓冲区 - 使用队列避免数据覆盖
#define RX_QUEUE_SIZE 4  // 队列大小
#define PACKET_SIZE 50   // 每个数据包最大长度
char Serial3_RxPacket[PACKET_SIZE];          // 当前接收缓冲区
char Serial3_RxQueue[RX_QUEUE_SIZE][PACKET_SIZE]; // 数据包队列
uint8_t Serial3_QueueHead = 0;               // 队列头指针
uint8_t Serial3_QueueTail = 0;               // 队列尾指针
uint8_t Serial3_RxFlag;

UART_HandleTypeDef huart1;
UART_HandleTypeDef huart3;

uint8_t u1_rx_buf;
uint8_t u3_rx_buf;

// 入队函数
static uint8_t Serial3_Enqueue(const char *data)
{
    uint8_t next_head = (Serial3_QueueHead + 1) % RX_QUEUE_SIZE;
    strncpy(Serial3_RxQueue[Serial3_QueueHead], data, PACKET_SIZE-1);
    Serial3_RxQueue[Serial3_QueueHead][PACKET_SIZE-1] = '\0';
    
    if(next_head == Serial3_QueueTail) {
        // 队列满，覆盖最旧的数据（移动尾指针）
        Serial3_QueueTail = (Serial3_QueueTail + 1) % RX_QUEUE_SIZE;
    }
    Serial3_QueueHead = next_head;
    return 1;
}

// 出队函数
uint8_t Serial3_Dequeue(char *data)
{
    if(Serial3_QueueHead == Serial3_QueueTail) {
        return 0; // 队列空
    }
    strcpy(data, Serial3_RxQueue[Serial3_QueueTail]);
    Serial3_QueueTail = (Serial3_QueueTail + 1) % RX_QUEUE_SIZE;
    return 1;
}

#if 1
#if (__ARMCC_VERSION >= 6010050)
__asm(".global __use_no_semihosting\n\t");
__asm(".global __ARM_use_no_argv \n\t");
#else
#pragma import(__use_no_semihosting)
struct __FILE { int handle; };
#endif

int _ttywrch(int ch) { return ch; }
void _sys_exit(int x) { x = x; }
char *_sys_command_string(char *cmd, int len) { return NULL; }
FILE __stdout;

int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)&ch, 1, 10);
    return ch;
}
#endif

// ��ʼ�� USART1��115200 ����ԭ��һ����
void Serial_Init(void)
{
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    HAL_UART_Init(&huart1);

    HAL_UART_Receive_IT(&huart1, &u1_rx_buf, 1);
}

// ��ʼ�� USART3������ԭ��һ����
void Serial3_Init(uint32_t BaudRate)
{
    huart3.Instance = USART3;
    huart3.Init.BaudRate = BaudRate;
    huart3.Init.WordLength = UART_WORDLENGTH_8B;
    huart3.Init.StopBits = UART_STOPBITS_1;
    huart3.Init.Parity = UART_PARITY_NONE;
    huart3.Init.Mode = UART_MODE_TX_RX;
    huart3.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    HAL_UART_Init(&huart3);

    HAL_UART_Receive_IT(&huart3, &u3_rx_buf, 1);
}

// ���ͺ��� ����ԭ�� ��ȫһ��������
void Serial_SendByte(uint8_t Byte)
{
    HAL_UART_Transmit(&huart1, &Byte, 1, 100);
}

void Serial_SendString(char *String)
{
    HAL_UART_Transmit(&huart1, (uint8_t*)String, strlen(String), 100);
}

void Serial3_SendByte(uint8_t Byte)
{
    HAL_UART_Transmit(&huart3, &Byte, 1, 100);
}

void Serial3_SendString(char *String)
{
    HAL_UART_Transmit(&huart3, (uint8_t*)String, strlen(String), 100);
}

void Serial_Printf(char *format, ...)
{
    char buf[512];
    va_list arg;
    va_start(arg, format);
    vsprintf(buf, format, arg);
    va_end(arg);
    Serial_SendString(buf);
}

// �жϽ����߼� ����ԭ�� ��ȫһ��������
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    static uint16_t pRx1 = 0;
    static uint16_t pRx3 = 0;

    if(huart->Instance == USART1)
    {
        Serial_RxPacket[pRx1++] = u1_rx_buf;

        if(u1_rx_buf == '\n'){
            Serial_RxPacket[pRx1] = 0;
            pRx1 = 0;
            Serial_RxFlag = 1;
        }
        if(pRx1 >= 99) pRx1 = 0;

        HAL_UART_Receive_IT(&huart1, &u1_rx_buf, 1);
    }

    if(huart->Instance == USART3)
    {
        static uint16_t pRx3 = 0;
        Serial3_RxPacket[pRx3++] = u3_rx_buf;

        if(u3_rx_buf == '\n'){
            Serial3_RxPacket[pRx3] = 0;
            Serial3_Enqueue(Serial3_RxPacket); // 入队
            Serial3_RxFlag = 1;
            pRx3 = 0;
        }
        if(pRx3 >= PACKET_SIZE-1) pRx3 = 0;

        HAL_UART_Receive_IT(&huart3, &u3_rx_buf, 1);
    }
}

// ������ HAL �ײ� MSP ��ʼ�����Զ�ƥ�� PA9 PA10 PB10 PB11��
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef gpio_conf = {0};

    if(huart->Instance == USART1)
    {
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        // PA9 TX
        gpio_conf.Pin = GPIO_PIN_9;
        gpio_conf.Mode = GPIO_MODE_AF_PP;
        gpio_conf.Pull = GPIO_PULLUP;
        gpio_conf.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &gpio_conf);

        // PA10 RX
        gpio_conf.Pin = GPIO_PIN_10;
        gpio_conf.Mode = GPIO_MODE_INPUT;
        HAL_GPIO_Init(GPIOA, &gpio_conf);

        HAL_NVIC_SetPriority(USART1_IRQn, 1, 1);
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }

    if(huart->Instance == USART3)
    {
        __HAL_RCC_USART3_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        // PB10 TX
        gpio_conf.Pin = GPIO_PIN_10;
        gpio_conf.Mode = GPIO_MODE_AF_PP;
        gpio_conf.Pull = GPIO_PULLUP;
        gpio_conf.Speed = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOB, &gpio_conf);

        // PB11 RX
        gpio_conf.Pin = GPIO_PIN_11;
        gpio_conf.Mode = GPIO_MODE_INPUT;
        HAL_GPIO_Init(GPIOB, &gpio_conf);

        HAL_NVIC_SetPriority(USART3_IRQn, 1, 2);
        HAL_NVIC_EnableIRQ(USART3_IRQn);
    }
}

void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

void USART3_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart3);
}