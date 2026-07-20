# 📋 项目进度记录

> ⚠️ **重要：每次关闭对话框前，请在下方记录当天的进度。**
> 下次打开时，从这里接着做。

---

## 📌 项目概述

| 项目 | 智能环境监测与物联网控制站 |
|------|---------------------------|
| 主控 | STM32F103C8T6 |
| 预算 | 约 76 元 |
| 目标 | 桌面环境终端 + WiFi数据上云 + GitHub开源 |

---

## 🗂️ 项目结构

```
电路实训/
├── PROGRESS.md          ← 进度清单（你在这里）
├── README.md            ← GitHub 项目主页
├── .gitignore
├── docs/
│   ├── 接线图.md         ← 引脚分配表
│   └── 开发日志.md       ← 关键对话 + 踩坑记录 + 设计思路
├── EnvMonitor/          ← CubeMX + Keil 工程（当前工作目录）
│   ├── Core/            ← HAL 配置 + main.c
│   ├── Drivers/         ← 8个传感器/外设驱动
│   ├── App/             ← 应用层
│   └── MDK-ARM/         ← Keil 工程文件
├── firmware/            ← 固件源码副本（与 EnvMonitor 同步）
├── hardware/pcb/        ← PCB 设计（后续）
└── assets/              ← 图片/素材
```

---

## 🔢 开发阶段

| 阶段 | 状态 | 内容 | 开始 | 完成 |
|------|------|------|------|------|
| P0 | ✅ 完成 | 项目规划 + 器件采购 | 2026/07/15 | 2026/07/15 |
| P1 | 📝 代码就绪 | GPIO 基础：LED亮灭、蜂鸣器、按键 | 2026/07/16 | 2026/07/16 |
| P2 | 📝 代码就绪 | OLED 驱动：I2C + SSD1306，显示文字 | 2026/07/16 | 2026/07/16 |
| P3 | 📝 代码就绪 | 传感器驱动：DHT22(1-Wire) + BH1750(I2C) | 2026/07/16 | 2026/07/16 |
| P4 | 📝 代码就绪 | OLED 多页面菜单 + 按键切换 | 2026/07/16 | 2026/07/16 |
| P5 | 📝 代码就绪 | HC-SR04 超声波 + 蜂鸣器报警逻辑 | 2026/07/16 | 2026/07/16 |
| P6 | 📝 代码就绪 | ESP8266 WiFi + HTTP 数据上云 | 2026/07/16 | 2026/07/16 |
| P7 | ✅ 完成 | FreeRTOS 任务重构 | 2026/07/16 | 2026/07/17 |
| P8 | 🔄 进行中 | PCB 设计 + 打样焊接 | 2026/07/17 | - |
| P9 | ⬜ 待开始 | 整机联调 + GitHub README + 项目发布 | - | - |

---

## 📝 进度日志

### 2026-07-15 — 项目启动

- [x] 确定项目方案：智能环境监测与物联网控制站
- [x] 完成器件清单，下单购买
- [x] 创建项目文件夹结构
- [ ] 待器件到货
- [x] 详细设计思路 → 见 `docs/开发日志.md`

### 2026-07-16（上） — 固件代码全部完成

- [x] Core：HAL配置、全局头文件、main.c（26个源文件覆盖 P1-P6）
- [x] Drivers：LED / Button / Buzzer / OLED(含字库) / DHT22 / BH1750 / HC-SR04 / ESP8266
- [x] App：状态机 / 5页面菜单 / 三级报警逻辑
- [x] 各驱动设计要点 → 见 `docs/开发日志.md`

### 2026-07-16（下） — CubeMX 工程整合 + Keil 编译通过 ✅

- [x] CubeMX 配置引脚/时钟/外设 → 生成 MDK-ARM 工程
- [x] 修正 gpio.c（PA0→开漏、按键→上拉、补 LED 引脚）
- [x] 修正 tim.c（Prescaler=71, Filter=4）
- [x] 替换 main.h + main.c
- [x] 复制 Drivers + App 到工程
- [x] 修复循环包含（11个.h → stm32f1xx_hal.h，11个.c → 加 main.h）
- [x] 补回 SystemClock_Config()
- [x] **Keil 编译通过，0 Error！**
- [x] 踩坑记录（7个关键问题）→ 见 `docs/开发日志.md`

---

### 2026-07-17 — P7 FreeRTOS 任务重构 ✅ 完成

- [x] 从 STM32Cube F1 固件包提取 FreeRTOS V10.3.1 内核源码
- [x] 创建 FreeRTOSConfig.h（6KB堆、5级优先级、1ms tick）
- [x] 创建 task_shared 共享资源模块（3互斥量 + 2队列）
- [x] 编写 6 个 RTOS 任务：Sensor / Display / Cloud / Alarm / Button / Buzzer
- [x] 重写 main.c 为 FreeRTOS 调度器启动
- [x] 修改 stm32f1xx_it.c（SVC/PendSV→FreeRTOS, SysTick→HAL+RTOS三合一）
- [x] 精简 app.c/app.h 为共享类型定义
- [x] Keil 工程配置（8个 FreeRTOS 源文件 + 7个任务文件 + 3条 include 路径）
- [x] 修复编译问题：补 configUSE_16_BIT_TICKS + configSUPPORT_STATIC/DYNAMIC_ALLOCATION
- [x] 修复链接问题：关 MallocFailedHook + StackOverflowHook + StaticAllocation 钩子
- [x] **Keil 编译通过，0 Error！**

