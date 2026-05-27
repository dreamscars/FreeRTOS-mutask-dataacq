#include "menu_system.h"
#include "oled.h"
#include "led.h"
#include "key.h"
#include "data_process.h"
#include "FreeRTOS.h"
#include "task.h"

static MenuState_t g_menuState = MENU_MAIN;
static uint8_t g_mainIndex = 0;
static uint8_t g_subIndex = 0;
static uint8_t g_ledStates[4] = {0, 0, 0, 0};
static uint8_t g_ledModeActive = 0;
static uint8_t g_ledModeStep = 0;
static TickType_t g_lastLEDTick = 0;

extern SensorData_t sensorData;

static const char *mainItems[3] = {
    "LED Mode Select",
    "Single LED Control",
    "Mule Data Show"
};

static const char *ledModeItems[3] = {
    "Flowing lights",
    "Twinkleing",
    "All On"
};

static void OLED_PutStr(uint8_t x, uint8_t y, const char *str, uint8_t size, uint8_t mode)
{
    while (*str) {
        OLED_ShowChar(x, y, *str, size, mode);
        if (size == 8) x += 6;
        else x += size / 2;
        str++;
    }
}

static void Menu_DrawMain(void)
{
    OLED_Clear();
    OLED_PutStr(28, 0, "= MENU =", 8, 1);
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t lineY = 16 + i * 16;
        if (i == g_mainIndex) {
            OLED_PutStr(0, lineY, ">", 8, 1);
            OLED_PutStr(12, lineY, mainItems[i], 8, 1);
        } else {
            OLED_PutStr(12, lineY, mainItems[i], 8, 1);
        }
    }
    OLED_Refresh();
}

static void Menu_DrawLEDMode(void)
{
    OLED_Clear();
    OLED_PutStr(16, 0, "LED Mode Select", 8, 1);
    for (uint8_t i = 0; i < 3; i++) {
        uint8_t lineY = 16 + i * 16;
        if (i == g_subIndex) {
            OLED_PutStr(0, lineY, ">", 8, 1);
            OLED_PutStr(12, lineY, ledModeItems[i], 8, 1);
        } else {
            OLED_PutStr(12, lineY, ledModeItems[i], 8, 1);
        }
    }
    OLED_Refresh();
}

static void Menu_DrawSingleLED(void)
{
    OLED_Clear();
    OLED_PutStr(0, 0, "Single LED Ctrl", 8, 1);
    for (uint8_t i = 0; i < 4; i++) {
        uint8_t lineY = 16 + i * 12;
        if (i == g_subIndex) {
            OLED_PutStr(0, lineY, ">", 8, 1);
        } else {
            OLED_PutStr(0, lineY, " ", 8, 1);
        }
        OLED_PutStr(12, lineY, "LED", 8, 1);
        OLED_ShowNum(36, lineY, i + 1, 1, 8, 1);
        OLED_PutStr(54, lineY, g_ledStates[i] ? "ON " : "OFF", 8, 1);
    }
    OLED_Refresh();
}

static void Menu_DrawLEDModeRunning(void)
{
    OLED_Clear();
    OLED_PutStr(8, 16, "Mode Active:", 8, 1);
    OLED_PutStr(8, 28, ledModeItems[g_ledModeActive - 1], 8, 1);
    OLED_PutStr(8, 48, "KEY4: Back", 8, 1);
    OLED_Refresh();
}

static void Menu_DrawDataShow(void)
{
    OLED_Clear();
    OLED_PutStr(0, 0, "AdcDist:", 8, 1);
    OLED_ShowNum(60, 0, sensorData.avg_adc_distance, 4, 8, 1);
    OLED_PutStr(0, 8, "Dist:", 8, 1);
    OLED_ShowfloatNum(60, 8, sensorData.distance, 8, 1);
    OLED_PutStr(0, 16, "AdcTemp:", 8, 1);
    OLED_ShowNum(60, 16, sensorData.avg_adc_temperature, 4, 8, 1);
    OLED_PutStr(0, 24, "Temp:", 8, 1);
    OLED_ShowfloatNum(60, 24, sensorData.temperature, 8, 1);
    OLED_PutStr(0, 32, "AdcIllu:", 8, 1);
    OLED_ShowNum(60, 32, sensorData.avg_adc_illuminance, 4, 8, 1);
    OLED_PutStr(0, 40, "Illu:", 8, 1);
    OLED_ShowfloatNum(60, 40, sensorData.illuminance, 8, 1);
    OLED_PutStr(8, 54, "KEY4: Back", 8, 1);
    OLED_Refresh();
}

