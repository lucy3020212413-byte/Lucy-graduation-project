#include  "dht22.h"
static uint8_t dht22_pwr_stable = 0;

// 【精准SysTick微秒延时】基于系统时钟，误差＜1μs，满足手册时序要求
static void delay_us(uint32_t us)
{
    if(us == 0) return;
    uint32_t ticks = (SystemCoreClock / 1000000) * us;
    SysTick->LOAD = ticks - 1;  // 重装计数值
    SysTick->VAL  = 0;          // 清空当前值
    // 开启SysTick，使用内核时钟
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;
    // 等待计数完成
    while(!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk));
    SysTick->CTRL = 0;          // 关闭SysTick
}


static void DHT22_SetOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

// 快速切换为输入模式（上拉，匹配手册5.1K上拉要求）
static void DHT22_SetInput(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;
    GPIO_Init(GPIOB, &GPIO_InitStructure);
}

// DHT22初始化：含官方要求的上电2S稳定延时
void DHT22_Init(void)
{
    // 使能GPIOB时钟
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
    // 初始化为输出，总线拉高（手册闲置高电平要求）
    DHT22_SetOutput();
    GPIO_SetBits(GPIOB, GPIO_Pin_11);
    
    // 官方要求：上电后等待2S越过不稳定状态
    if(dht22_pwr_stable == 0)
    {
        uint32_t i;
        for(i=0; i<2000; i++) delay_us(1000); // 精准2S延时
        dht22_pwr_stable = 1;
    }
}

// 读取温湿度数据：完全对齐官方时序，返回0成功，1失败
uint8_t DHT22_ReadData(float *temp, float *humi)
{
    uint8_t buf[5] = {0};
    uint8_t i, j;

    /************************ 1. 发送官方标准起始信号 ************************/
    DHT22_SetOutput();
    GPIO_ResetBits(GPIOB, GPIO_Pin_11);  // 拉低总线
    delay_us(1000);                      // 官方典型值1ms（≥800μs，满足要求）
    GPIO_SetBits(GPIOB, GPIO_Pin_11);    // 释放总线（拉高）
    delay_us(20);                        // 短延时，等待总线电平稳定

    /************************ 2. 等待传感器官方标准响应 ************************/
    DHT22_SetInput();
   
    uint32_t timeout = 100;
    while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == SET && timeout--) delay_us(1);
    if(timeout == 0) return 1; // 无响应，返回失败
    
    delay_us(80);                        // 等待传感器拉低阶段结束（手册80μs）
    if(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) != SET) return 1; // 未拉高，失败
    delay_us(80);                        // 等待传感器拉高阶段结束（手册80μs）

    /************************ 3. 读取40位数据：按官方位时序判断 ************************/
    for (i = 0; i < 5; i++)  // 5个字节：湿度高、湿度低、温度高、温度低、校验
    {
        buf[i] = 0;
        for (j = 0; j < 8; j++)  // 每个字节8位，高位先出（手册要求）
        {
            // 等待数据位起始的低电平（手册50μs），超时失败
            timeout = 60;
            while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == RESET && timeout--) delay_us(1);
            if(timeout == 0) return 1;
            
            delay_us(40);                // 延时40μs，按手册判断高低电平
            buf[i] <<= 1;                // 高位先出，左移一位
            // 手册：高电平>40μs=1，<40μs=0
            if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == SET)
            {
                buf[i] |= 0x01;
                // 等待1的高电平结束（手册70μs），超时失败
                timeout = 80;
                while(GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_11) == SET && timeout--) delay_us(1);
                if(timeout == 0) return 1;
            }
        }
    }

    /************************ 4. 官方校验规则：和校验 ************************/
    if ((buf[0] + buf[1] + buf[2] + buf[3]) == buf[4])
    {
        // 湿度：16位，实际值=数值/10（手册分辨率0.1%RH）
        *humi = (float)(buf[0] * 256 + buf[1]) / 10.0f;
        // 温度：16位，最高位为符号位（手册要求），实际值=数值/10（分辨率0.1℃）
        *temp = (float)((buf[2] & 0x7F) * 256 + buf[3]) / 10.0f;
        if (buf[2] & 0x80) *temp = -(*temp); // 最高位1为负温度
        return 0; // 读取成功
    }
    return 1; // 校验失败，数据无效
}
