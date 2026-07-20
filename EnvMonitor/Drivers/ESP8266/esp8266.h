/**
 * @file    esp8266.h
 * @brief   ESP8266 WiFi 模块驱动 �?UART AT 指令�? */

#ifndef ESP8266_H
#define ESP8266_H

#include "stm32f1xx_hal.h"

/* WiFi 配置（根据你的环境修改） */
#define WIFI_SSID       "YOUR_WIFI_NAME"
#define WIFI_PASSWORD   "YOUR_WIFI_PASSWORD"

/* Thingspeak 配置（免�?IoT 云平台） */
#define THINGSPEAK_API_KEY  "YOUR_API_KEY"

typedef enum {
    ESP_OK = 0,
    ESP_ERR_TIMEOUT,
    ESP_ERR_BUSY,
    ESP_ERR_CMD,
} ESP_Status_t;

void ESP8266_Init(void);
ESP_Status_t ESP8266_ConnectWiFi(void);
ESP_Status_t ESP8266_SendData(float temp, float humi, float light, float dist);
uint8_t ESP8266_IsConnected(void);

#endif
