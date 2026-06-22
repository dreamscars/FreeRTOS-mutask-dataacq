/**
 * @file test_data_process.c
 * @brief Unit tests for data_process module using Unity framework.
 *
 * This file directly #include's data_process.c to gain access to
 * its static helper functions (adc_to_distance, adc_to_temperature,
 * adc_to_illuminance) for white-box testing of Data_Process().
 *
 * FreeRTOS and STM32 HAL APIs are mocked via test/mocks/ headers.
 */

#include "unity.h"
#include "data_process.h"
#include "adc1.h"
#include <string.h>

/* ======================================================================
 * Mock external variables (normally defined in freertos_taskstart.c / adc1.c)
 * ====================================================================== */
QueueHandle_t sensorDataQueue;     /* defined in freertos_taskstart.c */
SemaphoreHandle_t xSemaphore;      /* defined in freertos_taskstart.c */
uint16_t adc_dma_buffer[ADC_DMA_BUFFER_SIZE];  /* defined in adc1.c */

/* ======================================================================
 * Mock control state — lets tests control FreeRTOS mock behaviour
 * ====================================================================== */
static BaseType_t mock_semaphore_take_return = pdTRUE;
static int        mock_semaphore_take_call_count = 0;
static int        mock_queue_send_call_count = 0;
static void      *mock_last_sent_ptr = NULL;

void reset_mocks(void)
{
    mock_semaphore_take_return = pdTRUE;
    mock_semaphore_take_call_count = 0;
    mock_queue_send_call_count = 0;
    mock_last_sent_ptr = NULL;
    memset(adc_dma_buffer, 0, sizeof(adc_dma_buffer));
}

/* ======================================================================
 * Mock FreeRTOS implementations (called via macros in mock headers)
 * ====================================================================== */
BaseType_t xSemaphoreTakeMock(SemaphoreHandle_t xSemaphore, TickType_t xBlockTime)
{
    (void)xSemaphore;
    (void)xBlockTime;
    mock_semaphore_take_call_count++;
    return mock_semaphore_take_return;
}

BaseType_t xSemaphoreGiveMock(SemaphoreHandle_t xSemaphore)
{
    (void)xSemaphore;
    return pdTRUE;
}

BaseType_t xQueueGenericSend(QueueHandle_t xQueue, const void *pvItemToQueue,
                             TickType_t xTicksToWait, BaseType_t xCopyPosition)
{
    (void)xQueue;
    (void)xTicksToWait;
    (void)xCopyPosition;
    mock_queue_send_call_count++;
    /* The queue transports a SensorData_t* — capture the pointer value */
    mock_last_sent_ptr = *(void **)pvItemToQueue;
    return pdTRUE;
}

BaseType_t xQueueGenericReceive(QueueHandle_t xQueue, void *pvBuffer,
                                TickType_t xTicksToWait, BaseType_t xJustPeeking)
{
    (void)xQueue;
    (void)pvBuffer;
    (void)xTicksToWait;
    (void)xJustPeeking;
    return pdTRUE;
}

/* Dummy QueueHandle_t — just needs to be non-NULL */
QueueHandle_t xQueueCreate(UBaseType_t uxQueueLength, UBaseType_t uxItemSize)
{
    (void)uxQueueLength;
    (void)uxItemSize;
    return (QueueHandle_t)0xABCD;
}

/* ======================================================================
 * Module-under-test: include the actual source so static helpers are visible
 * ====================================================================== */
#include "../BSP/Src/data_process.c"

/* ======================================================================
 * Helper: fill one ADC channel with a constant value across all 50 samples
 * ====================================================================== */
static void fill_adc_channel(int channel_index, uint16_t value)
{
    /* Channel layout: ch0 at offsets 0,3,6...; ch1 at 1,4,7...; ch2 at 2,5,8... */
    for (int j = 0; j < ADC_DMA_BUFFER_SIZE / 3; j++) {
        adc_dma_buffer[channel_index + j * 3] = value;
    }
}

/* ======================================================================
 * Setup / Teardown
 * ====================================================================== */
void setUp(void)
{
    reset_mocks();
}

void tearDown(void)
{
}

/* ======================================================================
 * Test cases — Data_Process()
 * ====================================================================== */

