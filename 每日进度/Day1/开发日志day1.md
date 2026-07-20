# 🛠️ 开发日志 — 智能环境监测与物联网控制站

> 记录项目的关键设计决策、踩坑经验和解决思路。
> 适合发到 GitHub 做项目文档，也方便复盘回顾。

---

## 📅 2026-07-15 — 项目启动 + 方案设计

### Q: 为什么选这个方案？

- STM32F103C8T6 是最经典的学习平台，有大量社区资源，出问题容易查
- 覆盖三种总线（I2C、1-Wire、UART），学一个项目打通嵌入式通信基础
- 有显示、有输入、有声光、有无线通信——组件齐全，属于完整 IoT 终端

### Q: 为什么 OLED 和 BH1750 用同一组 I2C？

- 两者都是 I2C 器件且地址不同（OLED=0x78, BH1750=0x46），不会冲突
- 共用 PB6/PB7 省下 GPIO 给后续扩展
- 这是工程上常见做法：一条总线挂多个从设备

### Q: 为什么先面包板再 PCB？

- 面包板阶段能随便改接线，发现设计错了不受惩罚
- PCB 打样一块板子 5 元但如果改一版就是 5+5，面包板改线零成本
- 功能全部调通再画 PCB，保证一次成功

### Q: 为什么用有源蜂鸣器而不是无源蜂鸣器？

- 有源蜂鸣器通电即响，不用生成 PWM 频率信号
- 报警场景只需要"滴滴滴"，不需要播放音乐旋律
- 一个 GPIO 拉高就响，代码量和调试难度都更低

### Q: 为什么用 ESP8266 AT 指令而不是直接编程 ESP8266？

- STM32 做主控更灵活：传感器数据先在本机处理，要传什么再发
- AT 指令方式调试方便：用串口助手就能看到通信过程
- ESP8266 出厂自带 AT 固件，不用刷 SDK

### 📌 引脚分配原则

- **PA9/PA10** → USART1（ESP8266 通信），这是 STM32 固定分配
- **PA13/PA14** → 保留 SWD 调试接口（烧录程序、断点调试）
- **PB6/PB7** → I2C1（硬件 I2C），比软件模拟 I2C 更稳定
- **PA0** → DHT22 需要精确微秒时序
- **PB0/PB1** → HC-SR04 Echo 需要定时器输入捕获
- **PB12-PB15** → LED、蜂鸣器等普通 GPIO 外设

---

## 📅 2026-07-16（上） — 固件代码编写

### 💡 整体架构：为什么分三层？

```
Drivers/  → 硬件抽象层（"怎么读传感器"）
   ↓
App/      → 业务逻辑层（"数据怎么用"）
   ↓
Core/     → 调度层（"什么时候做什么"）
```

好处：改一个传感器不影响别的模块。比如想把 DHT22 换成 DHT11，只改 `Drivers/DHT22/` 内的代码，App 层完全不需要动。

### 💡 各驱动设计要点

**DHT22 — 为什么关中断？**
单总线时序要求微秒级精度。SysTick 中断恰好 1ms 一次，如果在读 40bit 数据时被中断打断，高电平计时就偏了，校验必错。`__disable_irq()` / `__enable_irq()` 包围整个读取过程，2ms 内完成，不影响其他任务。

**HC-SR04 — 输入捕获反转极性**
先捕获上升沿记录起始时间，立刻切到下降沿捕获结束时间。硬件定时器自动记录，不用软件轮询（会被中断干扰）。TIM3 时钟 1MHz（1us精度），`距离(cm) = 脉宽(us) / 58`。

**按键 — 状态机消抖**
每个按键 4 个状态：空闲 → 消抖等待(30ms) → 确认按下 → 长按(1s)。
30ms 过滤机械抖动，1s 触发长按，超过 1s 后每 200ms 产生 HOLD 连发事件。事件读后自动清除，不会重复触发。

**ESP8266 — 为什么用 HTTP GET 而不是 MQTT？**
Thingspeak 原生支持 HTTP GET 更新数据，不需要 MQTT broker。AT 指令模式：TCP连接 → HTTP请求 → 关闭TCP，链路清晰。后续升级 MQTT 只需改 `esp8266.c`，应用层接口不变。

**I2C 总线共用**
OLED(0x78) 和 BH1750(0x46) 地址不同，共用 PB6/PB7。HAL 的 I2C 函数是同步阻塞的，主循环里顺序调用不会冲突。上 FreeRTOS 后只需加一个互斥信号量保护。

**为什么选 Thingspeak？**
免费（每年 300 万条消息），有现成的图表和数据导出。自建服务器要花钱租云服务，不符合 100 元预算。注册：https://thingspeak.com → 创建 Channel(4个Field) → 填 API Key 到 `esp8266.h`。

---

## 📅 2026-07-16（下） — CubeMX 工程整合 + Keil 编译

### 💡 关键踩坑记录

#### Q1: 标准库 vs HAL 库，会不会有影响？

