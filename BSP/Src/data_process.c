#include "data_process.h"
#include "freertos_taskstart.h"

SensorData_t sensorData; // 定义一个全局变量，用于存储处理后的传感器数据

/**
 * @brief 将ADC值转换为温度值
 * @param adc_value ADC采集到的原始值（0-4095）
 * @return 转换后的温度值（摄氏度）
 *
 * @details 使用Steinhart-Hart方程将NTC热敏电阻的ADC值转换为温度值。
 *          首先将ADC值转换为电压，计算电阻值，然后使用Steinhart-Hart方程
 *          计算温度，最后将开尔文温度转换为摄氏度。
 */
// static float adc_to_temperature(uint16_t adc_value)
// {
//     // 实现ADC值到温度的转换逻辑
//     float voltage = 0;	
// 	voltage  = adc_value / 4095.0f * 3.3f;

//     float resistance = (3.3 - voltage) * 10000 / voltage;
//     float steinhart = resistance / 10000.0;
//     steinhart = log(steinhart);
//     steinhart = 1.0 / (0.001129148 + 0.000234125 * steinhart +
//                       0.0000000876741 * pow(steinhart, 3));
//     float temperature = steinhart - 273.15; // 转换为摄氏度
//     return temperature;
// }
static float adc_to_temperature(uint16_t adc_value)
{
    float voltage = (float)adc_value / 4095.0f * 3.3f;  // ADC值转电压
    float resistance = voltage * 10000.0f / (3.3f - voltage);  // 电压转热敏电阻阻值
    float steinhart = log(resistance);  // 取对数
    steinhart = 1.0f / (0.001129148f + 0.000234125f * steinhart + 
                         0.0000000876741f * pow(steinhart, 3));  // Steinhart-Hart方程
    return steinhart - 273.15f;  // 转为摄氏度
}

/**
 * @brief 将ADC值转换为距离值
 * @param adc_value ADC采集到的原始值（0-4095）
 * @return 转换后的距离值（厘米，范围10-80cm）
 *
 * @details 将反射式红外传感器（Sharp GP2Y0A21）的ADC值转换为距离值。
 *          首先将ADC值转换为电压，然后应用经验公式计算距离。
 *          结果限制在传感器的有效测量范围内（10-80厘米）。
 */
// static float adc_to_distance(uint16_t adc_value)
// {
//     // 实现ADC值到距离的转换逻辑
//     const float adc_ref_voltage = 3.3f;
//     const uint32_t adc_max_value = 4095; // 2^12 - 1
//     float voltage = (adc_value * adc_ref_voltage) / adc_max_value;

//     // 应用Sharp GP2Y0A21的经验公式
//     float distance = (1.0 / voltage) - 0.42;

//     // 限制结果在传感器的有效范围内
//     if (distance < 10) distance = 10;
//     if (distance > 80) distance = 80;

//     return distance;
// }
static float adc_to_distance(uint16_t adc_value)
{
    // 1. ADC参数配置：根据你的实际ADC引脚配置填写以下两行
    const float adc_ref_voltage = 3.3f;   // 确认你的ADC参考电压是3.3V还是5.0V
    const uint32_t adc_max_value = 4095;  // 确认你的ADC是12位(4095)还是10位(1023)

    // 2. 计算传感器输出电压
    float voltage = adc_value * adc_ref_voltage / adc_max_value;

    // 3. 有效电压钳位 (避免除零和反向推算错误)
    if (voltage < 0.3f) voltage = 0.3f;
    if (voltage > 3.0f) voltage = 3.0f;

    // 4. 距离计算：采用官方推荐的反比公式，使用正确的传感器常数
    //    GP2Y0A21 的正确定标公式是 distance = 27 / voltage
    float distance = 27.0f / voltage;

    // 5. 结果限制在传感器有效范围内 (10-80 cm)
    if (distance < 10.0f) distance = 10.0f;
    if (distance > 80.0f) distance = 80.0f;

    return distance;
}

/**
 * @brief 将ADC值转换为光照强度值
 *
 * 根据光敏电阻的分压电路特性，将ADC采样值转换为实际的光照强度值（单位：lux）
 * 使用公式：illuminance = 40000 * R^(-0.6021)，其中R为光敏电阻阻值
 *
 * @param adc_value ADC采样值（12位分辨率，范围0-4095）
 * @return float 光照强度值（范围0-999 lux）
 */
static float adc_to_illuminance(uint16_t adc_value)
{
    // 实现ADC值到光强的转换逻辑
    float voltage = 0;	
	float R = 0;	
	uint16_t illuminance = 0;
	voltage  = adc_value / 4095.0f * 3.3f;
	
	R = voltage / (3.3f - voltage) * 10;
		
	illuminance =  242 / pow(R, 1.28);
	
	if (illuminance > 9999)
	{
		illuminance = 9999;
	}

    return illuminance;
}

/**
 * @brief 数据处理函数
 * @details 对采集到的传感数据进行计算平均值、转换为物理量等处理
 *          包括反射式红外传感器、热敏电阻传感器以及光敏电阻传感器的ADC数据
 */
void Data_Process(void)
{
    SensorData_t *pxPointerTosensorData = &sensorData; // 定义一个指向传感器数据结构的指针

    uint32_t sum = 0;            // ADC数据的累加和

    // 在这里实现传感数据处理的逻辑
    // 例如：对采集到的反射式红外传感器、热敏电阻传感器以及光敏电阻传感器的ADC数据进行计算平均值、转换为物理量等
    if(xSemaphoreTake(xSemaphore, portMAX_DELAY) == pdTRUE) {
        // 处理ADC DMA传输完成的数据
        // 例如：对adc_dma_buffer中的数据进行计算平均值、转换为物理量等
        // 处理完成后，重置数据就绪标志
        for(int i = 0; i < 3; i++) {
            // 循环处理三个ADC通道数据，计算平均值、转换为物理量
            sum = 0;
            for(int j = 0; j < ADC_DMA_BUFFER_SIZE / 3; j++) {
                sum += adc_dma_buffer[i + j*3]; // 累加当前通道的数据
            }
            // 将平均值转换为物理量
            if(i == 0) {
                // 处理光敏电阻传感器数据，转换为光强等物理量
                pxPointerTosensorData->avg_adc_illuminance = sum / (ADC_DMA_BUFFER_SIZE / 3);
                pxPointerTosensorData->illuminance = adc_to_illuminance(pxPointerTosensorData->avg_adc_illuminance);               
            } else if(i == 1) {
                // 处理反射式红外传感器数据，转换为距离等物理量
                pxPointerTosensorData->avg_adc_distance = sum / (ADC_DMA_BUFFER_SIZE / 3);
                pxPointerTosensorData->distance = adc_to_distance(pxPointerTosensorData->avg_adc_distance);
            } else if(i == 2) {
                // 处理热敏电阻传感器数据，转换为温度等物理量
                pxPointerTosensorData->avg_adc_temperature = sum / (ADC_DMA_BUFFER_SIZE / 3);
                pxPointerTosensorData->temperature = adc_to_temperature(pxPointerTosensorData->avg_adc_temperature);
            }
        }

        xQueueSend(sensorDataQueue, &pxPointerTosensorData, 0);

    }
}

