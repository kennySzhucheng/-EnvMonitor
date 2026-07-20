/**
 * @file    buzzer.c
 * @brief   有源蜂鸣器驱动实现
 * @note    PB15 控制，有源蜂鸣器通电即响
 *          STM32 IO 口通过 S8050 NPN 三极管驱动
 */

#include "main.h"
#include "buzzer.h"

static uint8_t  buzzer_ringing = 0;            /* 是否正在响 */
static uint32_t buzzer_stopTick = 0;           /* 停止时刻 */
static uint8_t  buzzer_pattern_active = 0;     /* 模式激活 */
static uint8_t  buzzer_pattern_times = 0;
static uint16_t buzzer_pattern_on = 0;
static uint16_t buzzer_pattern_off = 0;
static uint8_t  buzzer_phase = 0;              /* 0=响, 1=停 */
static uint32_t buzzer_phaseTick = 0;

void Buzzer_Init(void)
{
    Buzzer_Off();
}

void Buzzer_On(void)
{
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_SET);
}

void Buzzer_Off(void)
{
    HAL_GPIO_WritePin(BUZZER_PORT, BUZZER_PIN, GPIO_PIN_RESET);
}

/**
 * @brief  短鸣一声
 * @param  duration_ms  鸣叫时长（毫秒）
 */
void Buzzer_Beep(uint16_t duration_ms)
{
    Buzzer_On();
    buzzer_ringing = 1;
    buzzer_stopTick = g_sysTick + duration_ms;
    buzzer_pattern_active = 0;
}

/**
 * @brief  蜂鸣器模式：响N声
 * @param  times   响几声
 * @param  on_ms   每次响多久
 * @param  off_ms  每次停多久
 * @note   需在主循环中调用 Buzzer_Tick() 来驱动
 */
void Buzzer_BeepPattern(uint8_t times, uint16_t on_ms, uint16_t off_ms)
{
    buzzer_pattern_active = 1;
    buzzer_pattern_times = times;
    buzzer_pattern_on = on_ms;
    buzzer_pattern_off = off_ms;
    buzzer_phase = 0;
    Buzzer_On();
    buzzer_phaseTick = g_sysTick + on_ms;
}

/**
 * @brief  蜂鸣器状态机 — 主循环中每轮调用
 */
void Buzzer_Tick(void)
{
    /* 单次鸣叫超时 */
    if (buzzer_ringing && !buzzer_pattern_active) {
        if (g_sysTick >= buzzer_stopTick) {
            Buzzer_Off();
            buzzer_ringing = 0;
        }
    }

    /* 模式鸣叫 */
    if (buzzer_pattern_active) {
        if (g_sysTick >= buzzer_phaseTick) {
            if (buzzer_phase == 0) {
                /* 当前在响 → 停 */
                Buzzer_Off();
                buzzer_phase = 1;
                buzzer_phaseTick = g_sysTick + buzzer_pattern_off;
            } else {
                /* 当前在停 */
                buzzer_pattern_times--;
                if (buzzer_pattern_times == 0) {
                    /* 全部完成 */
                    buzzer_pattern_active = 0;
                    return;
                }
                Buzzer_On();
                buzzer_phase = 0;
                buzzer_phaseTick = g_sysTick + buzzer_pattern_on;
            }
        }
    }
}
