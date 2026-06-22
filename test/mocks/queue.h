#ifndef QUEUE_MOCK_H
#define QUEUE_MOCK_H

#include "FreeRTOS.h"
#include <stdint.h>

/* Queue handle type */
typedef void * QueueHandle_t;

/* Queue functions - will be implemented in test file as mocks */
QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize);

/* xQueueSend is actually a macro that calls xQueueGenericSend */
BaseType_t xQueueGenericSend(QueueHandle_t xQueue, const void *pvItemToQueue, TickType_t xTicksToWait, BaseType_t xCopyPosition);

#define xQueueSend(queue, item, delay)  xQueueGenericSend(queue, item, delay, 0)
#define xQueueReceive(queue, buf, delay) xQueueGenericReceive(queue, buf, delay, 0)

BaseType_t xQueueGenericReceive(QueueHandle_t xQueue, void *pvBuffer, TickType_t xTicksToWait, BaseType_t xJustPeeking);

#endif /* QUEUE_MOCK_H */
