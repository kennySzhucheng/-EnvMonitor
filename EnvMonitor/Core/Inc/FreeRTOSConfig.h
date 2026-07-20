/**
 * @file    FreeRTOSConfig.h
 * @brief   FreeRTOS 内核配置 — STM32F103C8T6 (Cortex-M3, 72MHz, 20KB SRAM)
 * @note    编译器: ARMCC (Keil MDK), 移植层: RVDS/ARM_CM3
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "stm32f1xx.h"   /* 为了使用 SystemCoreClock */

/* ==================== 调度器配置 ==================== */
#define configUSE_PREEMPTION                    1
#define configUSE_TIME_SLICING                  1
#define configUSE_16_BIT_TICKS                  0       /* 32位 Tick 计数器（STM32 是 32位 MCU） */
#define configUSE_PORT_OPTIMISED_TASK_SELECTION 0       /* Cortex-M3 有前导零指令但为兼容性关闭 */
#define configUSE_TICKLESS_IDLE                 0       /* STM32F1 低功耗模式暂不启用 */

/* ==================== 内存分配方式 ==================== */
#define configSUPPORT_STATIC_ALLOCATION         0       /* 使用动态分配（xTaskCreate），不需要静态分配 */
#define configSUPPORT_DYNAMIC_ALLOCATION        1       /* 动态分配（使用 heap_4.c） */

/* ==================== 时钟与Tick ==================== */
#define configCPU_CLOCK_HZ                      ( SystemCoreClock )  /* 72,000,000 */
#define configTICK_RATE_HZ                      1000U               /* 1ms 节拍 */
#define configSYSTICK_CLOCK_HZ                  configCPU_CLOCK_HZ  /* SysTick 时钟源 = HCLK */

/* ==================== 优先级 ==================== */
#define configMAX_PRIORITIES                    5                   /* 0..4, 共5级 */
#define configMINIMAL_STACK_SIZE                64                  /* 空闲任务最小栈, 单位: word */
#define configMAX_TASK_NAME_LEN                 12

/* ==================== 内存管理 ==================== */
#define configTOTAL_HEAP_SIZE                   ( ( size_t ) ( 6 * 1024 ) )  /* 6KB 堆 */
#define configAPPLICATION_ALLOCATED_HEAP        0                            /* 使用 FreeRTOS 自带 heap_4.c */

/* ==================== 钩子函数 ==================== */
#define configUSE_IDLE_HOOK                     0
#define configUSE_TICK_HOOK                     0
#define configUSE_MALLOC_FAILED_HOOK            0   /* 关：不需要内存分配失败钩子 */
#define configUSE_DAEMON_TASK_STARTUP_HOOK      0
#define configCHECK_FOR_STACK_OVERFLOW          0   /* 关闭栈溢出检测（调试时可改为 1 或 2） */

/* ==================== 同步与通信 ==================== */
#define configUSE_MUTEXES                       1
#define configUSE_RECURSIVE_MUTEXES             1
#define configUSE_COUNTING_SEMAPHORES           1
#define configQUEUE_REGISTRY_SIZE               4
#define configUSE_QUEUE_SETS                    0

/* ==================== 软件定时器 ==================== */
#define configUSE_TIMERS                        1
#define configTIMER_TASK_PRIORITY               ( configMAX_PRIORITIES - 1 )  /* 优先级 4 */
#define configTIMER_TASK_STACK_DEPTH            128
#define configTIMER_QUEUE_LENGTH                8

/* ==================== 协程 ==================== */
#define configUSE_CO_ROUTINES                   0
#define configMAX_CO_ROUTINE_PRIORITIES         2

/* ==================== 任务通知 ==================== */
#define configUSE_TASK_NOTIFICATIONS            1
#define configTASK_NOTIFICATION_ARRAY_ENTRIES   3

/* ==================== 调试 & 运行时统计 ==================== */
#define configGENERATE_RUN_TIME_STATS           0
#define configUSE_TRACE_FACILITY                1
#define configUSE_STATS_FORMATTING_FUNCTIONS    0

/* ==================== 中断优先级 (Cortex-M3, 4-bit 抢占优先级) ==================== */
/* STM32 HAL 默认使用 NVIC_PRIORITYGROUP_4 (4位抢占优先级, 0位子优先级) */
/* 优先级值: 0=最高, 15=最低, 写入时左移4位 */

#define configPRIO_BITS                         4   /* STM32F1 使用 4 位优先级 */

/* 内核中断优先级（最低 = 15），用于 PendSV 和 SysTick */
/* FreeRTOS 会将 PendSV/SysTick 设为最低优先级，确保不打断其他中断 */
#define configKERNEL_INTERRUPT_PRIORITY         0xF0    /* 15 << 4 */

/* 最大系统调用中断优先级（5），优先级 5~15 的 ISR 可以调用 FreeRTOS API */
/* 优先级 0~4 的 ISR 不能调用 FreeRTOS API（但能抢占 RTOS，用于超低延迟场景） */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    0x50    /* 5 << 4 */

/* 等同宏（有些移植版本需要） */
#define configLIBRARY_KERNEL_INTERRUPT_PRIORITY         15
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

/* ==================== 断言 ==================== */
#define configASSERT( x )   if( ( x ) == 0 ) { taskDISABLE_INTERRUPTS(); while(1); }

/* ==================== 可选功能 ==================== */
#define INCLUDE_vTaskPrioritySet                 1
#define INCLUDE_uxTaskPriorityGet                1
#define INCLUDE_vTaskDelete                      1
#define INCLUDE_vTaskSuspend                     1
#define INCLUDE_vTaskDelayUntil                  1
#define INCLUDE_vTaskDelay                       1
#define INCLUDE_xTaskGetIdleTaskHandle           0
#define INCLUDE_xTaskAbortDelay                  0
#define INCLUDE_xQueueGetMutexHolder             1
#define INCLUDE_xSemaphoreGetMutexHolder          1
#define INCLUDE_xTaskGetHandle                   1
#define INCLUDE_eTaskGetState                    1
#define INCLUDE_xEventGroupSetBitFromISR         1
#define INCLUDE_xTimerPendFunctionCall           1
#define INCLUDE_xTaskGetSchedulerState           1
#define INCLUDE_xTaskGetCurrentTaskHandle        1
#define INCLUDE_pcTaskGetTaskName                1

/* ==================== 中断向量映射 (Keil 启动文件兼容) ==================== */
/* 将 FreeRTOS 移植层的函数名映射到 CMSIS 标准中断向量名 */
/* 注意: SysTick_Handler 不在此映射，因为我们需要在 stm32f1xx_it.c
          中同时调用 HAL_IncTick() 和 xPortSysTickHandler()         */
#define vPortSVCHandler         SVC_Handler
#define xPortPendSVHandler      PendSV_Handler

#endif /* FREERTOS_CONFIG_H */
