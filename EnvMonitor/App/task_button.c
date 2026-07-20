/**
 * @file    task_button.c
 * @brief   ButtonTask 实现 — 扫描按键，通过队列发送事件
 */

#include "main.h"
#include "task_button.h"
#include "task_shared.h"

void ButtonTask_Run(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();

    while (1) {
        /* 精确 20ms 周期 */
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(BUTTON_SCAN_PERIOD_MS));

        /* 扫描三个按键 */
        Button_Scan();

        /* 检查三个按键的事件 */
        Button_Event_t events[BTN_COUNT];
        events[BTN_PAGE]    = Button_GetEvent(BTN_PAGE);
        events[BTN_CONFIRM] = Button_GetEvent(BTN_CONFIRM);
        events[BTN_BACK]    = Button_GetEvent(BTN_BACK);

        /* 有事件则发送到队列（事件值 = BTN_ID << SHIFT | BTN_EVENT） */
        for (uint8_t i = 0; i < BTN_COUNT; i++) {
            if (events[i] != BTN_EVENT_NONE) {
                uint8_t msg = (i << 4) | events[i];  /* 高4位=按键ID, 低4位=事件 */
                xQueueSend(ButtonQueue, &msg, 0);     /* 非阻塞发送 */
            }
        }
    }
}
