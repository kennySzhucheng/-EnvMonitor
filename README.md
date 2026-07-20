# EnvMonitor — 智能环境监测站

> STM32F103C8T6 + FreeRTOS + ESP8266 桌面环境监测终端 | 练习项目

一个从零开始搭建的嵌入式 IoT 项目：裸机固件 → FreeRTOS 重构 → KiCad 画板 → 嘉立创打样，走完嵌入式产品全流程。

---

## 📸 实物展示

> ⏳ PCB 已下单，等待到货焊接后补图。

---

## ✨ 功能

| 模块 | 作用 |
|------|------|
| 🌡️ **DHT22** | 温湿度传感器，单总线(1-Wire)通信，精度 ±0.5°C / ±2%RH，挂在 PA0 上 |
| ☀️ **BH1750** | 环境光强度传感器，I²C 接口，0~65535 lux 量程，和 OLED 共用 PB6/PB7 |
| 📏 **HC-SR04** | 超声波测距模块，探测前方物体距离，用于"有人靠近"检测，PB0 发触发信号、PB1 输入捕获回波 |
| 🖥️ **OLED SSD1306** | 0.96 寸 I²C 显示屏，5 页菜单循环切换(温湿度/光照/距离/WiFi/系统信息) |
| 📡 **ESP8266-01S** | WiFi 模块，通过 UART1(PA9/PA10) 走 AT 指令，HTTP GET 方式每 60s 上传一次 ThingSpeak |
| 🚨 **三色 LED** | 红色(高温>35°C)、黄色(湿度<30%)、绿色(有人靠近<30cm)，分别接 PB12/13/14 |
| 🔔 **蜂鸣器** | 5V 有源模块，出现报警条件时由 BuzzerTask 队列驱动蜂鸣 |
| 🎛️ **三按键** | KEY1 页面切换 / KEY2 确认 / KEY3 返回，PA4/5/6，消抖电容 + 10kΩ 外部上拉 |
| ⏱️ **FreeRTOS** | 6 个任务抢占式调度，3 个互斥量保护共享资源，2 条队列做任务间通信 |
| 🛡️ **自定义 PCB** | KiCad 10 设计，80×60mm 双面板，全插件 27 元件，双面 GND 铺铜 |

---

## 🧱 硬件架构

```
                    ┌──────────────────────────────────┐
                    │        STM32F103C8T6 (72MHz)     │
                    │           Blue Pill 核心板         │
                    │                                  │
    DHT22 ──────────┤ PA0 (1-Wire)                     │
    KEY1~3 ─────────┤ PA4/PA5/PA6 (GPIO Input)        │
    ESP8266 ────────┤ PA9/PA10 (USART1, 115200bps)     │
    HC-SR04 ────────┤ PB0(Trig) / PB1(Echo, TIM3_CH4) │
    OLED+BH1750 ────┤ PB6/PB7 (I2C1, 共用总线)          │
    LED_R/Y/G ──────┤ PB12/PB13/PB14 (GPIO Output)     │
    Buzzer ─────────┤ PB15 (GPIO Output, 直连模块IO)    │
                    └──────────────────────────────────┘

    电源: USB 5V → Blue Pill → AMS1117 → 3.3V（全板供电来自 USB）
```

---

## 📐 引脚分配

| 引脚 | 外设 | 功能 |
|------|------|------|
| PA0 | DHT22 | 1-Wire 单总线 |
| PA4/PA5/PA6 | KEY1/KEY2/KEY3 | 按键输入(10kΩ 上拉) |
| PA9/PA10 | ESP8266 | USART1 TX/RX |
| PB0 | HC-SR04 Trig | 触发信号 |
| PB1 | HC-SR04 Echo | TIM3_CH4 输入捕获 |
| PB6/PB7 | OLED + BH1750 | I2C1 SCL/SDA（共用总线） |
| PB12/PB13/PB14 | LED_R/Y/G | 报警指示灯 |
| PB15 | Buzzer | 有源蜂鸣器模块 IO |
| PA13/PA14 | SWDIO/SWCLK | ST-Link 调试（保留） |

---

## 🖥️ 软件架构