### 编译踩坑

| 问题 | 报错 | 解决 |
|------|------|------|
| 缺 configUSE_16_BIT_TICKS | `#error Missing definition` | V10.x 新增宏，FreeRTOSConfig.h 补上 |
| 静态分配未关 | `Undefined vApplicationGetIdleTaskMemory` | configSUPPORT_STATIC_ALLOCATION=0 |
| 钩子未实现 | `Undefined vApplicationMallocFailedHook` 等 | MallocFailedHook/StackOverflowHook 全关 |
| 栈检测方法2需钩子 | `Undefined vApplicationStackOverflowHook` | configCHECK_FOR_STACK_OVERFLOW=0 |

### Keil 工程配置记录

> 已在 Keil 中完成：
> - 分组 FreeRTOS/Kernel: tasks.c, queue.c, list.c, timers.c, event_groups.c, croutine.c
> - 分组 FreeRTOS/Port: port.c (RVDS/ARM_CM3), heap_4.c (MemMang)
> - 分组 App/Tasks: 7 个 task_*.c 文件
> - Include 路径: Drivers/FreeRTOS/include, portable/RVDS/ARM_CM3, Core/Inc

### 2026-07-17（下）— P8 PCB 原理图设计 🔄

- [x] 方案选型：确定方案B（全插件/模块化），Blue Pill 核心板通过排母连接
- [x] 工具选型：选择 KiCad 10.0（非立创EDA），Python 脚本辅助生成
- [x] 31 个元件全部放置完毕（11电阻 + 5电容 + 3LED + 1二极管 + 1三极管 + 3按键 + 1蜂鸣器 + 6连接器）
- [x] 7 组电路逐组布局：电源 → I2C上拉 → 模块接口 → 按键 → LED → 蜂鸣器 → 核心板插座
- [x] 15 条信号网络全部与引脚分配表对照验证通过
- [x] ERC 从 82 错误逐轮降至 6 错误
- [x] 踩坑记录（4个关键问题）→ 见 `docs/开发日志.md`

### PCB 原理图踩坑

| 问题 | 现象 | 解决 |
|------|------|------|
| 标签悬空 | KEY/DHT22等标签在空白处，网络分裂 | 标签必须放在导线上，KiCad 通过同名标签合并网络 |
| 按键电路拓扑错误 | R和C在开关两边，MCU脚孤悬读不到电压 | R下端 + C上端 + SW一脚 + MCU标签 = 同一交汇点；SW另一脚 → GND |
| 消抖电容未接地 | C18-C20 pin2 悬空，RC滤波不工作 | 电容负极直接连 GND，不可用 No-Connect |
| 开关角色反转 | 开关夹在 R+C 交汇点和 MCU 之间 | 开关一脚接交汇点，另一脚接 GND；MCU 直接挂在交汇点上 |

### 原理图最终状态

| 项目 | 数值 |
|------|------|
| 总元件 | 31 个（不含电源符号） |
| 信号网络 | 15 条全部验证通过 |
| 电源网络 | +5V(8pins) / +3.3V(17pins) / GND(37pins) |
| ERC 错误 | 6 个（电源声明，不影响功能） |
| KiCad 工程 | `hardware/pcb/EnvMonitor/EnvMonitor.kicad_sch` |

> ⚠️ 对话框意外关闭导致部分 PCB 开发记忆丢失，本次日志基于项目文件逆向还原。

---
## 🛑 下次接着做

>
> **当前状态（2026-07-18 收尾）：**
> - ✅ 器件已下单，等待到货
> - ✅ P0-P6 裸机固件全部完成并编译通过
> - ✅ P7 FreeRTOS 重构完成并编译通过（6任务 + 3互斥量 + 2队列）
> - 🔄 P8: PCB 原理图完成，待导入 PCB 布局布线
> - ⬜ P9: 整机联调 + GitHub 发布
>
> **工程位置：** `C:\Users\Monesy\Desktop\Envmonitor\`
> **KiCad 原理图：** `hardware\pcb\EnvMonitor\EnvMonitor.kicad_sch`
> **Keil 工程：** `EnvMonitor\MDK-ARM\EnvMonitor.uvprojx`
>
> **下次选项：**
> - 继续 P8 → 原理图导入 PCB Editor，布局布线，导出 Gerber 打样
> - 如果硬件已到货 → 面包板接线验证
> - 如果想继续优化代码 → 添加栈溢出检测钩子 / 优化内存配置
>
