#ifndef SEMPHR_MOCK_H
#define SEMPHR_MOCK_H

#include "FreeRTOS.h"
#include "queue.h"

/* Semaphore handle is just a queue handle */
typedef QueueHandle_t SemaphoreHandle_t;

/* Binary semaphore macros */
#define xSemaphoreCreateBinary()    ((SemaphoreHandle_t)0x1)

/* xSemaphoreTake is a macro -> call our mock backend */
#define xSemaphoreTake(sem, delay)  xSemaphoreTakeMock(sem, delay)

/* xSemaphoreGive is a macro */
#define xSemaphoreGive(sem)         xSemaphoreGiveMock(sem)

/* xSemaphoreGiveFromISR is a macro */
#define xSemaphoreGiveFromISR(sem, pxHigherPriorityTaskWoken) \
    xSemaphoreGiveMock(sem)

/* Mock backend functions - defined in test file */
BaseType_t xSemaphoreTakeMock(SemaphoreHandle_t xSemaphore, TickType_t xBlockTime);
BaseType_t xSemaphoreGiveMock(SemaphoreHandle_t xSemaphore);

#endif /* SEMPHR_MOCK_H */
