/**
 * @file    task_sensor.c
 * @brief   SensorTask 实现 — 采集 DHT22 / BH1750 / HC-SR04 / WiFi 状态
 * @note    DHT22: 读数据时进入临界区（微秒级时序，~2ms）
 *          BH1750: 需要 I2C 互斥量
 *          HC-SR04: 定时器输入捕获，阻塞但短
 *          WiFi状态: 需要 UART 互斥量
 */

#include "main.h"
#include "task_sensor.h"
#include "task_shared.h"

void SensorTask_Run(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    float val;

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(SENSOR_SAMPLE_PERIOD_MS));

        /* 临时存储（先采集再一次性写入，减少持锁时间） */
        float temperature = 0, humidity = 0, light = 0, distance = 0;
        uint8_t wifi_ok = 0;

        /* --- DHT22: 临界区保护（关全局中断 ~2ms） --- */
        taskENTER_CRITICAL();
        uint8_t dht_result = DHT22_Read(&temperature, &humidity);
        taskEXIT_CRITICAL();
        if (dht_result != DHT22_OK) {
            temperature = -999;   /* 标记无效，不更新 */
            humidity    = -999;
        }

        /* --- BH1750: I2C 互斥量 --- */
        xSemaphoreTakeRecursive(I2C_Mutex, pdMS_TO_TICKS(100));
        uint8_t bh_result = BH1750_ReadLight(&val);
        xSemaphoreGiveRecursive(I2C_Mutex);
        if (bh_result == 0) {
            light = val;
        } else {
            light = -999;
        }

        /* --- HC-SR04: 定时器捕获（无需互斥量，TIM3 独占 SR04） --- */
        if (SR04_Measure(&val) == 0) {
            distance = val;
        } else {
            distance = -999;
        }

        /* --- WiFi 状态 --- */
        xSemaphoreTakeRecursive(UART_Mutex, pdMS_TO_TICKS(50));
        wifi_ok = ESP8266_IsConnected();
        xSemaphoreGiveRecursive(UART_Mutex);

        /* --- 写入全局数据（持锁） --- */
        xSemaphoreTakeRecursive(Data_Mutex, portMAX_DELAY);
        if (temperature > -900) g_sensorData.temperature = temperature;
        if (humidity > -900)    g_sensorData.humidity    = humidity;
        if (light > -900)       g_sensorData.light       = light;
        if (distance > -900)    g_sensorData.distance    = distance;
        g_sensorData.wifi_connected = wifi_ok;
        xSemaphoreGiveRecursive(Data_Mutex);
    }
}
