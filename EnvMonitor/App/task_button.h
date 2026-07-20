/**
 * @file    task_button.h
 * @brief   ButtonTask — 20ms 周期按键扫描，事件发送到队列
 */

#ifndef TASK_BUTTON_H
#define TASK_BUTTON_H

#include "FreeRTOS.h"
#include "task.h"

#define BUTTON_TASK_STACK_SIZE  128
#define BUTTON_TASK_PRIORITY    2
#define BUTTON_SCAN_PERIOD_MS   20U

void ButtonTask_Run(void *pvParameters);

#endif
