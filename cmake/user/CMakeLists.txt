set(USER_Include_Dirs
    ${CMAKE_CURRENT_SOURCE_DIR}/../../FreeRTOS/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../../FreeRTOS/portable/GCC/ARM_CM3
    ${CMAKE_CURRENT_SOURCE_DIR}/../../BSP/Inc
)

set(USER_Application_Src
    ${CMAKE_CURRENT_SOURCE_DIR}/../../FreeRTOS/portable/MemMang/heap_4.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../FreeRTOS/Source/tasks.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../FreeRTOS/Source/queue.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../FreeRTOS/Source/event_groups.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../FreeRTOS/Source/timers.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../FreeRTOS/Source/list.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../FreeRTOS/Source/croutine.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../FreeRTOS/Source/stream_buffer.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../FreeRTOS/portable/GCC/ARM_CM3/port.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../BSP/Src/data_process.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../BSP/Src/adc1.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../BSP/Src/sys.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../BSP/Src/oled.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../BSP/Src/freertos_taskstart.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../BSP/Src/key.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../BSP/Src/led.c
    ${CMAKE_CURRENT_SOURCE_DIR}/../../BSP/Src/menu_system.c
)

target_include_directories(stm32cubemx INTERFACE ${USER_Include_Dirs})
target_sources(${CMAKE_PROJECT_NAME} PRIVATE ${USER_Application_Src})

