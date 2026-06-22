---
description: FreeRTOS 多任务数据采集系统（STM32F103C8Tx）的 AI 代理指南
alwaysApply: true
---

# AGENTS.md — FreeRTOS 多任务数据采集系统

## 平台信息

- **MCU**: STM32F103C8Tx (Cortex-M3)，72MHz，64KB Flash / 20KB RAM
- **RTOS**: FreeRTOS V202212.01，抢占式内核，5 级优先级，Tick 频率 1000Hz
- **堆**: 6KB (`configTOTAL_HEAP_SIZE`)，使用 heap_4 分配器
- **工具链**: ARM GCC (`cmake/gcc-arm-none-eabi.cmake`)，通过 CMake+Ninja 构建
- **语言**: C11（无 C++，STM32CubeMX 仅生成初始化代码）
- **链接脚本**: `STM32F103XX_FLASH.ld` — `_Min_Heap_Size=0x200`，`_Min_Stack_Size=0x400`，栈位于 RAM 末尾

## 构建与索引

```sh
cmake --preset Debug
cmake --build --preset Debug
```

`CMAKE_EXPORT_COMPILE_COMMANDS` 已启用；`.clangd` 期望路径为 `build/Debug`。

新增 `.c` 源文件请注册到 `cmake/user/CMakeLists.txt` 的 `USER_Application_Src` 列表中。

## 项目结构

| 路径 | 用途 |
|------|------|
| `BSP/Inc/`、`BSP/Src/` | 应用层驱动 + 菜单系统 |
| `Core/Inc/`、`Core/Src/` | HAL 初始化、`main()`、中断处理 |
| `FreeRTOS/` | 内核源码（未修改） |
| `cmake/user/CMakeLists.txt` | 注册新源文件 |
| `startup_stm32f103xb.s` | 中断向量表 |
| `test/` | 主机端单元测试（Unity 框架，x86_64 gcc） |

## 启动顺序 (`main.c`)

1. `HAL_Init()` → `SystemClock_Config()` → `MX_GPIO_Init()`
2. `MX_USART1_UART_Init()` → `OLED_Init()` → `ADC_DMA_Init()` → `ADC_Start_DMA()` → `LED_Init()` → `KEY_Init()`
3. `freertos_start()` 创建队列+信号量，启动 `Startled_Task`，调用 `vTaskStartScheduler()`

`Startled_Task` 创建三个工作任务后自我删除。

### 栈溢出钩子

在 `main.c` 中定义为 `vApplicationStackOverflowHook()` — 溢出时进入死循环。
通过 `configCHECK_FOR_STACK_OVERFLOW 2` 配置（方法 2：检查栈指针 + 栈尾部标记）。

## 任务一览

| 任务 | 函数 | 栈大小 | 优先级 | 职责 |
|------|------|--------|--------|------|
| `data_process_task` | `Data_Process_Task` | 256 | 2 | 等待 ADC DMA 信号量，对缓冲区求平均，发送 `SensorData_t*` 到队列 |
| `key_scan_task` | `Key_Scan_Task` | 256 | 2 | 轮询 `KEY_Scan()`，调用 `Menu_HandleKey()` + `Menu_LEDModeStep()`，20ms 延迟循环 |
| `oled_show_task` | `OLED_Show_Task` | 192 | 1 | 阻塞等待 `sensorDataQueue`，仅在 `Menu_IsDataShowActive()` 时调用 `OLED_Show()` |
| `startled_task` | `Startled_Task` | 128 | 1 | 创建上述三个任务后，调用 `Menu_Init()` 并自我删除 |

优先级 2 的任务可以抢占优先级 1 的任务。`Key_Scan_Task`（优先级 2）负责菜单状态转换，与 `OLED_Show_Task` 通过优先级实现互斥。

## 数据流

```
ADC DMA（循环模式，150 条目缓冲区）→ DMA TCIF1 ISR 给出 xSemaphoreFromISR
    → Data_Process_Task 获取信号量，按通道求平均，转换为物理量
    → xQueueSend(sensorDataQueue, &sensorData)
    → OLED_Show_Task 接收，若处于数据显示模式则调用 OLED_Show()
```

