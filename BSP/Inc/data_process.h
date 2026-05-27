#ifndef __DATA_PROCESS_H
#define __DATA_PROCESS_H

#include "stm32f1xx_hal.h"
#include "freertos_taskstart.h"
#include "adc1.h"
#include "math.h"

typedef struct SensorData{
    float distance;
    float temperature;
    float illuminance;
    uint16_t avg_adc_distance;
    uint16_t avg_adc_temperature;
    uint16_t avg_adc_illuminance;
} SensorData_t;

void Data_Process(void);


#endif /* __DATA_PROCESS_H */  