void Menu_Init(void)
{
    g_menuState = MENU_MAIN;
    g_mainIndex = 0;
    g_subIndex = 0;
    g_ledModeActive = 0;
    for (uint8_t i = 0; i < 4; i++) {
        g_ledStates[i] = 0;
        Single_LED_Control(i + 1, 0);
    }
    Menu_DrawMain();
}

void Menu_LEDModeStep(void)
{
    if (g_menuState != MENU_LED_MODE_RUNNING || g_ledModeActive == 0) return;

    TickType_t now = xTaskGetTickCount();

    switch (g_ledModeActive) {
        case 1: {
            if (now - g_lastLEDTick >= pdMS_TO_TICKS(200)) {
                for (uint8_t i = 1; i <= 4; i++) Single_LED_Control(i, 0);
                Single_LED_Control(g_ledModeStep + 1, 1);
                g_ledModeStep = (g_ledModeStep + 1) % 4;
                g_lastLEDTick = now;
            }
            break;
        }
        case 2: {
            if (now - g_lastLEDTick >= pdMS_TO_TICKS(500)) {
                uint8_t on = (g_ledModeStep % 2) ? 1 : 0;
                for (uint8_t i = 1; i <= 4; i++) Single_LED_Control(i, on);
                g_ledModeStep++;
                g_lastLEDTick = now;
            }
            break;
        }
        case 3: {
            for (uint8_t i = 1; i <= 4; i++) Single_LED_Control(i, 1);
            break;
        }
    }
}

void Menu_HandleKey(uint8_t key)
{
    switch (g_menuState) {
        case MENU_MAIN:
            if (key == KEY2_PRESSED) {
                g_mainIndex = (g_mainIndex > 0) ? g_mainIndex - 1 : 2;
                Menu_DrawMain();
            } else if (key == KEY3_PRESSED) {
                g_mainIndex = (g_mainIndex < 2) ? g_mainIndex + 1 : 0;
                Menu_DrawMain();
            } else if (key == KEY1_PRESSED) {
                switch (g_mainIndex) {
                    case 0:
                        g_menuState = MENU_LED_MODE_SELECT;
                        g_subIndex = 0;
                        Menu_DrawLEDMode();
                        break;
                    case 1:
                        g_menuState = MENU_SINGLE_LED_CTRL;
                        g_subIndex = 0;
                        Menu_DrawSingleLED();
                        break;
                    case 2:
                        g_menuState = MENU_DATA_SHOW;
                        break;
                }
            }
            break;

        case MENU_LED_MODE_SELECT:
            if (key == KEY2_PRESSED) {
                g_subIndex = (g_subIndex > 0) ? g_subIndex - 1 : 2;
                Menu_DrawLEDMode();
            } else if (key == KEY3_PRESSED) {
                g_subIndex = (g_subIndex < 2) ? g_subIndex + 1 : 0;
                Menu_DrawLEDMode();
            } else if (key == KEY1_PRESSED) {
                g_ledModeActive = g_subIndex + 1;
                g_ledModeStep = 0;
                g_lastLEDTick = xTaskGetTickCount();
                g_menuState = MENU_LED_MODE_RUNNING;
                Menu_DrawLEDModeRunning();
            } else if (key == KEY4_PRESSED) {
                g_menuState = MENU_MAIN;
                g_mainIndex = 0;
                Menu_DrawMain();
            }
            break;

        case MENU_SINGLE_LED_CTRL:
            if (key == KEY2_PRESSED) {
                g_subIndex = (g_subIndex > 0) ? g_subIndex - 1 : 3;
                Menu_DrawSingleLED();
            } else if (key == KEY3_PRESSED) {
                g_subIndex = (g_subIndex < 3) ? g_subIndex + 1 : 0;
                Menu_DrawSingleLED();
            } else if (key == KEY1_PRESSED) {
                g_ledStates[g_subIndex] = !g_ledStates[g_subIndex];
                Single_LED_Control(g_subIndex + 1, g_ledStates[g_subIndex]);
                Menu_DrawSingleLED();
            } else if (key == KEY4_PRESSED) {
                g_menuState = MENU_MAIN;
                g_mainIndex = 1;
                Menu_DrawMain();
            }
            break;

        case MENU_DATA_SHOW:
            if (key == KEY4_PRESSED) {
                g_menuState = MENU_MAIN;
                g_mainIndex = 0;
                Menu_DrawMain();
            }
            break;

        case MENU_LED_MODE_RUNNING:
            if (key == KEY4_PRESSED) {
                for (uint8_t i = 1; i <= 4; i++) Single_LED_Control(i, 0);
                g_ledModeActive = 0;
                g_menuState = MENU_LED_MODE_SELECT;
                Menu_DrawLEDMode();
            }
            break;
    }
}

uint8_t Menu_IsDataShowActive(void)
{
    return (g_menuState == MENU_DATA_SHOW);
}
