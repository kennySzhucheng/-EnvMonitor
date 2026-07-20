/**
 * @file    app.h
 * @brief   应用层 — 页面枚举与共享类型定义
 * @note    各功能已迁移到 FreeRTOS 任务模块 (task_*.c)
 *          本文件仅保留跨任务共享的类型定义
 */

#ifndef APP_H
#define APP_H

#include "stm32f1xx_hal.h"

/* 页面枚举 */
typedef enum {
    PAGE_MAIN = 0,      /* 主页：温湿度+光照+距离概览 */
    PAGE_DETAIL,        /* 详情：进度条 */
    PAGE_SETTINGS,      /* 设置：报警阈值查看 */
    PAGE_WIFI,          /* WiFi：连接状态 + 手动重连 */
    PAGE_ABOUT,         /* 关于 */
    PAGE_COUNT
} App_Page_t;

void APP_Init(void);

#endif
