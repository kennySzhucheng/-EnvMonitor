/**
 * @file    menu.c
 * @brief   OLED 5个页面的渲染实现
 * @note    128×64 像素，5×7字体，每行8像素，共8页
 *          ┌──────────────┐ 页0: 标题
 *          │  标题        │ 页1-2: 主内容
 *          │  内容区      │ 页7: 页脚（按键提示）
 *          │              │
 *          │  页脚提示    │
 *          └──────────────┘
 */

#include "main.h"
#include "menu.h"

/* ==================== 主页 ==================== */
void Menu_ShowMain(const SensorData_t *data)
{
    /* 第0行：标题 */
    OLED_ShowString(0, 0, "=== ENV MONITOR ===", 0);

    /* 第2-3行：温湿度 */
    OLED_ShowString(0, 2, "T:", 0);
    OLED_ShowFloat(12, 2, data->temperature, 2, 1);
    OLED_ShowString(48, 2, "C", 0);

    OLED_ShowString(66, 2, "H:", 0);
    OLED_ShowFloat(78, 2, data->humidity, 2, 1);
    OLED_ShowString(114, 2, "%", 0);

    /* 第4行：光照 */
    OLED_ShowString(0, 4, "Light:", 0);
    OLED_ShowFloat(36, 4, data->light, 5, 1);
    OLED_ShowString(96, 4, "lx", 0);

    /* 第5行：距离 */
    OLED_ShowString(0, 5, "Dist:", 0);
    OLED_ShowFloat(30, 5, data->distance, 4, 1);
    OLED_ShowString(78, 5, "cm", 0);

    /* 第7行：页脚（WiFi状态 + 页面指示） */
    if (data->wifi_connected) {
        OLED_ShowString(0, 7, "WiFi:OK", 0);
    } else {
        OLED_ShowString(0, 7, "WiFi:--", 0);
    }
    OLED_ShowString(78, 7, "[1/5]", 0);
}

/* ==================== 详情页：数值 + 简单条形图 ==================== */
void Menu_ShowDetail(const SensorData_t *data)
{
    OLED_ShowString(0, 0, "--- Details ---", 0);

    /* 温度条（页2-3，范围 0-50°C） */
    OLED_ShowString(0, 2, "T", 0);
    uint8_t t_percent = (uint8_t)((data->temperature / 50.0f) * 100);
    OLED_DrawProgressBar(10, 2, 100, t_percent);

    /* 湿度条（页3-4） */
    OLED_ShowString(0, 4, "H", 0);
    uint8_t h_percent = (uint8_t)data->humidity;  /* 0-100% 直映 */
    OLED_DrawProgressBar(10, 4, 100, h_percent);

    /* 光照数值 */
    OLED_ShowString(0, 6, "Lx:", 0);
    OLED_ShowFloat(20, 6, data->light, 5, 1);

    OLED_ShowString(78, 7, "[2/5]", 0);
}

/* ==================== 设置页：报警阈值 ==================== */
void Menu_ShowSettings(void)
{
    OLED_ShowString(0, 0, "--- Settings ---", 0);
    OLED_ShowString(0, 2, "Temp Alrm: >", 0);
    OLED_ShowFloat(72, 2, ALARM_TEMP_HIGH, 2, 1);
    OLED_ShowString(108, 2, "C", 0);

    OLED_ShowString(0, 4, "Humi Alrm: <", 0);
    OLED_ShowFloat(72, 4, ALARM_HUMI_LOW, 2, 1);
    OLED_ShowString(108, 4, "%", 0);

    OLED_ShowString(0, 6, "Dist Alrm: <", 0);
    OLED_ShowFloat(72, 6, ALARM_DIST_CLOSE, 2, 1);
    OLED_ShowString(108, 6, "cm", 0);

    OLED_ShowString(78, 7, "[3/5]", 0);
}

/* ==================== WiFi 页面 ==================== */
void Menu_ShowWiFi(const SensorData_t *data)
{
    OLED_ShowString(0, 0, "--- WiFi ---", 0);

    OLED_ShowString(0, 2, "SSID:", 0);
    OLED_ShowString(30, 2, WIFI_SSID, 0);

    OLED_ShowString(0, 4, "Status:", 0);
    if (data->wifi_connected) {
        OLED_ShowString(48, 4, "Connected", 0);
    } else {
        OLED_ShowString(48, 4, "Disconnected", 0);
    }

    OLED_ShowString(0, 6, "[OK] Reconnect", 0);

    OLED_ShowString(78, 7, "[4/5]", 0);
}

/* ==================== 关于页 ==================== */
void Menu_ShowAbout(void)
{
    OLED_ShowString(0, 0, "--- About ---", 0);
    OLED_ShowString(0, 2, "Env Monitor v1.0", 0);
    OLED_ShowString(0, 3, "STM32F103C8T6", 0);
    OLED_ShowString(0, 4, "Sensors: DHT22", 0);
    OLED_ShowString(0, 5, "BH1750 HC-SR04", 0);
    OLED_ShowString(0, 6, "WiFi: ESP8266", 0);
    OLED_ShowString(78, 7, "[5/5]", 0);
}
