#ifndef TASK_MOCK_H
#define TASK_MOCK_H

#include "FreeRTOS.h"

/* Task handle type */
typedef void * TaskHandle_t;

/* Task creation macro - no-op for test */
#define xTaskCreate(pvTaskCode, pcName, usStackDepth, pvParameters, uxPriority, pxCreatedTask) \
    ((BaseType_t)pdPASS)

#endif /* TASK_MOCK_H */
