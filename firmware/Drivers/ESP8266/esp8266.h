/**
 * @file    esp8266.h
 * @brief   ESP8266 WiFi 模块驱动 — UART AT 指令
 */

#ifndef ESP8266_H
#define ESP8266_H

#include "stm32f1xx_hal.h"

/*
 * WiFi & ThingSpeak 配置
 *
 * 真实凭据在 secrets.h（已被 .gitignore 忽略，不上传 GitHub）。
 * 如果是新克隆的项目，复制 secrets_template.h → secrets.h 并填入你的信息。
 */
#include "secrets.h"

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
