/**
 * @file    task_buzzer.c
 * @brief   BuzzerTask 实现 — 队列驱动的蜂鸣器模式播放
 * @note    阻塞在队列接收，收到指令后执行蜂鸣模式。
 *          AlarmTask 通过队列发送 BuzzerCmd_t 来控制蜂鸣。
 */

#include "main.h"
#include "task_buzzer.h"
#include "task_shared.h"

void BuzzerTask_Run(void *pvParameters)
{
    (void)pvParameters;
    BuzzerCmd_t cmd;

    while (1) {
        /* 阻塞等待蜂鸣指令（无指令时挂起，零 CPU 开销） */
        if (xQueueReceive(BuzzerQueue, &cmd, portMAX_DELAY) == pdTRUE) {
            /* 执行蜂鸣模式 */
            for (uint8_t i = 0; i < cmd.times; i++) {
                Buzzer_On();
                vTaskDelay(pdMS_TO_TICKS(cmd.on_ms));

                if (cmd.off_ms > 0) {
                    Buzzer_Off();
                    vTaskDelay(pdMS_TO_TICKS(cmd.off_ms));
                }
            }
            Buzzer_Off();  /* 确保最终关闭 */
        }
    }
}
