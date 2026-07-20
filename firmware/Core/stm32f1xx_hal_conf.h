/**
 * @file    stm32f1xx_hal_conf.h
 * @brief   HAL 库配置 — 只开启本项目用到的外设模块
 * @note    未使用的模块注释掉，减小编译体积和固件大小
 */

#ifndef STM32F1XX_HAL_CONF_H
#define STM32F1XX_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f1xx.h"

/* ==================== 本项目使用的外设 ==================== */

#define HAL_MODULE_ENABLED          /* HAL 核心，必须开启 */
#define HAL_GPIO_MODULE_ENABLED     /* GPIO: LED、按键、蜂鸣器 */
#define HAL_I2C_MODULE_ENABLED      /* I2C1: OLED + BH1750 共用总线 */
#define HAL_UART_MODULE_ENABLED     /* USART1: ESP8266 通信 */
#define HAL_TIM_MODULE_ENABLED      /* TIM3: HC-SR04 输入捕获 + 系统时基 */
#define HAL_DMA_MODULE_ENABLED      /* DMA: UART 接收不定长数据（ESP8266） */
#define HAL_CORTEX_MODULE_ENABLED   /* NVIC 中断优先级配置 */
#define HAL_RCC_MODULE_ENABLED      /* 时钟配置 */
#define HAL_FLASH_MODULE_ENABLED    /* 内部 Flash 存储用户设置 */
#define HAL_PWR_MODULE_ENABLED      /* 低功耗管理 */

/* ==================== 本项目不使用的外设（注释掉省空间） ==================== */
// #define HAL_ADC_MODULE_ENABLED
// #define HAL_CAN_MODULE_ENABLED
// #define HAL_SPI_MODULE_ENABLED
// #define HAL_RTC_MODULE_ENABLED
// #define HAL_WWDG_MODULE_ENABLED
// #define HAL_IWDG_MODULE_ENABLED

/* ==================== 时钟频率 ==================== */
#define HSE_VALUE            8000000UL   /* 外部晶振 8MHz */
#define HSI_VALUE            8000000UL   /* 内部 RC 8MHz */
#define HCLK_VALUE          72000000UL   /* HCLK: 72MHz (最终系统时钟) */


/* ==================== HAL 模块详细配置 ==================== */

/* --- NVIC 优先级分组 --- */
#define TICK_INT_PRIORITY             15U   /* SysTick 最低优先级 */
#define USE_RTOS                       0U

/* --- UART --- */
#define USE_HAL_UART_REGISTER_CALLBACKS   1U  /* 启用回调注册 */

/* --- I2C --- */
#define USE_HAL_I2C_REGISTER_CALLBACKS    1U

/* --- Flash --- */
#define USE_HAL_FLASH_REGISTER_CALLBACKS  0U

/* --- 断言 --- */
#define USE_FULL_ASSERT                 1U   /* 开发阶段开启断言，发布时关 */

#ifdef USE_FULL_ASSERT
  #define assert_param(expr)  ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
  void assert_failed(uint8_t *file, uint32_t line);
#else
  #define assert_param(expr)  ((void)0U)
#endif

#ifdef __cplusplus
}
#endif

#endif /* STM32F1XX_HAL_CONF_H */
