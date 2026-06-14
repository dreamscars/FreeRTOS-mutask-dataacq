#include "adc1.h"
#include "stm32f103xb.h"
#include "stm32f1xx_hal_cortex.h"
#include "sys.h"

/* ADC DMA数据缓冲区 */
uint16_t adc_dma_buffer[ADC_DMA_BUFFER_SIZE];


/**
 * @brief  ADC1初始化函数（寄存器版本）
 * @note   配置GPIO引脚为模拟输入模式，设置ADC工作参数并进行校准
 * @param  无
 * @retval 无
 */
void ADC1_Init(void)
{
    // ADC1初始化代码
    // 例如：配置ADC时钟、分辨率、采样时间等参数
    ADC_ADCX_CLK_ENABLE(); // 使能ADC1时钟
    // 其他ADC配置代码
    __HAL_RCC_GPIOA_CLK_ENABLE(); // 使能GPIOA时钟
    
    // 配置ADC通道对应的GPIO引脚
    // 例如：配置PA1为ADC通道1，PA2为ADC通道2，PA4为ADC通道4
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 其他ADC初始化代码
    RCC ->APB2RSTR |= RCC_APB2RSTR_ADC1RST; // 复位ADC1
    RCC ->APB2RSTR &= ~RCC_APB2RSTR_ADC1RST; // 释放ADC1复位
    RCC ->CFGR |= RCC_CFGR_ADCPRE_DIV6; // 设置ADC时钟分频
    ADC_ADCX -> CR1 |= 0X0 << 16; // 配置ADC工作模式为独立模式
    ADC_ADCX -> CR1 |= 0 << 8; //配置ADC为非扫描模式
    ADC_ADCX -> CR2 &= ~(1 << 1); // 配置ADC为连续转换模式
    ADC_ADCX -> CR2 |= 7 << 17; //配置ADC规则通道组转换的外部事件为SWSTART
    ADC_ADCX -> CR2 |= 1 << 20; // 配置ADC规则通道组转换由软件触发
    ADC_ADCX -> CR2 &= ~(1 << 11); // 配置ADC数据对齐方式为右对齐
    ADC_ADCX -> SQR1 &= ~(0xF << 20); // L[3:0] = 0，规则通道组转换序列长度清零
    ADC_ADCX -> SQR1 |= 2 << 20; // L[3:0] = 2，规则通道组转换序列长度为3
    ADC_ADCX -> SQR3 |= ADC_ADCX_CHANNEL_1; // 规则通道组转换序列1配置为ADC通道1
    ADC_ADCX -> SQR3 |= ADC_ADCX_CHANNEL_2 << 5; // 规则通道组转换序列2配置为ADC通道2
    ADC_ADCX -> SQR3 |= ADC_ADCX_CHANNEL_4 << 10; // 规则通道组转换序列3配置为ADC通道4
    ADC_ADCX -> CR2 |= 1 << 0; // 使能ADC1
    ADC_ADCX -> CR2 |= 1 << 3; //使能复位校准
    while(ADC_ADCX -> CR2 & (1 << 3)); // 等待复位校准完成
    ADC_ADCX -> CR2 |= 1 << 2; //使能开始校准
    while(ADC_ADCX -> CR2 & (1 << 2)); // 等待校准完成
}


/**
 * @brief  设置指定ADC通道的采样时间
 * @note   根据通道号选择SMPR1或SMPR2寄存器进行配置
 * @param  ADCx: ADC外设指针（如ADC1、ADC2等）
 * @param  channel: ADC通道号（0-18）
 * @param  sampling_time: 采样时间值（3位），可选值：
 *                      0: 1.5周期, 1: 7.5周期, 2: 13.5周期, 3: 28.5周期,
 *                      4: 41.5周期, 5: 55.5周期, 6: 71.5周期, 7: 239.5周期
 * @retval 无
 */
void ADC_Sampling_time_set(ADC_TypeDef *ADCx, uint32_t channel, uint32_t sampling_time)
{
    // 配置ADC采样时间的代码
    // 例如：根据channel参数选择对应的采样时间寄存器，并设置采样时间
    // 这里需要根据具体的ADC型号和寄存器定义进行相应的设置
    // ...
    if (channel < 10) {
        ADCx->SMPR2 &= ~(0x7 << (channel * 3)); // 清除原有的采样时间设置
        ADCx->SMPR2 |= (sampling_time << (channel * 3)); // 设置新的采样时间
    } else {
        channel -= 10; // 调整通道号以适应SMPR1寄存器
        ADCx->SMPR1 &= ~(0x7 << (channel * 3)); // 清除原有的采样时间设置
        ADCx->SMPR1 |= (sampling_time << (channel * 3)); // 设置新的采样时间
    }
}

