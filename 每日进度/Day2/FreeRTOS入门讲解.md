# 🎓 FreeRTOS 入门讲解 — 以环境监控器项目为例

> 今天我们把一个裸机程序（while(1)大循环）改成了 FreeRTOS 多任务架构。
> 这篇文档用人话解释：到底做了什么、为什么要这么做、每个概念是什么意思。

---

## 一、裸机 vs RTOS：一句话理解

### 裸机程序（昨天之前的样子）

```c
while (1) {
    if (过了5秒)  采集传感器();
    if (过了1秒)  刷新屏幕();
    if (过了60秒) 上传数据();
    if (过了半秒) 检查报警();
    扫描按键();
    蜂鸣器滴答();
}
```

就像一个**单线程的厨房**——你一个人洗菜、切菜、炒菜、装盘，全自己干。一个锅糊了也得等手上切完再说。

### RTOS 程序（今天改完的样子）

```
任务1: 每5秒采集传感器   (优先级3，高)
任务2: 每半秒检查报警    (优先级3，高)
任务3: 每1秒刷新屏幕     (优先级2，中)
任务4: 每20ms扫描按键    (优先级2，中)
任务5: 有指令时响蜂鸣器   (优先级2，中)
任务6: 每60秒上传云端    (优先级1，低)
```

就像雇了**6 个厨师**，各干各的。炒菜的炒菜，切菜的切菜，互不耽误。主厨（调度器）决定谁先用灶台。

---

## 二、核心概念，逐个击破

### 2.1 任务（Task）

**是什么**：一个独立的、永远在循环的函数。

```c
void SensorTask_Run(void *pvParameters) {
    while (1) {
        采集传感器();
        vTaskDelay(5000);  // 睡5秒，不占CPU
    }
}
```

**关键认知**：任务函数**永远不会 return**。它就像一个小 while(1)，一直在跑。

**vTaskDelay 和裸机 delay 的区别**：
- `HAL_Delay(1000)` → CPU 死等 1 秒，期间啥也不干
- `vTaskDelay(1000)` → 这个任务睡 1 秒，CPU 去跑其他任务

这就是 RTOS 最核心的价值：**等待的时候不浪费 CPU**。

---

### 2.2 优先级（Priority）与抢占

**规则：数字越大优先级越高。高优先级的任务就绪时，立即抢走 CPU。**

```
时间线 ──────────────────────────────────────────→

CloudTask(优先级1):   [=====上传数据=====>
                                   ↑
SensorTask(优先级3):              [采]  ← 抢占！CloudTask 被暂停
                                   ↓
CloudTask(优先级1):                    [=====继续上传=====>
```

**所以**：传感器采集（优先级3）绝不会被云端上传（优先级1）耽搁。反过来，上传时如果传感器要干活，上传会被暂停。

我们的设计逻辑：
- 传感器和报警 = 最高优先级（数据不能丢，报警不能晚）
- 屏幕和按键 = 中等（人眼感觉不到几十ms的延迟）
- 云端上传 = 最低（网络本来就慢，等一会无所谓）

---

### 2.3 互斥量（Mutex）— 共享资源的"门锁"

**问题场景**：OLED 和 BH1750 光照传感器共用 I2C 总线（两根线 PB6/PB7）。

```
DisplayTask:  [正在写OLED屏幕数据...>
SensorTask:       [突然想读BH1750光照...>
                  ↑ 冲突！两根线上同时有两种数据，全乱套
```

**解决方案**：互斥量 = 厕所门锁。

```c
// DisplayTask 中：
xSemaphoreTake(I2C_Mutex);   // 🔒 锁门，拿钥匙
OLED_显示数据();              // 安全使用 I2C
xSemaphoreGive(I2C_Mutex);   // 🔓 开门，还钥匙

// SensorTask 中：
xSemaphoreTake(I2C_Mutex);   // 🔒 等钥匙（如果 DisplayTask 在用就排队）
BH1750_读光照();              // 安全使用 I2C
xSemaphoreGive(I2C_Mutex);   // 🔓 还钥匙
```

**一句话**：谁拿到锁谁用，用完了还回去。别人想用就排队等。

