/**
 * @file    esp8266.c
 * @brief   ESP8266 AT 指令驱动实现
 * @note    UART 通信: 115200-8-N-1
 *         主要 AT 指令:
 *           AT          → OK       (模块正常)
 *           AT+CWMODE=1 → OK       (Station 模式)
 *           AT+CWJAP    → OK       (连接 WiFi)
 *           AT+CIPSTART → OK       (建立 TCP 连接)
 *           AT+CIPSEND  → >        (发送数据)
 *
 *         MQTT 走 TCP 透传方式，或直接 HTTP GET 上送 Thingspeak
 *         本项目使用 HTTP GET 方式（更简单可靠）
 */

#include "main.h"
#include "esp8266.h"

static uint8_t wifi_connected = 0;

/* ==================== 底层：收/发 AT 指令 ==================== */

/**
 * @brief  发送 AT 指令
 */
static void ESP_SendCmd(const char *cmd)
{
    HAL_UART_Transmit(&ESP_UART, (uint8_t *)cmd, strlen(cmd), 1000);
}

/**
 * @brief  发送 AT 指令并等待响应
 * @param  cmd      AT 指令字符串 (不含\r\n)
 * @param  expect   期望的响应关键词
 * @param  timeout_ms 超时(ms)
 * @return ESP_OK / ESP_ERR_TIMEOUT / ESP_ERR_CMD
 * @note   接收缓冲区 512 字节，超时返回
 */
static ESP_Status_t ESP_SendCmdWait(const char *cmd, const char *expect, uint32_t timeout_ms)
{
    uint8_t buf[512] = {0};
    uint16_t idx = 0;
    uint32_t deadline = g_sysTick + timeout_ms;

    /* 发送: cmd\r\n */
    ESP_SendCmd(cmd);
    ESP_SendCmd("\r\n");

    /* 等待响应 */
    while (g_sysTick < deadline) {
        if (__HAL_UART_GET_FLAG(&ESP_UART, UART_FLAG_RXNE)) {
            uint8_t ch = (uint8_t)(ESP_UART.Instance->DR & 0xFF);
            if (idx < sizeof(buf) - 1) {
                buf[idx++] = ch;
                buf[idx] = '\0';
            }
            /* 检查是否收到期望字符串 */
            if (strstr((char *)buf, expect) != NULL) {
                return ESP_OK;
            }
            /* 检查是否出错 */
            if (strstr((char *)buf, "ERROR") != NULL) {
                return ESP_ERR_CMD;
            }
        }
    }
    return ESP_ERR_TIMEOUT;
}

/**
 * @brief  清空 UART 接收缓冲区
 */
static void ESP_FlushRx(void)
{
    while (__HAL_UART_GET_FLAG(&ESP_UART, UART_FLAG_RXNE)) {
        (void)(ESP_UART.Instance->DR & 0xFF);
    }
}

/* ==================== 初始化序列 ==================== */

void ESP8266_Init(void)
{
    wifi_connected = 0;

    /* 等待模块稳定 */
    HAL_Delay(1000);

    /* AT 测试 */
    ESP_FlushRx();
    if (ESP_SendCmdWait("AT", "OK", 2000) != ESP_OK) {
        /* 模块无响应，可能波特率不对或模块未上电 */
        return;
    }

    /* 关闭回显 */
    ESP_SendCmdWait("ATE0", "OK", 1000);

    /* 设置 Station 模式 */
    ESP_SendCmdWait("AT+CWMODE=1", "OK", 1000);
}

/**
 * @brief  连接 WiFi
 * @return ESP_OK 连接成功
 */
ESP_Status_t ESP8266_ConnectWiFi(void)
{
    char cmd[128];

    /* 如果已连接，先断开 */
    ESP_SendCmdWait("AT+CWQAP", "OK", 2000);
    HAL_Delay(500);

    /* 连接 WiFi: AT+CWJAP="ssid","password" */
    snprintf(cmd, sizeof(cmd), "AT+CWJAP=\"%s\",\"%s\"", WIFI_SSID, WIFI_PASSWORD);
    ESP_FlushRx();

    /* WiFi 连接可能需要 5-10 秒 */
    if (ESP_SendCmdWait(cmd, "OK", 15000) == ESP_OK) {
        wifi_connected = 1;
        /* 设为单连接模式 */
        ESP_SendCmdWait("AT+CIPMUX=0", "OK", 1000);
        return ESP_OK;
    }
    wifi_connected = 0;
    return ESP_ERR_TIMEOUT;
}

uint8_t ESP8266_IsConnected(void)
{
    return wifi_connected;
}

/**
 * @brief  通过 HTTP GET 上送数据到 Thingspeak
 * @note   Thingspeak API:
 *         GET /update?api_key=XXX&field1=temp&field2=humi&field3=light&field4=dist
 *         Host: api.thingspeak.com
 *
 *         TCP 透传流程:
 *           1. AT+CIPSTART="TCP","api.thingspeak.com",80
 *           2. AT+CIPSEND=<长度>
 *           3. 发送 HTTP GET 请求
 *           4. 关闭连接
 */
ESP_Status_t ESP8266_SendData(float temp, float humi, float light, float dist)
{
    char buffer[512];
    uint16_t len;
    ESP_Status_t status;

    if (!wifi_connected) {
        return ESP_ERR_BUSY;
    }

    /* 1. 建立 TCP 连接到 Thingspeak */
    status = ESP_SendCmdWait("AT+CIPSTART=\"TCP\",\"api.thingspeak.com\",80", "OK", 5000);
    if (status != ESP_OK) return status;

    /* 2. 构造 HTTP GET 请求 */
    snprintf(buffer, sizeof(buffer),
        "GET /update?api_key=%s&field1=%.1f&field2=%.1f&field3=%.1f&field4=%.1f HTTP/1.1\r\n"
        "Host: api.thingspeak.com\r\n"
        "Connection: close\r\n"
        "\r\n",
        THINGSPEAK_API_KEY, temp, humi, light, dist);

    len = strlen(buffer);

    /* 3. 发送数据长度指令 */
    char cmdLen[32];
    snprintf(cmdLen, sizeof(cmdLen), "AT+CIPSEND=%d", len);
    ESP_FlushRx();
    status = ESP_SendCmdWait(cmdLen, ">", 2000);
    if (status != ESP_OK) {
        /* 关闭连接 */
        ESP_SendCmdWait("AT+CIPCLOSE", "OK", 1000);
        return status;
    }

    /* 4. 发送 HTTP 数据 */
    ESP_SendCmd(buffer);

    /* 等待发送成功 */
    status = ESP_SendCmdWait("", "SEND OK", 5000);
    if (status != ESP_OK) {
        ESP_SendCmdWait("AT+CIPCLOSE", "OK", 1000);
        return status;
    }

    /* 5. 关闭 TCP 连接 */
    ESP_SendCmdWait("AT+CIPCLOSE", "OK", 1000);

    return ESP_OK;
}
