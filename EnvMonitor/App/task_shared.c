/**
 * @file    task_shared.c
 * @brief   FreeRTOS 共享资源初始化
 */

#include "main.h"
#include "task_shared.h"

/* ==================== 全局同步对象定义 ==================== */
SemaphoreHandle_t I2C_Mutex   = NULL;
SemaphoreHandle_t Data_Mutex  = NULL;
SemaphoreHandle_t UART_Mutex  = NULL;
QueueHandle_t     BuzzerQueue = NULL;
QueueHandle_t     ButtonQueue = NULL;

/* ==================== 初始化 ==================== */
void TaskShared_Init(void)
{
    /* 互斥量（均为递归型，避免同一任务重复加锁死锁） */
    I2C_Mutex  = xSemaphoreCreateRecursiveMutex();
    Data_Mutex = xSemaphoreCreateRecursiveMutex();
    UART_Mutex = xSemaphoreCreateRecursiveMutex();

    /* 蜂鸣器指令队列（容量 4，足够缓冲多级报警） */
    BuzzerQueue = xQueueCreate(4, sizeof(BuzzerCmd_t));

    /* 按键事件队列（容量 8，短时间多次按键不丢失） */
    ButtonQueue = xQueueCreate(8, sizeof(uint8_t));  /* 直接传 Button_Event_t 枚举值 */

    /* 检查创建结果 */
    configASSERT(I2C_Mutex != NULL);
    configASSERT(Data_Mutex != NULL);
    configASSERT(UART_Mutex != NULL);
    configASSERT(BuzzerQueue != NULL);
    configASSERT(ButtonQueue != NULL);
}
