/**
 * @file    task_alarm.c
 * @brief   AlarmTask 实现 — 读取传感器数据、判断阈值、控制声光报警
 * @note    报警规则（优先级从高到低）:
 *           高温 >30°C  → 红灯闪 + 滴滴滴×3（最高优先，触发后跳过其余）
 *           靠近 <20cm  → 绿灯闪 + 短鸣×1
 *           低湿 <30%   → 黄灯常亮（不鸣叫）
 *           BuzzerTask 通过队列异步执行蜂鸣，不阻塞报警循环
 */

#include "main.h"
#include "task_alarm.h"
#include "task_buzzer.h"
#include "task_shared.h"

void AlarmTask_Run(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    BuzzerCmd_t cmd;

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(ALARM_CHECK_PERIOD_MS));

        /* 获取传感器数据快照 */
        xSemaphoreTakeRecursive(Data_Mutex, portMAX_DELAY);
        SensorData_t local = g_sensorData;
        xSemaphoreGiveRecursive(Data_Mutex);

        /* ====== 高温报警（最高优先级） ====== */
        if (local.temperature > ALARM_TEMP_HIGH) {
            LED_On(LED_RED);
            cmd.times  = 3;
            cmd.on_ms  = 200;
            cmd.off_ms = 200;
            xQueueOverwrite(BuzzerQueue, &cmd);
            continue;   /* 高温优先，跳过其余检查 */
        } else {
            LED_Off(LED_RED);
        }

        /* ====== 有人靠近 ====== */
        if (local.distance < ALARM_DIST_CLOSE && local.distance > 0.0f) {
            LED_On(LED_GREEN);
            cmd.times  = 1;
            cmd.on_ms  = 100;
            cmd.off_ms = 0;
            xQueueOverwrite(BuzzerQueue, &cmd);
        } else {
            LED_Off(LED_GREEN);
        }

        /* ====== 湿度太低 ====== */
        if (local.humidity < ALARM_HUMI_LOW && local.humidity > 0.0f) {
            LED_On(LED_YELLOW);
        } else {
            LED_Off(LED_YELLOW);
        }
    }
}
