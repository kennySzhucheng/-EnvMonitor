/**
 * @file    menu.h
 * @brief   OLED 菜单页面渲染
 */

#ifndef MENU_H
#define MENU_H

#include "stm32f1xx_hal.h"

void Menu_ShowMain(const SensorData_t *data);
void Menu_ShowDetail(const SensorData_t *data);
void Menu_ShowSettings(void);
void Menu_ShowWiFi(const SensorData_t *data);
void Menu_ShowAbout(void);

#endif