**不会。** 标准库学到的是寄存器思维，HAL 只是换了个函数名。初始化代码 CubeMX 生成，驱动逻辑完全自己写。标准库的 `GPIO_SetBits()` → HAL 的 `HAL_GPIO_WritePin()`，本质上操作同一个寄存器。HAL 是当前行业标准，简历和 GitHub 上更有说服力。

#### Q2: CubeMX 中 TIM3 的 Slave Mode 和 Trigger Source？

**都不选。** Slave Mode = Disabled，Trigger Source 保持灰色。HC-SR04 只需要 TIM3_CH4 做 Input Capture，不需要外部信号触发/复位/门控定时器。
- Prescaler = 72-1 = 71 → 1MHz = 1us 精度
- Period = 65535（最大计数）
- Channel4 = Input Capture direct mode

#### Q3: USART1 Mode 选什么？

**Asynchronous（异步模式）。** ESP8266 和 STM32 就是最普通的 TX+RX 两根线全双工串口，不需要同步时钟线，不需要硬件流控（RTS/CTS）。
- Baud Rate: 115200
- Word Length: 8 Bits
- Parity: None
- Stop Bits: 1

#### Q4: Clock Configuration 中 HSE 是黑的？

因为在 Pinout 页面没启用 HSE。**System Core → RCC → High Speed Clock → Crystal/Ceramic Resonator**，回到 Clock Configuration 就亮了。
- HSE(8MHz) → PLL Source Mux → PLL(×9) → SYSCLK(72MHz)
- AHB=72, APB2=72, APB1=36

#### Q5: Keil 编译 cannot open source input file "oled.h"？

头文件搜索路径没加。**Project → Options for Target → C/C++ → Include Paths → ...** → 添加 `..\Drivers\OLED` 等 9 个目录。`..\` 是从 MDK-ARM 目录向上一级到达 EnvMonitor 根目录。

#### Q6: 加了路径还是报 cannot open "oled.h"？（关键问题）

**根本原因：循环包含。**

```
main.h ──→ #include "oled.h"
               ↓
           oled.h ──→ #include "main.h"
                         ↓
                    头文件保护符(__MAIN_H)挡住
                         ↓
                    main.h 中后续的类型定义都不生效
                         ↓
                    编译器报错"找不到"
```

**解决方案：**
1. 所有驱动/应用的 `.h` 文件：`#include "main.h"` → `#include "stm32f1xx_hal.h"`
2. 所有 `.c` 文件：在 include 自己的 `.h` 之前先 `#include "main.h"`
3. `main.h` 中把所有宏定义和 typedef 移到 include 驱动头文件之前

**修复后的包含链：**
```
.c 文件 → #include "main.h" → 拿到所有类型定义
       → #include "自己.h" → 拿到函数声明
```

#### Q7: Undefined symbol SystemClock_Config？

替换 main.c 时把 CubeMX 生成的 `SystemClock_Config()` 函数体搞丢了。CubeMX 原版 main.c 底部有这个函数定义，替换后需要补回来。

---

## 🗺️ 当前阶段执行路线图

```
P0: 项目规划 + 器件采购     ✅ 2026-07-15
P1: GPIO (LED/按键/蜂鸣器)  ✅ 2026-07-16 代码就绪
P2: OLED 显示              ✅ 2026-07-16 代码就绪
P3: 传感器驱动 (DHT22/BH1750) ✅ 2026-07-16 代码就绪
P4: OLED 菜单              ✅ 2026-07-16 代码就绪
P5: HC-SR04 + 报警         ✅ 2026-07-16 代码就绪
P6: ESP8266 WiFi + 云      ✅ 2026-07-16 代码就绪
--- 代码与编译分界线 ---
P1-P6: CubeMX 工程整合      ✅ 2026-07-16 Keil 编译通过
P1-P6: 硬件烧录验证         ⏳ 等待器件到货
P7: FreeRTOS 重构           ⬜ 待开始
P8: PCB 设计 + 打样         ⬜ 待开始
P9: GitHub 发布             ⬜ 待开始
```

### 经验总结：花在工程配置上的时间值不值？

**值。** HAL 库工程配置（CubeMX + Keil + 循环包含修复）是整个项目中最容易卡住的环节。今天踩的坑——循环包含、头文件路径、SystemClock_Config 丢失——都是 HAL 和 CubeMix 组合的经典问题。**搞通一次，以后所有 STM32 项目都能快速起步。**

### 后续注意事项

1. **每次 CubeMX 重新生成代码后**，Include Paths 会被重置，需要重新添加 Drivers 和 App 路径
2. **WiFi 账号密码** 在 `esp8266.h` 中修改，不要提交到 GitHub（用 .gitignore 或环境变量）
3. **Thingspeak API Key** 同理，属于私密信息
4. **GPIO 配置** 如被 CubeMX 覆盖，需重新手动修正（PA0开漏、按键上拉等）
