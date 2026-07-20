/**
 * @file    button.h
 * @brief   按键驱动 �?软件消抖 + 长短按检�? */

#ifndef BUTTON_H
#define BUTTON_H

#include "stm32f1xx_hal.h"

typedef enum {
    BTN_PAGE = 0,       /* 页面切换 */
    BTN_CONFIRM,        /* 确认 */
    BTN_BACK,           /* 返回 */
    BTN_COUNT
} Button_ID_t;

typedef enum {
    BTN_EVENT_NONE = 0,
    BTN_EVENT_SHORT,    /* 短按 (<1s) */
    BTN_EVENT_LONG,     /* 长按 (>1s) */
    BTN_EVENT_HOLD,     /* 持续按住 (>2s 后每 200ms 重复) */
} Button_Event_t;

void Button_Init(void);
void Button_Scan(void);                         /* 主循环中调用 */
Button_Event_t Button_GetEvent(Button_ID_t id);  /* 获取事件（读后清除） */
uint8_t Button_IsPressed(Button_ID_t id);        /* 是否正在按下 */

#endif
