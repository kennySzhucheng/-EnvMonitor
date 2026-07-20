/**
 * @file    hc_sr04.h
 * @brief   HC-SR04 超声波测距驱�? */

#ifndef HC_SR04_H
#define HC_SR04_H

#include "stm32f1xx_hal.h"

void SR04_Init(void);
uint8_t SR04_Measure(float *distance_cm);

#endif
