/**
 * @file    led.c
 * @brief   LED 驱动实现
 * @note    红: PB12 — 高温报警
 *          黄: PB13 — 低湿提示
 *          绿: PB14 — 靠近检测
 */

#include "main.h"
#include "led.h"

/* 引脚映射表 */
static GPIO_TypeDef* led_ports[LED_COUNT] = {
    LED_RED_PORT,    /* 或 GPIOB */
    LED_YELLOW_PORT,
    LED_GREEN_PORT
};

static uint16_t led_pins[LED_COUNT] = {
    LED_RED_PIN,     /* 或 GPIO_PIN_12 */
    LED_YELLOW_PIN,
    LED_GREEN_PIN
};

void LED_Init(void)
{
    LED_AllOff();
}

void LED_On(LED_Color_t color)
{
    if (color < LED_COUNT) {
        HAL_GPIO_WritePin(led_ports[color], led_pins[color], GPIO_PIN_SET);
    }
}

void LED_Off(LED_Color_t color)
{
    if (color < LED_COUNT) {
        HAL_GPIO_WritePin(led_ports[color], led_pins[color], GPIO_PIN_RESET);
    }
}

void LED_Toggle(LED_Color_t color)
{
    if (color < LED_COUNT) {
        HAL_GPIO_TogglePin(led_ports[color], led_pins[color]);
    }
}

void LED_AllOff(void)
{
    LED_Off(LED_RED);
    LED_Off(LED_YELLOW);
    LED_Off(LED_GREEN);
}
