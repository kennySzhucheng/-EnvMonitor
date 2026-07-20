/**
 * @file    app.c
 * @brief   应用层实现 — 传感器采样、状态机调度、上下层衔接
 * @note    数据流向:
 *          传感器 → g_sensorData → OLED显示 / 云端上送 / 报警判断
 *         按键 → 页面切换 → 刷新显示
 */

#include "main.h"
#include "app.h"

static App_Page_t currentPage = PAGE_MAIN;

/* ==================== 初始化 ==================== */
void APP_Init(void)
{
    currentPage = PAGE_MAIN;

    /* 尝试连接 WiFi（失败不阻塞） */
    ESP8266_ConnectWiFi();
}

/* ==================== 传感器采样 ==================== */
/**
 * @brief  采集全部传感器，数据写入 g_sensorData
 *         各传感器容错：单个失败不影响其他
 */
void APP_SampleSensors(void)
{
    float val;

    /* DHT22 */
    if (DHT22_Read(&g_sensorData.temperature, &g_sensorData.humidity) != DHT22_OK) {
        /* 读数失败保持上次值，不填0（避免报警误触发） */
    }

    /* BH1750 */
    if (BH1750_ReadLight(&val) == 0) {
        g_sensorData.light = val;
    }

    /* HC-SR04 */
    if (SR04_Measure(&val) == 0) {
        g_sensorData.distance = val;
    }

    /* WiFi 状态 */
    g_sensorData.wifi_connected = ESP8266_IsConnected();
}

/* ==================== 显示更新 ==================== */
void APP_UpdateDisplay(void)
{
    OLED_Clear();

    switch (currentPage) {
        case PAGE_MAIN:
            Menu_ShowMain(&g_sensorData);
            break;
        case PAGE_DETAIL:
            Menu_ShowDetail(&g_sensorData);
            break;
        case PAGE_SETTINGS:
            Menu_ShowSettings();
            break;
        case PAGE_WIFI:
            Menu_ShowWiFi(&g_sensorData);
            break;
        case PAGE_ABOUT:
            Menu_ShowAbout();
            break;
        default:
            break;
    }

    OLED_Refresh();

    /* 处理按键事件（每次刷新屏幕时检查） */
    APP_KeyHandler();
}

/* ==================== 按键处理 ==================== */
void APP_KeyHandler(void)
{
    Button_Event_t evt_page    = Button_GetEvent(BTN_PAGE);
    Button_Event_t evt_confirm = Button_GetEvent(BTN_CONFIRM);
    Button_Event_t evt_back    = Button_GetEvent(BTN_BACK);

    /* 按键1: 页面切换（短按循环） */
    if (evt_page == BTN_EVENT_SHORT) {
        currentPage = (currentPage + 1) % PAGE_COUNT;
    }

    /* 按键2: 确认（不同页面不同行为） */
    if (evt_confirm == BTN_EVENT_SHORT) {
        switch (currentPage) {
            case PAGE_WIFI:
                /* WiFi 页面按确认 → 重连 */
                ESP8266_ConnectWiFi();
                break;
            case PAGE_SETTINGS:
                /* 设置页面预留: 切换设置项 */
                break;
            default:
                break;
        }
    }

    /* 按键2: 长按 → 回到主页 */
    if (evt_confirm == BTN_EVENT_LONG) {
        currentPage = PAGE_MAIN;
    }

    /* 按键3: 返回主页 */
    if (evt_back == BTN_EVENT_SHORT) {
        currentPage = PAGE_MAIN;
    }
}

/* ==================== 云端上送 ==================== */
void APP_UploadToCloud(void)
{
    if (!ESP8266_IsConnected()) {
        /* WiFi 断了，尝试重连 */
        ESP8266_ConnectWiFi();
        return;
    }

    ESP8266_SendData(
        g_sensorData.temperature,
        g_sensorData.humidity,
        g_sensorData.light,
        g_sensorData.distance
    );
}
