/**
 * @file    task_sensor.h
 * @brief   SensorTask — 5s 周期采集全部传感器数据
 */

#ifndef TASK_SENSOR_H
#define TASK_SENSOR_H

#include "FreeRTOS.h"
#include "task.h"

#define SENSOR_TASK_STACK_SIZE  256
#define SENSOR_TASK_PRIORITY    3
#define SENSOR_SAMPLE_PERIOD_MS 5000U

void SensorTask_Run(void *pvParameters);

#endif