`SensorData_t` 结构体字段：`distance` / `temperature` / `illuminance`（float），`avg_adc_*`（uint16_t）。

`xSemaphore` 是二值信号量。`sensorDataQueue` 是 `SensorData_t*` 队列，长度为 1。

### ADC DMA ISR 详情（`adc1.c`）

- 中断源：`DMA1_Channel1_IRQn`，由 TCIF1（传输完成）触发
- 调用 `xSemaphoreGiveFromISR(xSemaphore, NULL)`，清除 `DMA1->IFCR` 的 TCIF1 标志
- NVIC 优先级：11（使用 `NVIC_PRIORITYGROUP_2`），在 `configMAX_SYSCALL_INTERRUPT_PRIORITY=191` 安全范围内

### 传感器转换公式

| 通道 | ADC 引脚 | 传感器 | 公式 |
|------|----------|--------|------|
| ch1 | PA1 | 距离 (GP2Y0A21) | `distance = 27.0 / voltage`，钳位 10-80cm |
| ch2 | PA2 | 温度 (NTC) | Steinhart-Hart：`1/T = A + B·ln(R) + C·(ln(R))³` |
| ch4 | PA4 | 光照强度 (LDR) | `illuminance = 242 / R^1.28`，上限钳位 9999 |

## 外设与驱动

### OLED（SSD1306 128×64，I2C 位敲驱动）
- SCL=PB11，SDA=PB10（软件 I2C，非硬件 I2C 外设）
- `OLED_ShowChar()` 仅写入 GRAM 缓冲区 **不**刷新屏幕
- `OLED_ShowString()` 写入字符后调用 `OLED_Refresh()`（完整 I2C 传输，约 104ms）
- `OLED_ShowNum()` / `OLED_ShowfloatNum()` 写入字符后局部刷新
- `OLED_Show()` 内部清除 GRAM 缓冲区，通过 `OLED_DrawStr()` 绘制标签，最后调用一次 `OLED_Refresh()`
- **在性能敏感的路径中避免调用 `OLED_ShowString()`** — 每次调用都会进行 1024 字节的完整 I2C 传输
- `OLED_GRAM[128][8]` 在 `oled.c` 中声明为 `static`
- 菜单系统使用内部辅助函数 `OLED_PutStr()`（位于 `menu_system.c`），该函数调用 `OLED_ShowChar()` **不触发刷新**
- `OLED_DrawPoint(x, y, 0)`（清除像素）使用 `~byte | bit | ~byte` 模式——正确但写法不常见
- `OLED_ScrollDisplay()` 是死循环——**切勿调用**

### LED（低电平有效）
- PB5..PB8，state=1 表示点亮（引脚输出低电平）
- 宏：`LED1(state)`、`LED1_TOGGLE()`，LED2~LED4 同理
- `Single_LED_Control(num, state)` — num 1-4，state 0/1
- `LED_Mode_Control(mode)` — **阻塞函数**（包含 `vTaskDelay` 循环），仅适合独立任务使用
- 非阻塞 LED 动画请使用 `Menu_LEDModeStep()`（由 `Key_Scan_Task` 每 20ms 调用）

### 按键（低电平有效，内部上拉）
- KEY1=PB4，KEY2=PB15，KEY3=PA8，KEY4=PA15
- `KEY_Scan()` — 轮询模式，边缘检测 + 10ms 消抖延迟，返回 `KEY_NONE` 或 `KEY[1-4]_PRESSED`
- `KEY_Scan()` **内部调用** `vTaskDelay(pdMS_TO_TICKS(10))` 进行消抖——每次按键会阻塞调用任务约 10ms

### ADC（DMA 循环模式，12 位）
- PA1（距离/ch1），PA2（温度/ch2），PA4（光照/ch4）
- `ADC_DMA_Init()`（寄存器级，非 HAL） + `ADC_Start_DMA()` 启动连续转换
- `adc_dma_buffer[150]` = 50 个采样 × 3 通道，排列顺序：ch1, ch2, ch4 循环
- DMA：循环模式，16 位数据宽度，高优先级，内存地址递增使能
- 采样时间：所有通道 239.5 个周期（ADC 时钟 = 72MHz / 6 = 12MHz → 约 20µs/采样）
- ADC 处于 **连续转换** + **扫描模式禁用**（3 通道单组），通过软件 `SWSTART` 触发
- ADC DMA 寄存器级初始化使用 `NVIC_Init(11, 0, ...)` 配合 `NVIC_PRIORITYGROUP_2`——如更改中断优先级，确保其 ≥ 11 以满足 FreeRTOS ISR API 安全要求

