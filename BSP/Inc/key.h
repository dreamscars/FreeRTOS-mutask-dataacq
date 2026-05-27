#ifndef __KEY_H
#define __KEY_H

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
/***************根据自己需求更改****************/
#define KEY1_PIN                GPIO_PIN_4
#define KEY1_GPIO_PORT          GPIOB
#define KEY1_GPIO_CLK_ENABLE() do{ RCC->APB2ENR |= 1 << 3; }while(0)

#define KEY2_PIN                GPIO_PIN_15
#define KEY2_GPIO_PORT          GPIOB
#define KEY2_GPIO_CLK_ENABLE() do{ RCC->APB2ENR |= 1 << 3; }while(0)

#define KEY3_PIN                GPIO_PIN_8
#define KEY3_GPIO_PORT          GPIOA
#define KEY3_GPIO_CLK_ENABLE() do{ RCC->APB2ENR |= 1 << 2; }while(0)

#define KEY4_PIN                GPIO_PIN_15
#define KEY4_GPIO_PORT          GPIOA
#define KEY4_GPIO_CLK_ENABLE() do{ RCC->APB2ENR |= 1 << 2; }while(0)

typedef enum {
    KEY_NONE = 0,
    KEY1_PRESSED,
    KEY2_PRESSED,
    KEY3_PRESSED,
    KEY4_PRESSED
} KeyState_t;

void KEY_Init(void);
KeyState_t KEY_Scan(void);

#endif /* __KEY_H */