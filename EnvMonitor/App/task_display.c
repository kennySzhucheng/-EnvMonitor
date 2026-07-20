/**
 * @file    task_display.c
 * @brief   DisplayTask 实现 — OLED 菜单渲染 + 按键事件消费
 * @note    通过队列接收 ButtonTask 发送的按键事件
 *          通过 Data_Mutex 安全读取传感器数据
 *          通过 I2C_Mutex 独占 OLED I2C 总线
 */

#include "main.h"
#include "task_display.h"
#include "task_shared.h"
#include "app.h"

/* ==================== 静态页面状态 ==================== */
static App_Page_t currentPage = PAGE_MAIN;

/* ==================== 内部函数 ==================== */
static void HandleKeyEvent(uint8_t msg)
{
    uint8_t btn_id  = (msg >> 4) & 0x0F;
    uint8_t event   = msg & 0x0F;
    (void)btn_id;  /* 保留用于多按键区分 */

    switch ((Button_Event_t)event) {
        case BTN_EVENT_SHORT:
            if (btn_id == BTN_PAGE) {
                currentPage = (currentPage + 1) % PAGE_COUNT;
            } else if (btn_id == BTN_CONFIRM) {
                if (currentPage == PAGE_WIFI) {
                    xSemaphoreTakeRecursive(UART_Mutex, pdMS_TO_TICKS(200));
                    ESP8266_ConnectWiFi();
                    xSemaphoreGiveRecursive(UART_Mutex);
                }
            } else if (btn_id == BTN_BACK) {
                currentPage = PAGE_MAIN;
            }
            break;

        case BTN_EVENT_LONG:
            if (btn_id == BTN_CONFIRM) {
                currentPage = PAGE_MAIN;
            }
            break;

        default:
            break;
    }
}

/* ==================== 任务入口 ==================== */
void DisplayTask_Run(void *pvParameters)
{
    (void)pvParameters;
    TickType_t xLastWakeTime = xTaskGetTickCount();
    uint8_t keyMsg;

    while (1) {
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(DISPLAY_REFRESH_PERIOD_MS));

        /* --- 消费按键队列（非阻塞，有多少处理多少） --- */
        while (xQueueReceive(ButtonQueue, &keyMsg, 0) == pdTRUE) {
            HandleKeyEvent(keyMsg);
        }

        /* --- 读取传感器数据 --- */
        xSemaphoreTakeRecursive(Data_Mutex, pdMS_TO_TICKS(50));
        SensorData_t local = g_sensorData;
        xSemaphoreGiveRecursive(Data_Mutex);

        /* --- 渲染页面（需要 I2C 总线） --- */
        xSemaphoreTakeRecursive(I2C_Mutex, pdMS_TO_TICKS(100));
        {
            OLED_Clear();

            switch (currentPage) {
                case PAGE_MAIN:     Menu_ShowMain(&local);     break;
                case PAGE_DETAIL:   Menu_ShowDetail(&local);   break;
                case PAGE_SETTINGS: Menu_ShowSettings();        break;
                case PAGE_WIFI:     Menu_ShowWiFi(&local);     break;
                case PAGE_ABOUT:    Menu_ShowAbout();           break;
                default: break;
            }

            OLED_Refresh();
        }
        xSemaphoreGiveRecursive(I2C_Mutex);
    }
}
