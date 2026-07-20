/**
 * @file    app.c
 * @brief   应用层共享函数（初始化等）
 * @note    传感器采样、显示更新、云端上送、报警、按键处理
 *          已分别迁移至 task_sensor / task_display / task_cloud / task_alarm / task_button
 */

#include "main.h"
#include "app.h"

void APP_Init(void)
{
    /* ESP8266 连接由 CloudTask 在启动后自行处理 */
}