```
                    ┌───────────────┐
                    │   FreeRTOS    │
                    │  V10.3.1      │
                    ├───────┬───────┤
                    │ 互斥量 ×3     │  队列 ×2
                    │ I2C / Data   │  Button → Display
                    │ / UART       │  Alarm → Buzzer
                    └───────┴───────┘
                           │
    ┌──────────────────────┼──────────────────────┐
    │                      │                      │
┌───▼──────┐  ┌──────▼─────┐  ┌────────▼───────┐
│SensorTask│  │DisplayTask │  │  CloudTask      │
│ (5s)     │  │ (1s)       │  │  (60s)          │
│ Prio:3   │  │ Prio:2     │  │  Prio:1         │
│          │  │             │  │                 │
│ DHT22    │  │ OLED 5页   │  │ ESP8266 AT指令  │
│ BH1750   │  │ 菜单渲染    │  │ HTTP GET        │
│ HC-SR04  │  │             │  │ ThingSpeak ☁️   │
└──────────┘  └─────────────┘  └─────────────────┘

┌──────────┐  ┌──────────────┐  ┌──────────────┐
│AlarmTask │  │ BuzzerTask   │  │ ButtonTask    │
│ (500ms)  │  │ (事件驱动)    │  │ (20ms)        │
│ Prio:3   │  │ Prio:2       │  │ Prio:2        │
│          │  │              │  │               │
│ 三级报警  │  │ xQueueReceive│  │ 状态机消抖     │
│ 阈值判断  │──▶ 阻塞等待     │  │ 队列发送事件   │
└──────────┘  └──────────────┘  └──────────────┘
```

**任务优先级原则：** 采集 > 报警 > 显示 = 按键 = 蜂鸣 > 上云

---

## 📦 物料清单 (BOM)

### PCB 焊接件（27个）

| 类型 | 规格 | 数量 | 位号 |
|------|------|------|------|
| 排母 | 1×20P 2.54mm | 2 | H1, H2 |
| 排针 | 1×4P 2.54mm | 3 | J2, J3, J4 |
| 排针 | 2×4P 2.54mm | 1 | J5 |
| 排针 | 1×3P 2.54mm | 1 | LS1 |
| 电阻 | 330Ω 1/4W | 3 | R10, R11, R12 |
| 电阻 | 4.7kΩ 1/4W | 3 | R4, R5, R6 |
| 电阻 | 10kΩ 1/4W | 3 | R7, R8, R9 |
| 瓷片电容 | 100nF (104) | 3 | C18, C19, C20 |
| 电解电容 | 100µF/10V | 2 | C21, C22 |
| LED 3mm | 红/黄/绿 | 3 | D2, D3, D4 |
| 轻触开关 | 6×6mm | 3 | SW2, SW3, SW4 |

### 外接模块（7个）

| 模块 | 数量 |
|------|------|
| STM32F103C8T6 Blue Pill | 1 |
| DHT22 温湿度传感器 | 1 |
| GY-302 光照 (BH1750) | 1 |
| OLED 0.96" SSD1306 | 1 |
| HC-SR04 超声波 | 1 |
| ESP8266-01S WiFi | 1 |
| 5V 有源蜂鸣器模块 | 1 |

> 💰 全部物料实际花费约 **90 元**（含 PCB 打样 20 元 + 模块 51 元 + 分立元件 13 元 + 工具 5 元）。完整清单见 `hardware/pcb/EnvMonitor/BOM_EnvMonitor.md`

---

## 🔧 开发环境

| 工具 | 用途 |
|------|------|
| **Keil MDK5** | 固件编译/烧录 |
| **STM32CubeMX** | 引脚配置 + HAL 代码生成 |
| **STM32CubeF1 V1.8.7** | HAL 库 + FreeRTOS 源码 |
| **ST-Link V2** | 调试/烧录器 |
| **KiCad 10.0** | 原理图 + PCB 设计 |
| **ThingSpeak** | IoT 数据可视化云平台 |

### 编译

1. 用 CubeMX 打开 `EnvMonitor.ioc` 重新生成代码（如需要）
2. `EnvMonitor\MDK-ARM\EnvMonitor.uvprojx` → Keil 打开
3. 确认 Include Paths 包含：`Drivers/*`、`App/`、`FreeRTOS/include`、`FreeRTOS/portable/RVDS/ARM_CM3`
4. Build (F7) → 0 Error → Download (F8)

### ThingSpeak 配置

1. 注册 https://thingspeak.com → Create Channel → 4 Fields
2. 将 Write API Key 填入 `esp8266.h` 中的 `THINGSPEAK_API_KEY`
3. 修改 WiFi 账号密码：`esp8266.h` → `WIFI_SSID` / `WIFI_PASS`

---

## 📂 项目结构