我们的项目有 3 把锁：
| 锁名 | 保护什么 | 谁在用 |
|------|---------|--------|
| I2C_Mutex | PB6/PB7 总线 | DisplayTask(OLED) + SensorTask(BH1750) |
| Data_Mutex | g_sensorData 全局变量 | 写:SensorTask / 读:Display+Alarm+Cloud |
| UART_Mutex | USART1 串口 | CloudTask(上云) + DisplayTask(WiFi重连) |

---

### 2.4 队列（Queue）— 任务间的"传纸条"

**问题场景**：ButtonTask 发现按键被按下，但 DisplayTask 才负责处理按键逻辑。怎么通知它？

**裸机做法**：全局变量 `g_button_pressed = 1`，DisplayTask 轮询检查。

**RTOS 做法**：队列 = 传送带。

```c
// ButtonTask: 往队列里塞纸条
uint8_t msg = KEY1_PRESSED;
xQueueSend(ButtonQueue, &msg, 0);  // 塞进去，不等

// DisplayTask: 从队列里取纸条
uint8_t msg;
if (xQueueReceive(ButtonQueue, &msg, 0) == pdTRUE) {
    处理按键(msg);  // 收到纸条了！
}
```

**好处**：
- 不会丢事件（裸机全局变量可能被覆盖）
- 先进先出（先按的键先处理）
- 队列空了接收方就跳过（不阻塞）

我们的项目有 2 条队列：
| 队列名 | 谁发 | 谁收 | 传什么 |
|--------|------|------|--------|
| ButtonQueue | ButtonTask | DisplayTask | 按键事件 |
| BuzzerQueue | AlarmTask | BuzzerTask | 蜂鸣次数/时长 |

---

### 2.5 调度器（Scheduler）— 大管家

**启动调度器**：

```c
vTaskStartScheduler();  // 这行代码之后，main() 永远不会再往下走了
```

调度器一旦启动：
1. 检查所有任务的优先级和状态
2. 挑优先级最高的就绪任务运行
3. 定时（1ms tick）检查是否需要切换任务
4. 任务主动 `vTaskDelay()` 时切换到其他任务

**SysTick 定时器**就是调度器的"心跳"。每 1ms 产生一次中断，调度器在中断里判断"要不要换个人干活"。

---

## 三、今天的迁移做了什么

### 改之前的 main.c：

```
初始化 → while(1) { 一堆 if(到时间了) }  ← 30行
```

### 改之后的 main.c：

```
初始化 → 创建6个任务 → vTaskStartScheduler()  ← 从此 CPU 被 RTOS 接管
```

### 每个"到时间了就干"变成了独立任务：

| 裸机写法 | RTOS 写法 |
|---------|----------|
| `if (过了5秒) APP_SampleSensors()` | SensorTask 独立运行，vTaskDelayUntil 精确每 5 秒 |
| `if (过了半秒) ALARM_Check()` | AlarmTask 独立运行 |
| `每次循环 Button_Scan()` | ButtonTask 每 20ms 精准扫描 |
| `每次循环 Buzzer_Tick()` | BuzzerTask 阻塞等待指令（零 CPU 开销） |

---

## 四、SysTick 的三合一魔法

今天最关键的一个修改在 `stm32f1xx_it.c` 的 SysTick_Handler：

```c
void SysTick_Handler(void) {
    HAL_IncTick();              // ① HAL 库需要（HAL_Delay 等）
    g_sysTick++;                // ② 我们自己的毫秒计数
    xPortSysTickHandler();      // ③ FreeRTOS 的任务调度时钟
}
```

**SysTick 就像一个 1ms 响一次的闹钟**，响了之后三个"室友"各干各的：
- HAL 库更新自己的计数器
- 我们更新 g_sysTick
- FreeRTOS 检查要不要切换任务

重点是那句 `if (调度器还没启动) 别调 FreeRTOS`——调度器启动前内核结构还没初始化，调了直接死机。

---

## 五、SVC 和 PendSV 是干嘛的

这两个是 Cortex-M3 内核级的"特权中断"，FreeRTOS 独占使用，我们不用管。

**SVC（系统服务调用）**：启动第一个任务时用。FreeRTOS 通过它设置栈指针（MSP→PSP 切换），然后跳转到第一个任务的代码。

