#include "led.h"

void LED_Init(void)
{
    // 使能GPIO时钟
    __HAL_RCC_GPIOB_CLK_ENABLE();
    // 配置LED引脚为输出模式
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = LED1_PIN | LED2_PIN | LED3_PIN | LED4_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP; // 推挽输出
    GPIO_InitStruct.Pull = GPIO_NOPULL; // 无上下拉
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW; // 低速
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
    // 初始化LED状态为关闭
    HAL_GPIO_WritePin(GPIOB, LED1_PIN | LED2_PIN | LED3_PIN | LED4_PIN, GPIO_PIN_SET); // 这里LED是低电平点亮
}

void LED_Mode_Control(LEDMode_State_t mode)
{
    switch (mode)
    {
        case Flowing_lights:
            // 实现流水灯效果
            for (int i = 0; i < 4; i++)
            {
                HAL_GPIO_WritePin(GPIOB, LED1_PIN << i, GPIO_PIN_RESET); // 点亮当前LED
                vTaskDelay(pdMS_TO_TICKS(200)); // 延时200ms
                HAL_GPIO_WritePin(GPIOB, LED1_PIN << i, GPIO_PIN_SET); // 熄灭当前LED
            }
            break;
        case Twinkleing:
            // 实现闪烁效果
            for (int i = 0; i < 4; i++)
            {
                HAL_GPIO_WritePin(GPIOB, LED1_PIN << i, GPIO_PIN_RESET); // 点亮当前LED
            }
            vTaskDelay(pdMS_TO_TICKS(500)); // 延时500ms
            for (int i = 0; i < 4; i++)
            {
                HAL_GPIO_WritePin(GPIOB, LED1_PIN << i, GPIO_PIN_SET); // 熄灭当前LED
            }
            vTaskDelay(pdMS_TO_TICKS(500)); // 延时500ms
            break;
        case All_On:
            // 全部点亮
            HAL_GPIO_WritePin(GPIOB, LED1_PIN | LED2_PIN | LED3_PIN | LED4_PIN, GPIO_PIN_RESET);
            break;
        default:
            break;
    }
}

void Single_LED_Control(uint8_t led_num, uint8_t state)
{
    if (led_num < 1 || led_num > 4) return; // 确保LED编号在1-4范围内
    HAL_GPIO_WritePin(GPIOB, LED1_PIN << (led_num - 1), (state) ? GPIO_PIN_RESET : GPIO_PIN_SET); // 这里LED是低电平点亮
}

