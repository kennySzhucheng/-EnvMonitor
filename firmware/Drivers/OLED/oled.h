/**
 * @file    oled.h
 * @brief   0.96�?OLED SSD1306 I2C 驱动
 */

#ifndef OLED_H
#define OLED_H

#include "stm32f1xx_hal.h"

#define OLED_ADDR       0x78        /* 8�?I2C 地址 */
#define OLED_WIDTH      128
#define OLED_HEIGHT     64
#define OLED_PAGES      8           /* 64/8 = 8�?*/

void OLED_Init(void);
void OLED_Clear(void);
void OLED_Refresh(void);                            /* 将缓冲区刷到屏幕 */
void OLED_SetCursor(uint8_t x, uint8_t page);
void OLED_ShowChar(uint8_t x, uint8_t page, char ch);
void OLED_ShowString(uint8_t x, uint8_t page, const char *str, uint8_t highlight);
void OLED_ShowNum(uint8_t x, uint8_t page, int32_t num, uint8_t len);
void OLED_ShowFloat(uint8_t x, uint8_t page, float num, uint8_t intLen, uint8_t decLen);
void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color);
void OLED_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1);
void OLED_DrawProgressBar(uint8_t x, uint8_t page, uint8_t width, uint8_t percent);

#endif
