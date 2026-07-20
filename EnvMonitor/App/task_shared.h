/**
 * @file    task_shared.h
 * @brief   FreeRTOS 共享资源声明 — 互斥量 / 队列 / 事件组句柄
 * @note    所有任务通过此模块获取同步对象句柄，不直接定义 extern
 */

#ifndef TASK_SHARED_H
#define TASK_SHARED_H

#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

/* ==================== 互斥量句柄 ==================== */
extern SemaphoreHandle_t I2C_Mutex;      /* OLED + BH1750 共享 I2C1 */
extern SemaphoreHandle_t Data_Mutex;     /* g_sensorData 读写保护 */
extern SemaphoreHandle_t UART_Mutex;     /* ESP8266 独占 UART1 */

/* ==================== 队列句柄 ==================== */
extern QueueHandle_t BuzzerQueue;        /* 蜂鸣器指令: PatternCmd_t */
extern QueueHandle_t ButtonQueue;        /* 按键事件: Button_Event_t */

/* ==================== 蜂鸣器指令结构 ==================== */
typedef struct {
    uint8_t times;      /* 重复次数 */
    uint16_t on_ms;     /* 鸣叫时长 (ms) */
    uint16_t off_ms;    /* 间隔时长 (ms) */
} BuzzerCmd_t;

/* ==================== 初始化 ==================== */
void TaskShared_Init(void);

#endif /* TASK_SHARED_H */
