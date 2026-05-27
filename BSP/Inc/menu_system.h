#ifndef __MENU_SYSTEM_H
#define __MENU_SYSTEM_H

#include "stm32f1xx_hal.h"
#include <stdint.h>

typedef enum {
    MENU_MAIN = 0,
    MENU_LED_MODE_SELECT,
    MENU_SINGLE_LED_CTRL,
    MENU_DATA_SHOW,
    MENU_LED_MODE_RUNNING,
} MenuState_t;

void Menu_Init(void);
void Menu_HandleKey(uint8_t key);
void Menu_LEDModeStep(void);
uint8_t Menu_IsDataShowActive(void);

#endif
