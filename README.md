# FreeRTOS Multi-Task Data Acquisition System

STM32F103 (Cortex-M3) 多任务数据采集系统，基于 FreeRTOS 实时操作系统，集成 OLED 显示、ADC DMA 采样、LED 控制和按键菜单交互。

## 硬件需求

| 外设 | 引脚 | 说明 |
|------|------|------|
| OLED (SSD1306) | PB11(SCL), PB10(SDA) | 128×64, I2C 位敲驱动 |
| LED1~LED4 | PB5~PB8 | 有源低（state=1 点亮） |
| KEY1~KEY4 | PB4, PB15, PA8, PA15 | 有源低，内部上拉 |
| ADC1_CH1 (距离) | PA1 | DMA 循环采样 |
| ADC1_CH2 (温度) | PA2 | DMA 循环采样 |
| ADC1_CH4 (光照) | PA4 | DMA 循环采样 |

## 项目结构

```
├── BSP/Inc/              # 板级驱动头文件
│   ├── oled.h            # OLED 显示驱动
│   ├── led.h             # LED 控制驱动
│   ├── key.h             # 按键扫描驱动
│   ├── adc1.h            # ADC DMA 驱动
│   ├── data_process.h    # 传感器数据处理
│   ├── menu_system.h     # UI 菜单系统
│   └── freertos_taskstart.h
├── BSP/Src/              # 驱动实现
├── Core/Inc/ & Core/Src/ # HAL 初始化、main、中断
├── FreeRTOS/             # FreeRTOS V202212.01 内核源码
├── Drivers/              # STM32 HAL + CMSIS
├── cmake/                # CMake 构建配置
└── start_stm32f103xb.s   # 启动文件
```

## 构建

需要 ARM GCC 工具链。

```sh
cmake --preset Debug
cmake --build --preset Debug
```

## 任务架构

| 任务 | 栈 | 优先级 | 职责 |
|------|-----|--------|------|
| `Data_Process_Task` | 256 | 2 | 等待 ADC DMA 信号量，处理采样数据 |
| `Key_Scan_Task` | 256 | 2 | 轮询按键，驱动菜单和 LED 模式 |
| `OLED_Show_Task` | 192 | 1 | 数据显示模式下刷新 OLED |
| `Startled_Task` | 128 | 1 | 创建上述任务后自删除 |

数据流：`ADC DMA → ISR → Semaphore → Data_Process_Task → Queue → OLED_Show_Task`

## 菜单系统

| 按键 | 功能 |
|------|------|
| KEY1 | 确认 / 切换 LED |
| KEY2 | 向上 |
| KEY3 | 向下 |
| KEY4 | 返回 |

### 菜单层次

1. **LED Mode Select** — 选择 LED 模式（流水灯 / 闪烁 / 全亮）
2. **Single LED Control** — 单独控制 4 个 LED 亮灭
3. **Mule Data Show** — 实时显示 ADC 采样数据

## License

This project uses STM32 HAL and CMSIS under their respective licenses.