```
EnvMonitor/
├── firmware/               ← 固件源码（裸机版 + FreeRTOS版）
│   ├── Core/               HAL 配置 + main.c
│   ├── Drivers/            8 个外设驱动
│   │   ├── DHT22/          温湿度传感器
│   │   ├── BH1750/         光照传感器
│   │   ├── HC_SR04/        超声波测距
│   │   ├── OLED/           SSD1306 显示 + 字库
│   │   ├── ESP8266/        WiFi + ThingSpeak
│   │   ├── LED/            三色指示灯
│   │   ├── Button/         按键(状态机消抖)
│   │   └── Buzzer/         蜂鸣器
│   └── App/                应用层(状态机/菜单/报警)
│
├── EnvMonitor/             ← Keil + CubeMX 工程
│   ├── Core/               CubeMX 生成(含 main.c)
│   ├── Drivers/            同上 8 驱动 + ST HAL 库
│   ├── App/                6 个 FreeRTOS 任务 + task_shared
│   ├── MDK-ARM/            Keil 工程文件
│   └── EnvMonitor.ioc      CubeMX 项目
│
├── hardware/pcb/EnvMonitor/ ← KiCad PCB 设计
│   ├── EnvMonitor.kicad_*  工程文件
│   ├── BOM_EnvMonitor.md   完整物料清单
│   ├── EnvMonitor_Gerber_Full.zip  嘉立创打样包
│   ├── gerber/             Gerber 源文件(11层)
│   └── analysis/           出厂分析报告
│
└── docs/                   项目文档
    ├── 接线图.md            引脚分配表
    ├── 开发日志.md           设计决策 + 踩坑记录
    ├── 原理图讲解.md
    ├── KiCad入门讲解.md
    ├── PCB布局布线入门.md
    └── FreeRTOS入门讲解.md
```

---

## 📝 开发历程

| 阶段 | 内容 | 状态 |
|------|------|------|
| P0 | 项目规划 + 器件采购 | ✅ |
| P1-P6 | 裸机固件：8 驱动 + 5 页菜单 + 报警 + 上云 | ✅ |
| P7 | FreeRTOS 6 任务重构 | ✅ |
| P8 | KiCad 原理图 → 布局 → 布线 → DRC → Gerber | ✅ |
| P9 | 嘉立创打样 → 焊接 → 联调 → 发布 | 🔄 |

### 🔥 关键问题与解决

> 完整记录见 [`docs/开发日志.md`](docs/开发日志.md)

| # | 问题 | 根因 | 解决 |
|---|------|------|------|
| 1 | Keil 编译 `cannot open oled.h` | **循环包含**：`main.h` ↔ 驱动 `.h` 互相 include | 驱动 `.h` 改用 `stm32f1xx_hal.h`，`.c` 先 include `main.h` |
| 2 | FreeRTOS 链接 4 个 Undefined | 调试钩子未实现，V10.x 默认开启 | 关闭 MallocFailedHook / StackOverflowHook / StaticAllocation |
| 3 | KiCad 标签放空白处 | 标签必须附着在导线上，否则 = 独立网络 | 标签放在导线段上，高亮确认后放下 |
| 4 | 按键消抖电路出错 | R/C/SW/MCU 未在同一节点，MCU 脚孤悬 | 四者交汇一点，SW 另一脚 → GND |
| 5 | 14 个元件 PCB 导入失败 | 封装名缺脚距后缀（如 `_P2.54mm_Vertical`） | 编辑 `.kicad_sch` 批量替换封装名 |
| 6 | 蜂鸣器方案紧急变更 | 实物 3 脚模块 vs 原理图 2 脚+NPN 驱动 | 换 3 脚排针，删 Q1/R13/R14/D5，MCU 直连 |
| 7 | GND 铺铜 DRC 仍报未连接 | 铺铜只创建轮廓，没填充 | 按 **B** 键填充所有区域 |
| 8 | Keil 缺 `configUSE_16_BIT_TICKS` | FreeRTOS V10.x 新增必选宏 | `FreeRTOSConfig.h` 补定义 |
| 9 | 面包板测试失败，跳过直接上 PCB | STM32 内部上拉 ~40kΩ 太弱，面包板寄生干扰导致引脚浮空、按键读值随机跳变 | 外部 10kΩ 上拉 + PCB 实体焊点可彻底解决，不在面包板上浪费时间 |

### 💡 设计决策

| 决策 | 选择 | 理由 |
|------|------|------|
| RTOS | FreeRTOS 原生 API | 不依赖 CMSIS-RTOS 封装，社区资源多 |
| WiFi | ESP8266 AT 指令 + HTTP GET | ThingSpeak 原生支持，无需 MQTT broker |
| PCB | 全插件 THT + 模块化 | 新手可焊，Blue Pill 已有电路不重复画 |
| EDA | KiCad 10.0 | 开源免费，S-expression 格式透明 |
| I2C | OLED + BH1750 同总线 | 地址不同(0x3C/0x46)，不冲突，省 GPIO |

---

## 📄 许可

MIT License
