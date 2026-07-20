/**
 * @file    button.c
 * @brief   按键驱动实现 — 状态机消抖 + 长短按检测
 * @note    按键接 PA4/PA5/PA6，内部上拉，按下读到低电平
 *          ┌───────────────┐
 *          │  释放    →   按下
 *          │  ↑           ↓
 *          │  确认 ← 消抖等待
 *          └───────────────┘
 */

#include "main.h"
#include "button.h"

#define DEBOUNCE_MS         30      /* 消抖时间 */
#define LONG_PRESS_MS       1000    /* 长按阈值 */
#define HOLD_REPEAT_MS      200     /* 长按连发间隔 */

typedef enum {
    BTN_STATE_IDLE = 0,     /* 释放状态 */
    BTN_STATE_DEBOUNCE,     /* 消抖等待 */
    BTN_STATE_PRESSED,      /* 已确认按下 */
    BTN_STATE_LONG,         /* 长按状态 */
} Button_State_t;

typedef struct {
    Button_ID_t     id;
    GPIO_TypeDef   *port;
    uint16_t        pin;
    Button_State_t  state;
    uint32_t        pressTime;      /* 按下时刻 */
    uint32_t        lastHoldTime;   /* 上次连发时刻 */
    Button_Event_t  event;          /* 待读取事件 */
} Button_t;

static Button_t buttons[BTN_COUNT] = {
    { BTN_PAGE,     KEY1_PORT, KEY1_PIN, BTN_STATE_IDLE, 0, 0, BTN_EVENT_NONE },
    { BTN_CONFIRM,  KEY2_PORT, KEY2_PIN, BTN_STATE_IDLE, 0, 0, BTN_EVENT_NONE },
    { BTN_BACK,     KEY3_PORT, KEY3_PIN, BTN_STATE_IDLE, 0, 0, BTN_EVENT_NONE },
};

void Button_Init(void)
{
    /* GPIO 已在 GPIO_Init_All() 中配置为内部上拉输入 */
}

/**
 * @brief  按键扫描 — 每轮主循环调用一次
 *         采用"多实例状态机"模式，每个按键独立状态
 */
void Button_Scan(void)
{
    for (uint8_t i = 0; i < BTN_COUNT; i++) {
        Button_t *btn = &buttons[i];
        uint8_t is_down = (HAL_GPIO_ReadPin(btn->port, btn->pin) == GPIO_PIN_RESET);

        switch (btn->state) {

            case BTN_STATE_IDLE:
                if (is_down) {
                    btn->state = BTN_STATE_DEBOUNCE;
                    btn->pressTime = g_sysTick;
                }
                break;

            case BTN_STATE_DEBOUNCE:
                if (is_down) {
                    /* 消抖时间到 → 确认按下 */
                    if (g_sysTick - btn->pressTime >= DEBOUNCE_MS) {
                        btn->state = BTN_STATE_PRESSED;
                        btn->event = BTN_EVENT_SHORT;   /* 先产生短按事件 */
                        btn->pressTime = g_sysTick;     /* 记录按下时刻供长按判断 */
                    }
                } else {
                    /* 消抖期间松开了，视为抖动，回到空闲 */
                    btn->state = BTN_STATE_IDLE;
                }
                break;

            case BTN_STATE_PRESSED:
                if (!is_down) {
                    /* 松开了 → 回到空闲 */
                    btn->state = BTN_STATE_IDLE;
                } else {
                    /* 按够1秒 → 进长按 */
                    if (g_sysTick - btn->pressTime >= LONG_PRESS_MS) {
                        btn->state = BTN_STATE_LONG;
                        btn->event = BTN_EVENT_LONG;
                        btn->lastHoldTime = g_sysTick;
                    }
                }
                break;

            case BTN_STATE_LONG:
                if (!is_down) {
                    /* 长按后松开 → 回到空闲 */
                    btn->state = BTN_STATE_IDLE;
                } else {
                    /* 持续按住 → 每200ms产生一次HOLD事件 */
                    if (g_sysTick - btn->lastHoldTime >= HOLD_REPEAT_MS) {
                        btn->event = BTN_EVENT_HOLD;
                        btn->lastHoldTime = g_sysTick;
                    }
                }
                break;
        }
    }
}

/**
 * @brief  获取按键事件（读后自动清除）
 * @return 按键事件类型
 */
Button_Event_t Button_GetEvent(Button_ID_t id)
{
    Button_Event_t evt = buttons[id].event;
    buttons[id].event = BTN_EVENT_NONE;
    return evt;
}

/**
 * @brief  按键是否正在按下
 */
uint8_t Button_IsPressed(Button_ID_t id)
{
    return (HAL_GPIO_ReadPin(buttons[id].port, buttons[id].pin) == GPIO_PIN_RESET);
}
