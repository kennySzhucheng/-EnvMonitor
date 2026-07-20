/**
 * @file    task_buzzer.h
 * @brief   BuzzerTask — 从队列接收蜂鸣指令并执行
 */

#ifndef TASK_BUZZER_H
#define TASK_BUZZER_H

#include "FreeRTOS.h"
#include "task.h"

#define BUZZER_TASK_STACK_SIZE  128
#define BUZZER_TASK_PRIORITY    2

void BuzzerTask_Run(void *pvParameters);

#endif
