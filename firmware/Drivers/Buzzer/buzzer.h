/**
 * @file    buzzer.h
 * @brief   有源蜂鸣器驱�?�?GPIO 控制即可
 */

#ifndef BUZZER_H
#define BUZZER_H

#include "stm32f1xx_hal.h"

void Buzzer_Init(void);
void Buzzer_On(void);
void Buzzer_Off(void);
void Buzzer_Beep(uint16_t duration_ms);
void Buzzer_BeepPattern(uint8_t times, uint16_t on_ms, uint16_t off_ms);
void Buzzer_Tick(void);

#endif
