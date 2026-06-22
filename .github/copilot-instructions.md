# 嵌入式项目通用开发规范

> 本文件适用于所有基于 MCU 的嵌入式 C/C++ 项目。
> 所有 AI 生成的代码、注释、文档和提交信息必须严格遵守以下规则。

---

## 🌐 语言要求

- **所有 AI 生成的回答、代码注释、文档和 Git 提交信息必须使用简体中文。**
- 仅保留必要的技术术语（如函数名、变量名、寄存器地址、API 名称）为英文。
- 代码中的字符串常量（如日志输出）如需国际化，可保留英文，但必须附中文注释说明。

---

## 🧠 核心编码哲学

### 1. 资源优先原则
嵌入式系统资源（RAM/ROM/CPU时间）有限，任何代码生成必须遵循：
- **RAM 优先**：全局变量 > 静态变量 > 栈变量 > 堆变量（禁止 malloc/free）
- **ROM 优先**：`const` 修饰只读数据，`#define` 替代 `const` 以节省 ROM
- **速度优先**：关键路径（中断、高频调用函数）禁用函数指针、禁用递归、禁用浮点运算（除非硬件 FPU）

### 2. 可移植性要求
- 硬件相关代码（寄存器操作、外设初始化）必须**隔离**在 `HAL/` 或 `BSP/` 目录
- 核心算法（滤波、协议解析）必须**平台无关**，不包含任何硬件寄存器操作
- 使用标准整数类型：`uint8_t`, `int16_t`, `uint32_t`，禁止使用 `int`, `short`, `long`（长度不确定）

### 3. 防御性编程
- 所有指针使用前必须判空（`if (ptr == NULL)`）
- 所有数组访问必须检查边界（使用宏 `ARRAY_SIZE(x)` 获取数组长度）
- 所有函数返回值必须检查（特别是 `HAL_StatusTypeDef` 类型）
- 所有循环必须有最大迭代次数保护（防止死循环）

---

## 🔧 代码风格规范

### 命名规则

| 类型 | 规范 | 示例 |
|------|------|------|
| 文件名 | 全小写+下划线 | `uart_driver.c`, `i2c_utils.h` |
| 函数名 | `模块_动作_对象` | `uart_send_byte()`, `timer_start_pwm()` |
| 全局变量 | `g_` 前缀 + 驼峰 | `g_systemTick`, `g_uartRxBuffer` |
| 静态变量 | `s_` 前缀 + 驼峰 | `s_irqCounter`, `s_filterCoeff` |
| 宏定义 | 全大写+下划线 | `#define BUFFER_SIZE 256` |
| 枚举/结构体 | 全大写+下划线（类型名加 `_t`） | `typedef enum { STATE_IDLE } state_t;` |
| 中断函数 | `ISR_` 前缀 | `ISR_USART1_RX()`, `ISR_TIM2_OVF()` |

### 文件头注释模板（每个 `.c/.h` 文件必须包含）

