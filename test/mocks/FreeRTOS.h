#ifndef FREERTOS_MOCK_H
#define FREERTOS_MOCK_H

#include <stdint.h>

/* Base types */
typedef signed long     BaseType_t;
typedef unsigned long   UBaseType_t;
typedef uint32_t        TickType_t;

/* Stack depth type */
typedef uint16_t configSTACK_DEPTH_TYPE;

/* Common constants */
#define pdTRUE          ((BaseType_t)1)
#define pdFALSE         ((BaseType_t)0)
#define pdPASS          (pdTRUE)
#define pdFAIL          (pdFALSE)
#define portMAX_DELAY   ((TickType_t)0xFFFFFFFFUL)
#define portTICK_PERIOD_MS ((TickType_t)1)
#define pdMS_TO_TICKS(x) ((TickType_t)(x))

/* Task function type */
typedef void (*TaskFunction_t)(void *);

/* Critical section macros (no-op for host testing) */
#define portDISABLE_INTERRUPTS()
#define portENABLE_INTERRUPTS()

/* For configCHECK_FOR_STACK_OVERFLOW - method 2 */
#define traceMOVED_TASK_TO_READY_STATE(pxTCB)

#endif /* FREERTOS_MOCK_H */
