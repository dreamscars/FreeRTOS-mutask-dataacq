#ifndef __LED_H
#define __LED_H

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"

typedef enum {
    Flowing_lights = 0,
    Twinkleing,
    All_On,
} LEDMode_State_t;

#define   LED1_PIN                  GPIO_PIN_5
#define   LED1_GPIO_PORT            GPIOB
#define   LED1_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOB_CLK_ENABLE()
#define   LED1(state)               HAL_GPIO_WritePin(GPIOB, LED1_PIN, (state) ? GPIO_PIN_RESET : GPIO_PIN_SET) // 这里LED是低电平点亮
#define   LED1_TOGGLE()             HAL_GPIO_TogglePin(GPIOB, LED1_PIN) // 切换LED1状态

#define   LED2_PIN                  GPIO_PIN_6
#define   LED2_GPIO_PORT            GPIOB
#define   LED2_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOB_CLK_ENABLE()
#define   LED2(state)               HAL_GPIO_WritePin(GPIOB, LED2_PIN, (state) ? GPIO_PIN_RESET : GPIO_PIN_SET) // 这里LED是低电平点亮
#define   LED2_TOGGLE()             HAL_GPIO_TogglePin(GPIOB, LED2_PIN) // 切换LED2状态

#define   LED3_PIN                  GPIO_PIN_7
#define   LED3_GPIO_PORT            GPIOB
#define   LED3_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOB_CLK_ENABLE()
#define   LED3(state)               HAL_GPIO_WritePin(GPIOB, LED3_PIN, (state) ? GPIO_PIN_RESET : GPIO_PIN_SET) // 这里LED是低电平点亮
#define   LED3_TOGGLE()             HAL_GPIO_TogglePin(GPIOB, LED3_PIN) // 切换LED3状态

#define   LED4_PIN                  GPIO_PIN_8
#define   LED4_GPIO_PORT            GPIOB
#define   LED4_GPIO_CLK_ENABLE()    __HAL_RCC_GPIOB_CLK_ENABLE()
#define   LED4(state)               HAL_GPIO_WritePin(GPIOB, LED4_PIN, (state) ? GPIO_PIN_RESET : GPIO_PIN_SET) // 这里LED是低电平点亮
#define   LED4_TOGGLE()             HAL_GPIO_TogglePin(GPIOB, LED4_PIN) // 切换LED4状态

void LED_Init(void);
void LED_Mode_Control(LEDMode_State_t mode);
void Single_LED_Control(uint8_t led_num, uint8_t state);

#endif /* __LED_H */