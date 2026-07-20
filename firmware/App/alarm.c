/**
 * @file    alarm.c
 * @brief   报警逻辑实现
 * @note    报警规则（多条件可同时触发）:
 *           高温: 温度 > 30°C  → 红灯闪 + 蜂鸣器滴滴×3
 *           低湿: 湿度 < 30%   → 黄灯常亮
 *           靠近: 距离 < 20cm  → 绿灯闪 + 蜂鸣器短鸣×1
 *           WiFi断: 未连接    → 红灯常亮
 *
 *         共用蜂鸣器，按优先级: 高温 > 靠近 > 低湿
 */

#include "main.h"
#include "alarm.h"

void ALARM_Check(const SensorData_t *data)
{
    /* --- 高温报警（最高优先级） --- */
    if (data->temperature > ALARM_TEMP_HIGH) {
        LED_On(LED_RED);
        Buzzer_BeepPattern(3, 200, 200);   /* 滴滴滴三声 */
        return;  /* 高温优先，不再检查其他 */
    } else {
        LED_Off(LED_RED);
    }

    /* --- 有人靠近 --- */
    if (data->distance < ALARM_DIST_CLOSE && data->distance > 0) {
        LED_On(LED_GREEN);
        Buzzer_BeepPattern(1, 100, 0);     /* 短鸣一声 */
    } else {
        LED_Off(LED_GREEN);
    }

    /* --- 湿度太低 --- */
    if (data->humidity < ALARM_HUMI_LOW && data->humidity > 0) {
        LED_On(LED_YELLOW);
        /* 低湿不鸣叫（避免频繁打扰） */
    } else {
        LED_Off(LED_YELLOW);
    }

    /* --- WiFi 断连 --- */
    if (!data->wifi_connected) {
        /* 不用蜂鸣器只亮灯, WiFi断不太紧急 */
    }
}
