#ifndef __FREERTOS_TASKSTART_H
#define __FREERTOS_TASKSTART_H

#include "stm32f1xx_hal.h"
#include "data_process.h"
#include "oled.h"
#include "key.h"
#include "led.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define DATA_PROCESS_TASK_STACK_SIZE 256
#define DATA_PROCESS_TASK_PRIORITY 2

#define OLED_SHOW_TASK_STACK_SIZE 192
#define OLED_SHOW_TASK_PRIORITY 1

#define KEY_SCAN_TASK_STACK_SIZE 256
#define KEY_SCAN_TASK_PRIORITY 2

#define STARTLED_TASK_SIZE 128
#define STARTLED_TASK_PRIORITY 1

extern QueueHandle_t sensorDataQueue;
extern SemaphoreHandle_t xSemaphore;

void freertos_start(void);

#endif /* __FREERTOS_TASKSTART_H */
