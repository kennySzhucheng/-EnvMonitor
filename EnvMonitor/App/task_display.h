/**
 * @file    task_display.h
 * @brief   DisplayTask — 1s 周期 OLED 刷新 + 按键事件处理
 */

#ifndef TASK_DISPLAY_H
#define TASK_DISPLAY_H

#include "FreeRTOS.h"
#include "task.h"

#define DISPLAY_TASK_STACK_SIZE     256
#define DISPLAY_TASK_PRIORITY       2
#define DISPLAY_REFRESH_PERIOD_MS   1000U

void DisplayTask_Run(void *pvParameters);

#endif
