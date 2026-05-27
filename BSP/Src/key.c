#include "key.h"

void KEY_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    // 使能 GPIO 时钟
    KEY1_GPIO_CLK_ENABLE();
    KEY2_GPIO_CLK_ENABLE();
    KEY3_GPIO_CLK_ENABLE();
    KEY4_GPIO_CLK_ENABLE();

    // 配置按键引脚为输入模式，带上拉电阻
    GPIO_InitStruct.Pin = KEY1_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(KEY1_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = KEY2_PIN;
    HAL_GPIO_Init(KEY2_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = KEY3_PIN;
    HAL_GPIO_Init(KEY3_GPIO_PORT, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = KEY4_PIN;
    HAL_GPIO_Init(KEY4_GPIO_PORT, &GPIO_InitStruct);
}

KeyState_t KEY_Scan(void)
{
    // 静态变量保存每个按键上一次的电平状态（初始为高电平，因为按键按下为低）
    static uint8_t last_key1 = GPIO_PIN_SET;
    static uint8_t last_key2 = GPIO_PIN_SET;
    static uint8_t last_key3 = GPIO_PIN_SET;
    static uint8_t last_key4 = GPIO_PIN_SET;

    // ----- 按键1处理 -----
    uint8_t current_key1 = HAL_GPIO_ReadPin(KEY1_GPIO_PORT, KEY1_PIN);
    if (current_key1 == GPIO_PIN_RESET && last_key1 == GPIO_PIN_SET) {
        // 下降沿触发，加入消抖延时
        vTaskDelay(pdMS_TO_TICKS(10));
        // 再次读取确认
        if (HAL_GPIO_ReadPin(KEY1_GPIO_PORT, KEY1_PIN) == GPIO_PIN_RESET) {
            last_key1 = current_key1;  // 更新上次状态
            return KEY1_PRESSED;
        }
    }
    last_key1 = current_key1;

    // ----- 按键2处理 -----
    uint8_t current_key2 = HAL_GPIO_ReadPin(KEY2_GPIO_PORT, KEY2_PIN);
    if (current_key2 == GPIO_PIN_RESET && last_key2 == GPIO_PIN_SET) {
        vTaskDelay(pdMS_TO_TICKS(10));
        if (HAL_GPIO_ReadPin(KEY2_GPIO_PORT, KEY2_PIN) == GPIO_PIN_RESET) {
            last_key2 = current_key2;
            return KEY2_PRESSED;
        }
    }
    last_key2 = current_key2;

    // ----- 按键3处理 -----
    uint8_t current_key3 = HAL_GPIO_ReadPin(KEY3_GPIO_PORT, KEY3_PIN);
    if (current_key3 == GPIO_PIN_RESET && last_key3 == GPIO_PIN_SET) {
        vTaskDelay(pdMS_TO_TICKS(10));
        if (HAL_GPIO_ReadPin(KEY3_GPIO_PORT, KEY3_PIN) == GPIO_PIN_RESET) {
            last_key3 = current_key3;
            return KEY3_PRESSED;
        }
    }
    last_key3 = current_key3;

    // ----- 按键4处理 -----
    uint8_t current_key4 = HAL_GPIO_ReadPin(KEY4_GPIO_PORT, KEY4_PIN);
    if (current_key4 == GPIO_PIN_RESET && last_key4 == GPIO_PIN_SET) {
        vTaskDelay(pdMS_TO_TICKS(10));
        if (HAL_GPIO_ReadPin(KEY4_GPIO_PORT, KEY4_PIN) == GPIO_PIN_RESET) {
            last_key4 = current_key4;
            return KEY4_PRESSED;
        }
    }
    last_key4 = current_key4;

    return KEY_NONE;
}