/**
 * @brief Data_Process successfully takes semaphore, averages data,
 *        converts to physical units, and sends to queue.
 */
void test_Data_Process_Nominal(void)
{
    /* Fill each channel with a known constant */
    fill_adc_channel(0, 2048);  /* illuminance — mid-scale */
    fill_adc_channel(1, 2048);  /* distance    — mid-scale */
    fill_adc_channel(2, 2048);  /* temperature — mid-scale */

    /* Reset sensorData global */
    memset(&sensorData, 0, sizeof(sensorData));

    Data_Process();

    TEST_ASSERT_EQUAL_INT(1, mock_semaphore_take_call_count);
    TEST_ASSERT_EQUAL_INT(1, mock_queue_send_call_count);
    TEST_ASSERT_NOT_NULL(mock_last_sent_ptr);
    TEST_ASSERT_EQUAL_PTR(&sensorData, mock_last_sent_ptr);

    /* Verify averages */
    TEST_ASSERT_EQUAL_UINT16(2048, sensorData.avg_adc_illuminance);
    TEST_ASSERT_EQUAL_UINT16(2048, sensorData.avg_adc_distance);
    TEST_ASSERT_EQUAL_UINT16(2048, sensorData.avg_adc_temperature);

    /* Physical values should be non-zero (conversion produced something) */
    /* ADC=2048 → 1.65V → LDR R=10kΩ → illuminance ≈ 242/10^1.28 ≈ 12.7 lux */
    TEST_ASSERT_FLOAT_WITHIN(5.0f, sensorData.illuminance, 12.7f);
    /* ADC=2048 → 1.65V → distance = 27/1.65 ≈ 16.4 cm */
    TEST_ASSERT_FLOAT_WITHIN(1.0f, sensorData.distance, 16.36f);
    /* ADC=2048 → temperature should be a reasonable value */
    TEST_ASSERT_TRUE(sensorData.temperature > -50.0f);
    TEST_ASSERT_TRUE(sensorData.temperature < 150.0f);
}

/**
 * @brief Data_Process clamps distances correctly for high voltage (close object).
 *        V = 3.0V → distance = 27/3.0 = 9.0cm → clamped to 10cm.
 *        ADC value for 3.0V ≈ 4095 * 3.0/3.3 ≈ 3723
 */
void test_Data_Process_Distance_Clamp_Low(void)
{
    fill_adc_channel(0, 0);
    fill_adc_channel(1, 3723);  /* ~3.0V → 9cm → clamped to 10cm */
    fill_adc_channel(2, 0);

    memset(&sensorData, 0, sizeof(sensorData));
    Data_Process();

    TEST_ASSERT_FLOAT_WITHIN(0.1f, 10.0f, sensorData.distance);
}

/**
 * @brief Data_Process clamps distances for low voltage (far object).
 *        V = 0.3V → distance = 27/0.3 = 90cm → clamped to 80cm.
 *        ADC value for 0.3V ≈ 4095 * 0.3/3.3 ≈ 372
 */
void test_Data_Process_Distance_Clamp_High(void)
{
    fill_adc_channel(0, 0);
    fill_adc_channel(1, 372);   /* ~0.3V → 90cm → clamped to 80cm */
    fill_adc_channel(2, 0);

    memset(&sensorData, 0, sizeof(sensorData));
    Data_Process();

    TEST_ASSERT_FLOAT_WITHIN(0.1f, 80.0f, sensorData.distance);
}

/**
 * @brief Data_Process clamps illuminance at 9999 lux for very bright light.
 *        Very low LDR resistance → low voltage → very high illuminance.
 *        ADC ≈ 23 gives V ≈ 0.018V → R ≈ 57Ω → illuminance > 9999 → clamped.
 */
void test_Data_Process_Illuminance_Clamp(void)
{
    fill_adc_channel(0, 20);   /* ~0.016V → very bright → clamped to 9999 */
    fill_adc_channel(1, 0);
    fill_adc_channel(2, 0);

    memset(&sensorData, 0, sizeof(sensorData));
    Data_Process();

    TEST_ASSERT_FLOAT_WITHIN(0.1f, 9999.0f, sensorData.illuminance);
}

/**
 * @brief Data_Process with semaphore not available — should skip and NOT send.
 */
