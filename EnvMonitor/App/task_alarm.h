/**
 * @file    task_alarm.h
 * @brief   AlarmTask — 500ms 周期报警检查，控制 LED + 蜂鸣器
 */

#ifndef TASK_ALARM_H
#define TASK_ALARM_H

#include "FreeRTOS.h"
#include "task.h"

#define ALARM_TASK_STACK_SIZE   128
#define ALARM_TASK_PRIORITY     3
#define ALARM_CHECK_PERIOD_MS   500U

void AlarmTask_Run(void *pvParameters);

#endif
