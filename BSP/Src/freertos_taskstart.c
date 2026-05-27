#include "freertos_taskstart.h"
#include "menu_system.h"

TaskHandle_t dataProcessTaskHandle;

TaskHandle_t oledShowTaskHandle;

TaskHandle_t keyScanTaskHandle;

TaskHandle_t StartledTaskHandle;

QueueHandle_t sensorDataQueue; // 定义一个队列句柄，用于传递传感器数据
SemaphoreHandle_t xSemaphore;

void Data_Process_Task(void *pvParameters)
{
    (void) pvParameters; // 防止编译器警告

    while (1)
    {
        Data_Process(); // 调用数据处理函数
    }
}

void OLED_Show_Task(void *pvParameters)
{
    (void) pvParameters; // 防止编译器警告
    SensorData_t *sensorDatapoint; // 定义一个指针，用于接收队列中的传感器数据

    while (1)
    {
        xQueueReceive(sensorDataQueue, &sensorDatapoint, portMAX_DELAY);
        if (Menu_IsDataShowActive()) {
            OLED_Show(sensorDatapoint);
        }
    }
}

void Key_Scan_Task(void *pvParameters)
{
    (void) pvParameters;

    while (1)
    {
        KeyState_t keyState = KEY_Scan();
        if (keyState != KEY_NONE) {
            Menu_HandleKey((uint8_t)keyState);
        }
        Menu_LEDModeStep();
        vTaskDelay(pdMS_TO_TICKS(20));
    }
}

void Startled_Task(void *pvParameters)
{
    (void)pvParameters;

    xTaskCreate((TaskFunction_t)Data_Process_Task,
                (char *)"data_process_task",
                (configSTACK_DEPTH_TYPE)DATA_PROCESS_TASK_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t)DATA_PROCESS_TASK_PRIORITY,
                (TaskHandle_t *)&dataProcessTaskHandle);

    xTaskCreate((TaskFunction_t)OLED_Show_Task,
                (char *)"oled_show_task",
                (configSTACK_DEPTH_TYPE)OLED_SHOW_TASK_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t)OLED_SHOW_TASK_PRIORITY,
                (TaskHandle_t *)&oledShowTaskHandle);

    xTaskCreate((TaskFunction_t)Key_Scan_Task,
                (char *)"key_scan_task",
                (configSTACK_DEPTH_TYPE)KEY_SCAN_TASK_STACK_SIZE,
                (void *)NULL,
                (UBaseType_t)KEY_SCAN_TASK_PRIORITY,
                (TaskHandle_t *)&keyScanTaskHandle);

    Menu_Init();

    vTaskDelete(NULL);
}

void freertos_start(void)
{
    sensorDataQueue = xQueueCreate(1, sizeof(SensorData_t*)); // 创建一个队列，用于传递传感器数据
    xSemaphore = xSemaphoreCreateBinary(); // 创建一个二值信号量，用于同步ADC数据处理
    
    xTaskCreate((TaskFunction_t)Startled_Task, 
                (char *)"startled_task", 
                (configSTACK_DEPTH_TYPE)STARTLED_TASK_SIZE, 
                (void *)NULL, 
                (UBaseType_t)STARTLED_TASK_PRIORITY, 
                (TaskHandle_t *)&StartledTaskHandle);
    
    vTaskStartScheduler();
}
