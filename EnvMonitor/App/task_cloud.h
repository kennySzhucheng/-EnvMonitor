/**
 * @file    task_cloud.h
 * @brief   CloudTask — 60s 周期数据上云 (ThingSpeak HTTP GET)
 */

#ifndef TASK_CLOUD_H
#define TASK_CLOUD_H

#include "FreeRTOS.h"
#include "task.h"

#define CLOUD_TASK_STACK_SIZE   512
#define CLOUD_TASK_PRIORITY     1           /* 最低优先级，不干扰 UI 和采集 */
#define CLOUD_UPLOAD_PERIOD_MS  60000U

void CloudTask_Run(void *pvParameters);

#endif
