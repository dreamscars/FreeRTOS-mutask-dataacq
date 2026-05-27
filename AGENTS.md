# AGENTS.md — FreeRTOS mutask dataacq

## Platform

- MCU: STM32F103 (Cortex-M3), 72MHz, 6KB heap (`configTOTAL_HEAP_SIZE`)
- RTOS: FreeRTOS V202212.01, preemptive, 5 priority levels, tick 1000Hz
- Toolchain: ARM GCC (`cmake/gcc-arm-none-eabi.cmake`), build via CMake+Ninja
- Language: C11 (no C++, no STM32CubeMX HAL codegen beyond init)

## Build & index

```sh
cmake --preset Debug
cmake --build --preset Debug
```

`CMAKE_EXPORT_COMPILE_COMMANDS` is on; `.clangd` expects `build/Debug`.

Add new `.c` sources to `cmake/user/CMakeLists.txt` `USER_Application_Src` list.

## Project layout

| Path | Purpose |
|------|---------|
| `BSP/Inc/`, `BSP/Src/` | All application drivers + menu system |
| `Core/Inc/`, `Core/Src/` | HAL init, main(), interrupt handlers |
| `FreeRTOS/` | Kernel source (unmodified) |
| `cmake/user/CMakeLists.txt` | Register new source files here |
| `startup_stm32f103xb.s` | Vector table |

## Boot sequence (`main.c`)

1. `HAL_Init()` → `SystemClock_Config()` → `MX_GPIO_Init()`
2. `OLED_Init()` → `ADC_DMA_Init()` → `ADC_Start_DMA()` → `LED_Init()` → `KEY_Init()`
3. `freertos_start()` creates queue+semaphore, starts `Startled_Task`, calls `vTaskStartScheduler()`

`Startled_Task` spawns the three worker tasks then self-deletes.

## Tasks

| Task | Function | Stack | Prio | Role |
|------|----------|-------|------|------|
| `data_process_task` | `Data_Process_Task` | 256 | 2 | Waits on ADC DMA semaphore, averages buffer, sends `SensorData_t*` to queue |
| `key_scan_task` | `Key_Scan_Task` | 256 | 2 | Polls `KEY_Scan()`, calls `Menu_HandleKey()` + `Menu_LEDModeStep()`, 20ms delay loop |
| `oled_show_task` | `OLED_Show_Task` | 192 | 1 | Blocks on `sensorDataQueue`, calls `OLED_Show()` only when `Menu_IsDataShowActive()` |
| `startled_task` | `Startled_Task` | 128 | 1 | Creates the 3 tasks above, calls `Menu_Init()`, self-deletes |

Priority 2 tasks can preempt priority 1 tasks. `Key_Scan_Task` (prio 2) handles menu state transitions atomically from `OLED_Show_Task`'s view.

## Data flow

```
ADC DMA (circular, 150-entry buffer) → ISR gives xSemaphore
    → Data_Process_Task takes sem, averages per channel, converts to physical units
    → xQueueSend(sensorDataQueue, &sensorData)
    → OLED_Show_Task receives, calls OLED_Show() if in data-show mode
```

`SensorData_t` struct fields: `distance` / `temperature` / `illuminance` (float), `avg_adc_*` (uint16_t).

`xSemaphore` is a binary semaphore. `sensorDataQueue` is queue of `SensorData_t*`, size 1.

## Peripherals & drivers

### OLED (SSD1306 128×64, I2C bit-banged)
- SCL=PB11, SDA=PB10 (software I2C, not hardware I2C peripheral)
- `OLED_ShowChar()` draws to GRAM buffer **without** refresh
- `OLED_ShowString()` draws chars then calls `OLED_Refresh()` (full I2C send, ~104ms)
- `OLED_ShowNum()` / `OLED_ShowfloatNum()` draw chars then partial refresh
- `OLED_Show()` clears GRAM buffer inline, draws labels via `OLED_DrawStr()`, single `OLED_Refresh()` at end
- **Avoid calling `OLED_ShowString()` in performance-sensitive paths** — each call does a full 1024-byte I2C transfer
- `OLED_GRAM[128][8]` is `static` in `oled.c`

### LEDs (active-low)
- PB5..PB8, state=1=ON (drives pin low)
- Macros: `LED1(state)`, `LED1_TOGGLE()`, same for LED2..LED4
- `Single_LED_Control(num, state)` — num 1-4, state 0/1
- `LED_Mode_Control(mode)` — blocking with vTaskDelay, meant for dedicated task use

### Buttons (active-low, pull-up)
- KEY1=PB4, KEY2=PB15, KEY3=PA8, KEY4=PA15
- `KEY_Scan()` — polling, edge-detection + 10ms debounce delay, returns `KEY_NONE` or `KEY[1-4]_PRESSED`

### ADC (DMA circular, 12-bit)
- PA1 (distance), PA2 (temperature), PA4 (illuminance)
- `ADC_DMA_Init()` + `ADC_Start_DMA()` starts continuous conversion
- `adc_dma_buffer[150]` = 50 samples × 3 channels, order: ch1, ch2, ch4 repeating

## Menu system (`menu_system.h` / `menu_system.c`)

State machine with 5 states: `MENU_MAIN`, `MENU_LED_MODE_SELECT`, `MENU_SINGLE_LED_CTRL`, `MENU_DATA_SHOW`, `MENU_LED_MODE_RUNNING`.

### Key mapping

| Key | Function |
|-----|----------|
| KEY1 | Enter / Toggle LED |
| KEY2 | Up |
| KEY3 | Down |
| KEY4 | Back |

### Architecture rules (hard-earned)

1. **`OLED_Show()` clears its own buffer** — callers must NOT call `OLED_Clear()` before it
2. **Only one task at a time may write to `OLED_GRAM`** — `Key_Scan_Task` (prio 2) handles menu rendering, `OLED_Show_Task` (prio 1) handles data display; priority ensures mutual exclusion
3. **`Menu_DrawDataShow()` must NOT be called from `Menu_HandleKey()`** — the previous bug was concurrent GRAM access between `Menu_HandleKey` and `OLED_Show_Task`
4. **LED mode animation runs in `Menu_LEDModeStep()`** (called from `Key_Scan_Task` every 20ms), NOT blocking functions like `LED_Mode_Control()`
5. **`OLED_GRAM` clear loop + single `OLED_Refresh()` pattern** is the efficient display update — don't use multiple `OLED_ShowString()` calls for data display

## Key gotchas

- `OLED_DrawPoint(x, y, 0)` (clear pixel) uses a `~byte | bit | ~byte` pattern — correct but unusual
- `OLED_ScrollDisplay()` is an infinite loop — never call it
- FreeRTOS timers are **disabled** (`configUSE_TIMERS` commented out) — use `vTaskDelay` for periodic work
- `stdlib.h` is included by `oled.h` — `sprintf` works but consumes flash; prefer `OLED_ShowNum` / manual char drawing
- `OLED_ShowfloatNum` internally calls `OLED_ShowNum` (which does partial refresh) for both integer and fractional parts