```c
/**
 * @file        filename.c
 * @brief       文件功能简述（如：UART 驱动实现）
 * @author      作者名
 * @date        2026-06-22
 * @version     1.0
 * @copyright   Copyright (c) 2026
 * 
 * @details     详细说明：
 *              - 支持波特率 9600 ~ 115200
 *              - 使用 DMA 接收，中断发送
 *              - 线程安全（FreeRTOS 互斥锁保护）
 */

### 函数注释模板（Doxygen 风格，所有函数必须包含）

/**
 * @brief       函数功能简述
 * @param[in]   param1  参数1说明
 * @param[out]  param2  参数2说明（输出参数）
 * @return      0: 成功, -1: 失败, 其他: 错误码
 * @note        特殊注意事项（如：不可在中断中调用）
 * @see         相关函数引用
 */
int function_name(uint8_t param1, uint32_t *param2);

### 🧩 模块划分与目录结构

项目必须按功能模块分层，目录结构建议如下：
/ProjectRoot
├── /Core                    # 核心代码（启动文件、系统配置）
├── /HAL                     # 硬件抽象层（UART, I2C, SPI, GPIO, TIM, ADC, DMA）
├── /BSP                     # 板级支持包（LCD, 传感器, Flash, EEPROM）
├── /Middleware              # 中间件（RTOS, 文件系统, 网络协议栈）
├── /Application             # 应用层（Tasks, Protocol, UI）
├── /Utilities               # 工具函数（Filter, Math, CRC, RingBuffer）
├── /Config                  # 配置文件（链接脚本, RTOS配置）
└── /Docs                    # 文档
模块间依赖规则（由 AI 严格遵循）：
- Application 可以调用 HAL、BSP、Middleware、Utilities
- Middleware 可以调用 HAL、Utilities
- BSP 可以调用 HAL、Utilities
- HAL 只能调用芯片标准库（如 CMSIS、HAL 库），不能调用 Utilities
- Utilities 必须平台无关，不能调用任何硬件相关代码

### ⚡ 实时性与 RTOS 规范
任务优先级分配原则（适用于 FreeRTOS / RT-Thread）
| 优先级 | 任务类型 | 说明 |
| 最高 | 硬件中断（ISR） | 由硬件触发，响应时间 < 1us |
| 高 | 传感器采集任务 | 周期性采集，采样率 >= 100Hz |
| 中高 | 通信任务（UART/SPI 收发） | 数据包处理，超时敏感 |
| 中 | UI 更新任务 | LVGL 刷新周期 20~50ms |
| 低 | 日志记录、后台维护 | 非实时，可被抢占 |
RTOS 使用规则
- 任务函数必须包含 while(1) 循环，并在循环内调用 vTaskDelay() 或 rt_thread_sleep()
- 任务间通信必须使用队列（Queue）或消息邮箱，禁止使用全局变量传递实时数据
- 共享资源访问必须使用互斥锁（Mutex），禁止使用二值信号量替代（会导致优先级翻转）
- 中断服务函数只能使用带 FromISR 后缀的 API（如 xQueueSendFromISR）
- 每个任务栈大小必须通过 uxTaskGetStackHighWaterMark() 验证，确保栈使用率 < 80%

### 📦 内存管理

- 堆内存：默认禁用 malloc() / free()，使用静态分配或内存池
- 栈内存：禁止在栈上分配大数组（> 256 字节）
- 全局内存：使用 static 限制作用域
- 常量数据：使用 const 修饰，放入 .rodata 段
- DMA 缓冲区：需按 32 字节对齐（Cache 行大小）

### 🖥️ 中断服务函数 (ISR) 规范

- ISR 必须尽可能短（< 1ms）
- 耗时操作必须通过队列/信号量委托给任务执行
- ISR 中禁止调用：printf()、malloc()、带阻塞的 API
- 中断向量表必须包含所有使用的中断服务函数

### 🧪 编译与调试

| 操作 | 命令 |
|------|------|
| 编译（Debug） | make DEBUG=1 |
| 编译（Release） | make RELEASE=1 |
| 烧录 | make flash |
| 调试（GDB） | make debug |
| 静态分析 | cppcheck --enable=all . |
编译警告级别：必须使用 -Wall -Wextra -Wpedantic -Wshadow -Wconversion，不允许有警告。

### 🔒 安全与可靠性

- 看门狗：必须在 main() 初始化时启动，定期喂狗
- 电源监控：掉电时立即保存关键数据到 EEPROM/Flash
- 错误处理：外设初始化失败时，进入 Error_Handler() 并点亮故障 LED
- 关键数据：存储在外部 Flash/EEPROM 的数据必须带 CRC 校验

### 📝 Git 提交规范

- 格式：<类型>: <中文简短描述>
- 类型：feat(新功能), fix(修复), docs(文档), refactor(重构), perf(性能), test(测试), chore(构建/工具)
- 禁止提交：.o, .elf, .hex, .bin, IDE临时文件, 调试日志