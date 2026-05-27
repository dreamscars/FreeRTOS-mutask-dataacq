#ifndef __ADC1_H
#define __ADC1_H
#include "stm32f1xx_hal.h"
#include "freertos_taskstart.h"
#include <stdint.h>

#define        ADC_ADCX                            ADC1
#define        ADC_ADCX_CLK_ENABLE()               __HAL_RCC_ADC1_CLK_ENABLE()
#define        ADC_ADCX_CHANNEL_1                  1
#define        ADC_ADCX_CHANNEL_2                  2
#define        ADC_ADCX_CHANNEL_4                  4

#define        ADC_ADCX_CHANNEL_1_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOA_CLK_ENABLE()
#define        ADC_ADCX_CHANNEL_2_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOA_CLK_ENABLE()
#define        ADC_ADCX_CHANNEL_4_GPIO_CLK_ENABLE()   __HAL_RCC_GPIOA_CLK_ENABLE()

#define        ADC_ADCX_DMA1_IRQn                   DMA1_Channel1_IRQn
#define        ADC_ADCX_DMA1_IRQHandler             DMA1_Channel1_IRQHandler

/* ADC DMA缓冲区大小（根据ADC通道数设置，当前3个通道） */
#define        ADC_DMA_BUFFER_SIZE                 50*3

/* ADC DMA数据缓冲区（外部声明，在adc1.c中定义） */
extern uint16_t adc_dma_buffer[ADC_DMA_BUFFER_SIZE];


void ADC1_Init(void);
void ADC_Sampling_time_set(ADC_TypeDef *ADCx, uint32_t channel, uint32_t sampling_time);
void ADC_DMA_Init(void);
void ADC_Start_DMA(void);


#endif