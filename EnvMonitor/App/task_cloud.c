/**
 * @file    task_cloud.c
 * @brief   CloudTask 实现 — 每 60 秒将传感器数据上传到 ThingSpeak
 * @note    最低优先级 (prio=1)，ESP8266 AT 指令阻塞时间 5~15s
 *          UART_Mutex 序列化所有 ESP8266 通信
 */

#include "main.h"
#include "task_cloud.h"
#include "task_shared.h"

void CloudTask_Run(void *pvParameters)
{
    (void)pvParameters;

    /* 等系统稳定后首次上云（启动后 10s） */
    vTaskDelay(pdMS_TO_TICKS(10000));
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(CLOUD_UPLOAD_PERIOD_MS));

        /* 读取传感器数据快照 */
        xSemaphoreTakeRecursive(Data_Mutex, portMAX_DELAY);
        SensorData_t local = g_sensorData;
        xSemaphoreGiveRecursive(Data_Mutex);

        /* 获取 UART 锁 (ESP8266 操作耗时较长，但 CloudTask 是最低优先级不碍事) */
        xSemaphoreTakeRecursive(UART_Mutex, portMAX_DELAY);
        {
            /* 检查并确保 WiFi 连接 */
            if (!ESP8266_IsConnected()) {
                ESP8266_ConnectWiFi();
            }

            /* 推送数据到 ThingSpeak */
            if (ESP8266_IsConnected()) {
                ESP8266_SendData(local.temperature,
                                 local.humidity,
                                 local.light,
                                 local.distance);
            }
        }
        xSemaphoreGiveRecursive(UART_Mutex);
    }
}
