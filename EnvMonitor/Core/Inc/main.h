/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : 智能环境监测与物联网控制站 — 全局头文件
  ******************************************************************************
  */
/* USER CODE END Header */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* ==================== HAL 核心 ==================== */
#include "stm32f1xx_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* ==================== FreeRTOS ==================== */
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "timers.h"

/* ==================== 硬件引脚宏 ==================== */

/* --- I2C1: 共享总线 --- */
#define I2C_BUS                     hi2c1

/* --- DHT22 --- */
#define DHT22_PORT                  GPIOA
#define DHT22_PIN                   GPIO_PIN_0

/* --- HC-SR04 --- */
#define SR04_TRIG_PORT              GPIOB
#define SR04_TRIG_PIN               GPIO_PIN_0
#define SR04_ECHO_PORT              GPIOB
#define SR04_ECHO_PIN               GPIO_PIN_1
#define SR04_TIM                    (&htim3)
#define SR04_TIM_CHANNEL            TIM_CHANNEL_4

/* --- UART1: ESP8266 --- */
#define ESP_UART                    huart1

/* --- 按键 --- */
#define KEY1_PORT                   GPIOA
#define KEY1_PIN                    GPIO_PIN_4
#define KEY2_PORT                   GPIOA
#define KEY2_PIN                    GPIO_PIN_5
#define KEY3_PORT                   GPIOA
#define KEY3_PIN                    GPIO_PIN_6

/* --- LED --- */
#define LED_RED_PORT                GPIOB
#define LED_RED_PIN                 GPIO_PIN_12
#define LED_YELLOW_PORT             GPIOB
#define LED_YELLOW_PIN              GPIO_PIN_13
#define LED_GREEN_PORT              GPIOB
#define LED_GREEN_PIN               GPIO_PIN_14

/* --- 蜂鸣器 --- */
#define BUZZER_PORT                 GPIOB
#define BUZZER_PIN                  GPIO_PIN_15

/* ==================== 系统参数宏 ==================== */
#define SENSOR_SAMPLE_INTERVAL_MS   5000U
#define CLOUD_UPLOAD_INTERVAL_MS    60000U
#define OLED_REFRESH_INTERVAL_MS    1000U
#define ALARM_CHECK_INTERVAL_MS     500U

/* ==================== 报警阈值 ==================== */
#define ALARM_TEMP_HIGH             30.0f
#define ALARM_HUMI_LOW              30.0f
#define ALARM_DIST_CLOSE            20.0f

/* ==================== 传感器数据结构体（必须在驱动头文件之前定义） ==================== */
typedef struct {
    float temperature;
    float humidity;
    float light;
    float distance;
    uint8_t wifi_connected;
} SensorData_t;

/* ==================== 全局变量声明 ==================== */
extern SensorData_t g_sensorData;
extern volatile uint32_t g_sysTick;

extern I2C_HandleTypeDef  hi2c1;
extern UART_HandleTypeDef huart1;
extern TIM_HandleTypeDef  htim3;

/* ==================== 驱动层头文件 ==================== */
#include "oled.h"
#include "dht22.h"
#include "bh1750.h"
#include "hc_sr04.h"
#include "esp8266.h"
#include "button.h"
#include "buzzer.h"
#include "led.h"

/* ==================== 应用层头文件 ==================== */
#include "app.h"
#include "menu.h"
#include "alarm.h"

/* ==================== RTOS 任务头文件 ==================== */
#include "task_shared.h"
#include "task_button.h"
#include "task_buzzer.h"
#include "task_alarm.h"
#include "task_sensor.h"
#include "task_display.h"
#include "task_cloud.h"

/* ==================== 函数声明 ==================== */
void SystemClock_Config(void);
void Error_Handler(void);

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