/**
 * @brief  ADC DMA初始化函数（寄存器版本）
 * @note   配置DMA1通道1用于ADC1数据传输，循环模式，外设到内存
 * @param  无
 * @retval 无
 */
void ADC_DMA_Init(void)
{
    // 初始化ADC1
    ADC1_Init();
        
    // 使能ADC DMA请求
    ADC_ADCX->CR2 |= ADC_CR2_DMA;
    
    // 配置ADC为连续转换模式（配合DMA循环模式）
    ADC_ADCX->CR2 |= ADC_CR2_CONT;

    ADC_Sampling_time_set(ADC_ADCX, ADC_ADCX_CHANNEL_1, 7); // 设置通道1采样时间为239.5周期
    ADC_Sampling_time_set(ADC_ADCX, ADC_ADCX_CHANNEL_2, 7); // 设置通道2采样时间为239.5周期
    ADC_Sampling_time_set(ADC_ADCX, ADC_ADCX_CHANNEL_4, 7); // 设置通道4采样时间为239.5周期

    // 使能DMA1时钟
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;
    
    // 禁用DMA1通道1（配置前必须先禁用）
    DMA1_Channel1->CCR &= ~DMA_CCR_EN;
    
    // 等待DMA通道禁用完成
    while(DMA1_Channel1->CCR & DMA_CCR_EN);
    
    // 配置DMA1通道1参数
    DMA1_Channel1->CPAR = (uint32_t)&(ADC_ADCX->DR);    // 外设地址：ADC1数据寄存器
    DMA1_Channel1->CMAR = (uint32_t)adc_dma_buffer; // 内存地址：ADC数据缓冲区
    DMA1_Channel1->CNDTR = ADC_DMA_BUFFER_SIZE;     // 数据传输数量
    
    // 配置DMA控制寄存器
    DMA1_Channel1->CCR = 0;                         // 先清零
    DMA1_Channel1->CCR |= DMA_CCR_PL_1;             // 优先级：高
    DMA1_Channel1->CCR |= DMA_CCR_MSIZE_0;          // 内存数据宽度：16位
    DMA1_Channel1->CCR |= DMA_CCR_PSIZE_0;          // 外设数据宽度：16位
    DMA1_Channel1->CCR |= DMA_CCR_MINC;             // 内存地址递增
    // DMA_CCR_PINC 默认不设置 = 外设地址不递增
    DMA1_Channel1->CCR |= DMA_CCR_CIRC;             // 循环模式
    // DMA_CCR_DIR 默认不设置 = 外设到内存
    DMA1_Channel1->CCR |= DMA_CCR_TCIE;              // 传输完成中断使能

    // 设置中断优先级（必须 <= configMAX_SYSCALL_INTERRUPT_PRIORITY）
    // NVIC_PRIORITYGROUP_2下，优先级值=优先级组值×抢占优先级范围
    // configMAX_SYSCALL_INTERRUPT_PRIORITY=191对应优先级11，需设置>=11才安全
    NVIC_Init(11, 0, ADC_ADCX_DMA1_IRQn, NVIC_PRIORITYGROUP_2);
}


/**
 * @brief  ADC DMA1中断处理函数
 * @note   处理ADC DMA传输完成中断，清除中断标志并设置数据就绪标志
 * @param  无
 * @retval 无
 */
void ADC_ADCX_DMA1_IRQHandler(void)
{
    if ((DMA1 -> ISR & (1 << 1))) {
        // 处理ADC DMA传输完成事件
        xSemaphoreGiveFromISR( xSemaphore, NULL ); // 释放信号量，通知数据处理任务
        DMA1->IFCR |= DMA_ISR_TCIF1;
    }
}

void ADC_Start_DMA(void)
{
    // 启动ADC转换并使能DMA传输
    ADC_ADCX->CR2 |= ADC_CR2_SWSTART; // 软件触发开始转换
    DMA1_Channel1->CCR |= DMA_CCR_EN; // 使能DMA通道1
}