## 菜单系统（`menu_system.h` / `menu_system.c`）

5 状态状态机：`MENU_MAIN` → `MENU_LED_MODE_SELECT` / `MENU_SINGLE_LED_CTRL` / `MENU_DATA_SHOW` → `MENU_LED_MODE_RUNNING`

### 按键映射

| 按键 | 功能 |
|------|------|
| KEY1 | 确认 / 切换 LED |
| KEY2 | 向上 |
| KEY3 | 向下 |
| KEY4 | 返回 |

### 菜单项
- **LED Mode Select**：流水灯 / 闪烁 / 全亮 三种模式
- **Single LED Control**：单独控制 4 个 LED 亮灭
- **Mule Data Show**：实时显示 ADC 三通道采样数据

### 架构规则（来之不易的经验）

1. **`OLED_Show()` 自身会清空缓冲区** —— 调用者 **不要** 在之前调用 `OLED_Clear()`
2. **同一时间只有一个任务可以写入 `OLED_GRAM`** —— `Key_Scan_Task`（优先级 2）处理菜单渲染，`OLED_Show_Task`（优先级 1）处理数据显示；优先级机制保证互斥
3. **`Menu_DrawDataShow()` 不能从 `Menu_HandleKey()` 中调用** —— 之前的 bug 就是 `Menu_HandleKey` 与 `OLED_Show_Task` 并发访问 GRAM
4. **LED 模式动画在 `Menu_LEDModeStep()` 中运行**（由 `Key_Scan_Task` 每 20ms 调用），**不要**使用 `LED_Mode_Control()` 等阻塞函数
5. **`OLED_GRAM` 清除循环 + 单次 `OLED_Refresh()` 模式**是高效的显示更新方式——不要在数据显示中使用多次 `OLED_ShowString()` 调用

## 测试框架

主机端单元测试位于 `test/` 目录，使用 [Unity](https://github.com/ThrowTheSwitch/Unity) 测试框架。

```sh
cd test && make test
```

- 通过直接 `#include` `data_process.c` 来测试内部静态函数（白盒测试）
- FreeRTOS 和 STM32 HAL 通过 `test/mocks/` 头文件模拟
- 模拟实现位于 `test/test_data_process.c`，包括 `xSemaphoreTakeMock`、`xQueueGenericSend` 等
- 添加新测试时，在 `test/test_runner.c` 中注册测试组，并在 `test/test_data_process.c`（或新建文件）中添加测试用例

## 常见陷阱

- `stdlib.h` 由 `oled.h` 间接包含——`sprintf` 可用但消耗 Flash；优先使用 `OLED_ShowNum` / 手动字符绘制
- `OLED_ShowfloatNum` 内部对整数部分和小数部分都调用 `OLED_ShowNum`（会触发局部刷新）
- FreeRTOS 定时器 **已禁用**（`configUSE_TIMERS` 被注释）——周期性工作请使用 `vTaskDelay`
- `configUSE_TRACE_FACILITY` 和 `configUSE_STATS_FORMATTING_FUNCTIONS` 均已启用——`vTaskList()` / `vTaskGetRunTimeStats()` 可用于调试
- `configUSE_MUTEXES`、`configUSE_COUNTING_SEMAPHORES`、`configUSE_RECURSIVE_MUTEXES` 均已启用——所有 FreeRTOS 同步原语均可使用
- `configSUPPORT_DYNAMIC_ALLOCATION` 为 1，`configSUPPORT_STATIC_ALLOCATION` 为 0——仅使用动态内存分配
- 栈溢出检查方法 2 同时检查栈指针有效性和栈尾部标记——如果栈溢出钩子触发，检查任务栈大小是否足够
- `test/mocks/` 目录包含 STM32 硬件的简化模拟头文件——修改真实 HAL 调用时需同步更新模拟