void test_Data_Process_Semaphore_TimedOut(void)
{
    mock_semaphore_take_return = pdFALSE;

    fill_adc_channel(0, 3000);
    fill_adc_channel(1, 1500);
    fill_adc_channel(2, 1000);

    memset(&sensorData, 0, sizeof(sensorData));
    Data_Process();

    TEST_ASSERT_EQUAL_INT(1, mock_semaphore_take_call_count);
    TEST_ASSERT_EQUAL_INT(0, mock_queue_send_call_count);
    TEST_ASSERT_EQUAL_UINT16(0, sensorData.avg_adc_illuminance);
    TEST_ASSERT_EQUAL_UINT16(0, sensorData.avg_adc_distance);
    TEST_ASSERT_EQUAL_UINT16(0, sensorData.avg_adc_temperature);
}

/**
 * @brief Data_Process with zero ADC values — tests edge case.
 *        Zero voltage for distance: voltage clamped to 0.3V → 27/0.3=90cm→80cm
 */
void test_Data_Process_Zero_ADC(void)
{
    fill_adc_channel(0, 0);
    fill_adc_channel(1, 0);
    fill_adc_channel(2, 0);

    memset(&sensorData, 0, sizeof(sensorData));
    Data_Process();

    TEST_ASSERT_EQUAL_INT(1, mock_semaphore_take_call_count);
    TEST_ASSERT_EQUAL_INT(1, mock_queue_send_call_count);

    /* All averages should be zero */
    TEST_ASSERT_EQUAL_UINT16(0, sensorData.avg_adc_illuminance);
    TEST_ASSERT_EQUAL_UINT16(0, sensorData.avg_adc_distance);
    TEST_ASSERT_EQUAL_UINT16(0, sensorData.avg_adc_temperature);

    /* Distance at 0V → voltage clamped to 0.3V → 27/0.3=90 → clamped to 80 */
    TEST_ASSERT_FLOAT_WITHIN(0.1f, 80.0f, sensorData.distance);
    /* Temperature at 0V → 0 resistance → log(0) = -inf → large result */
    /* Just verify it's a finite value */
    TEST_ASSERT_TRUE(!isnan(sensorData.temperature));
    TEST_ASSERT_TRUE(isfinite(sensorData.temperature));
}

/* ======================================================================
 * Test cases — static conversion helpers (indirectly via known ADC values)
 * ====================================================================== */

/**
 * @brief Known-voltage distance test: 1.65V (ADC=2048) should give ~16.4cm.
 *        Formula: V = 1.65, distance = 27/1.65 ≈ 16.36cm
 */
void test_Data_Process_Known_Distance_At_2048(void)
{
    fill_adc_channel(0, 0);
    fill_adc_channel(1, 2048);  /* ~1.65V */
    fill_adc_channel(2, 0);

    memset(&sensorData, 0, sizeof(sensorData));
    Data_Process();

    TEST_ASSERT_FLOAT_WITHIN(1.0f, 16.36f, sensorData.distance);
}

/**ADC=512 → ~0.41V.
 *        R = 0.413/(3.3-0.413)*10 ≈ 1.43kΩ
 *        illuminance = 242/1.43^1.28 ≈ 155 lux
 */
void test_Data_Process_Known_Illuminance_At_512(void)
{
    fill_adc_channel(0, 512);   /* ~0.41V */
    fill_adc_channel(1, 0);
    fill_adc_channel(2, 0);

    memset(&sensorData, 0, sizeof(sensorData));
    Data_Process();

    TEST_ASSERT_FLOAT_WITHIN(20.0f, 155.0f, sensorData.illuminance);
}

/* ======================================================================
 * Run all tests — called from test_runner.c
 * ====================================================================== */
void run_data_process_tests(void)
{
    RUN_TEST(test_Data_Process_Nominal);
    RUN_TEST(test_Data_Process_Distance_Clamp_Low);
    RUN_TEST(test_Data_Process_Distance_Clamp_High);
    RUN_TEST(test_Data_Process_Illuminance_Clamp);
    RUN_TEST(test_Data_Process_Semaphore_TimedOut);
    RUN_TEST(test_Data_Process_Zero_ADC);
    RUN_TEST(test_Data_Process_Known_Distance_At_2048);
    RUN_TEST(test_Data_Process_Known_Illuminance_At_512);
}
