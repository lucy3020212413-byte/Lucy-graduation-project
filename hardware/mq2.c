#include "mq2.h"
#include "stm32f10x.h"
#include "Delay.h"

// ���ݴ�ֵ���������ݴ�״̬�µĴ�ֵ
static float g_mq2_baseline = 0.0f;

// 移动平均滤波缓冲区（5个样本）
#define FILTER_SAMPLES 5
static float g_mq2_filter_buf[FILTER_SAMPLES] = {0};
static uint8_t g_filter_index = 0;

/**
 * @brief  MQ2���ݴ�校��
 */
void MQ2_Calibrate(void)
{
    uint32_t sum = 0;
    uint8_t i;
    
    // ȡ10�δ��ֵ����
    for(i = 0; i < 10; i++)
    {
        sum += MQ2_GetValue();
        Delay_ms(50);
    }
    
    g_mq2_baseline = (sum / 10.0f * 5.0f) / 4095.0f;
}

void MQ2_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    ADC_InitTypeDef ADC_InitStructure;

    // 1. ʹ��ʱ��
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_ADC1, ENABLE);
    RCC_ADCCLKConfig(RCC_PCLK2_Div6); // ADCʱ�ӷ�Ƶ��72/6=12MHz������ADCҪ��

    // 2. ����Pa4Ϊģ�����루����������
    GPIO_InitStructure.GPIO_Pin = MQ2_PIN;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AIN; // ģ������ģʽ
    GPIO_Init(MQ2_PORT, &GPIO_InitStructure);

    // 3. ����ADC1������ת��������������
    ADC_DeInit(MQ2_ADC);
    ADC_InitStructure.ADC_Mode = ADC_Mode_Independent; // ����ģʽ
    ADC_InitStructure.ADC_ScanConvMode = DISABLE;      // ��ͨ��
    ADC_InitStructure.ADC_ContinuousConvMode = DISABLE; // ����ת��
    ADC_InitStructure.ADC_ExternalTrigConv = ADC_ExternalTrigConv_None; // ��������
    ADC_InitStructure.ADC_DataAlign = ADC_DataAlign_Right; // �Ҷ���
    ADC_InitStructure.ADC_NbrOfChannel = 1; // ͨ����=1
    ADC_Init(MQ2_ADC, &ADC_InitStructure);

    // 4. ʹ��ADC1
    ADC_Cmd(MQ2_ADC, ENABLE);

    // 5. ADCУ׼
    ADC_ResetCalibration(MQ2_ADC);
    while(ADC_GetResetCalibrationStatus(MQ2_ADC));
    ADC_StartCalibration(MQ2_ADC);
    while(ADC_GetCalibrationStatus(MQ2_ADC));
}

/**
 * @brief  ��ȡMQ2����Ũ�ȣ�ADCԭʼֵ��0~4095��
 */
uint16_t MQ2_GetValue(void)
{
    // ����ADCͨ���Ͳ���ʱ��
    ADC_RegularChannelConfig(MQ2_ADC, MQ2_CHANNEL, 1, ADC_SampleTime_239Cycles5);
    // ��������ת��
    ADC_SoftwareStartConvCmd(MQ2_ADC, ENABLE);
    // �ȴ�ת�����
    while(!ADC_GetFlagStatus(MQ2_ADC, ADC_FLAG_EOC));
    // ����ת�����
    return ADC_GetConversionValue(MQ2_ADC);
}

/**
 * @brief  ��ȡMQ2����Ũ�ȣ�带移动平均滤波
 */
uint16_t MQ2_GetFilteredValue(void)
{
    uint16_t raw_val = MQ2_GetValue();
    uint32_t sum = 0;
    uint8_t i;
    
    // 加入缓冲区
    g_mq2_filter_buf[g_filter_index] = raw_val;
    g_filter_index = (g_filter_index + 1) % FILTER_SAMPLES;
    
    // 计算平均值
    for(i = 0; i < FILTER_SAMPLES; i++)
    {
        sum += (uint32_t)g_mq2_filter_buf[i];
    }
    
    return (uint16_t)(sum / FILTER_SAMPLES);
}

/**
 * @brief  ���������⣨threshold=��ֵ����2000��
 * @retval 1=���꣬0=����
 */
uint8_t MQ2_CheckAlarm(uint16_t threshold)
{
    uint16_t mq2_val = MQ2_GetValue();
    return (mq2_val > threshold) ? 1 : 0;
}
float MQ2_GetVoltage(void)
{
    uint16_t adc_val = MQ2_GetFilteredValue();  // 使用滤波后的值
    return (adc_val * 5.0f) / 4095.0f;  // 12位ADC，参考电压5V
}

// 2. ��ȡ����Ũ�Ȱٷֱ� 0~100%
float MQ2_GetPercent(void)
{
    float voltage = MQ2_GetVoltage();
    float percent;
    float diff_voltage;

    // �������ݴ�ֵ���ɼ���������
    if(g_mq2_baseline > 0)
    {
        // �������ݴ�ֵ�����ɵľ���
        diff_voltage = voltage - g_mq2_baseline;
    }
    else
    {
        // û�жϵ�情况下使用默认基准
        diff_voltage = voltage - 0.4f;  // MQ2���ܿ���������ڴ�0.3~0.5V
    }

    // 5V供电 MQ2传感器专用计算
	if(diff_voltage < 0)      // 基线状态 = 0%
		percent = 0.0f;
	else if(diff_voltage > 4.5f)   // 最大值 = 100%
		percent = 100.0f;
	else                     // 0~4.5V 映射到 0~100%
		percent = diff_voltage * 22.22f;

    return percent;
}