**PendSV（可挂起系统调用）**：每次切换任务时用。它把所有寄存器的值保存到当前任务的栈里，再从下一个任务的栈里恢复回来——就像游戏存档/读档。

```c
// FreeRTOSConfig.h 中的两行宏：
#define vPortSVCHandler    SVC_Handler    // 启动第一个任务 = FreeRTOS 的 SVC 函数
#define xPortPendSVHandler PendSV_Handler // 任务切换 = FreeRTOS 的 PendSV 函数
```

这两行告诉 Keil 的启动文件："别用你的空 SVC_Handler，用 FreeRTOS 的版本"。

---

## 六、FreeRTOSConfig.h 是什么

这是 FreeRTOS 的"设置菜单"。关键配置：

| 配置 | 我们的值 | 什么意思 |
|------|---------|---------|
| configTICK_RATE_HZ | 1000 | 调度器心跳 1ms 一次 |
| configMAX_PRIORITIES | 5 | 支持 5 级优先级 (0~4) |
| configTOTAL_HEAP_SIZE | 6KB | RTOS 可用内存（任务栈、队列等都从这里分） |
| configUSE_MUTEXES | 1 | 启用互斥量功能 |
| configUSE_RECURSIVE_MUTEXES | 1 | 启用递归互斥量（同一个任务可以多次锁） |

**堆大小 6KB 够用吗？**
- 6 个任务栈 ≈ 5KB
- 队列 + 互斥量结构体 ≈ 500B
- 内核自己 ≈ 300B
- 剩余约 200B 余量

STM32F103C8T6 总 SRAM 20KB，FreeRTOS 用了 6KB，剩余 14KB 给全局变量和 HAL 库——完全够。

---

## 七、今天踩的坑

### 坑1：configUSE_16_BIT_TICKS 没定义

FreeRTOS V10.x 新增的必选配置，忘了写。设为 0 = 32 位计数器，否则 49 天后 tick 溢出。

### 坑2：静态分配未关

开了 `configSUPPORT_STATIC_ALLOCATION=1` 但没提供对应的内存分配函数。FreeRTOS 想要我们告诉它"Idle 任务的栈放哪里"，但我们选择动态分配（`xTaskCreate` 自动从堆里拿），所以直接关掉。

### 坑3：调试钩子函数没实现

FreeRTOS 提供了"钩子"机制——发生特定事件时回调我们的函数。比如栈溢出时调用 `vApplicationStackOverflowHook`。但我们没写这些函数，编译器找不到就报错。解决方案：**全关**——这些都是调试用的，不影响功能。

---

## 八、新旧对比一览

| 维度 | 裸机 (v1.0) | FreeRTOS (v2.0) |
|------|-----------|----------------|
| 代码结构 | 1 个 while(1) 大循环 | 6 个独立任务 |
| 定时精度 | `if (g_sysTick - last > N)` 近似 | `vTaskDelayUntil()` 精确 |
| 任务切换 | 不支持（一个卡全卡） | 抢占式（高优先级立即可用） |
| 共享资源 | 单线程，不需要保护 | 3 个互斥量保护 I2C/数据/UART |
| 任务通信 | 全局变量 | 队列（先进先出，不丢事件） |
| 等待方式 | HAL_Delay 死等 | vTaskDelay 切到其他任务 |
| 实时性 | 差（DHT22 关中断期间全停） | 好（仅 SensorTask 关中断 2ms） |
| 专业性 | 课程设计级别 | 量产项目级别 |

---

## 九、如果想继续深入

1. **调优栈大小**：启用 `configCHECK_FOR_STACK_OVERFLOW=2` + 实现钩子，看实际栈使用量
2. **添加软件定时器**：用 FreeRTOS Timer 替代 vTaskDelayUntil（更省内存）
3. **中断与任务的配合**：传感器数据通过中断收集 → 信号量通知任务处理
4. **低功耗**：空闲任务钩子进入 Sleep 模式（电池供电场景）

---

> 一句话总结今天：**把 6 个"到点就干"的 if 判断，变成了 6 个独立的、被 RTOS 调度的、可以互相抢占的 while(1) 任务。**
