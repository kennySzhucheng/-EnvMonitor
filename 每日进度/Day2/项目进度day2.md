# 📋 项目进度记录

> ⚠️ **重要：每次关闭对话框前，请在下方记录当天的进度。**
> 下次打开时，从这里接着做。
> claude会自动更新进度和日志，此举动仅为每日保存记录。

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
| P8 | ⬜ 待开始 | PCB 设计 + 打样焊接 | - | - |
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


