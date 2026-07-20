/**
 * @file    oled.c
 * @brief   OLED SSD1306 I2C 驱动实现
 * @note    使用页寻址模式，128×64 像素，8页×128列
 *          I2C 写格式: [控制字节 0x40=数据] + [数据字节...]
 */

#include "main.h"
#include "oled.h"
#include "fonts.h"

static uint8_t OLED_Buffer[OLED_PAGES][OLED_WIDTH];  /* 显存缓冲区 */

/* ==================== I2C 底层通信 ==================== */

static void OLED_WriteCmd(uint8_t cmd)
{
    uint8_t buf[2] = { 0x00, cmd };     /* 0x00 = 命令模式 */
    HAL_I2C_Master_Transmit(&I2C_BUS, OLED_ADDR, buf, 2, 10);
}

static void OLED_WriteData(uint8_t data)
{
    uint8_t buf[2] = { 0x40, data };    /* 0x40 = 数据模式 */
    HAL_I2C_Master_Transmit(&I2C_BUS, OLED_ADDR, buf, 2, 10);
}

/* ==================== 初始化序列 ==================== */
void OLED_Init(void)
{
    HAL_Delay(100);     /* 等待 OLED 上电稳定 */

    OLED_WriteCmd(0xAE);    /* 关闭显示 */
    OLED_WriteCmd(0x20);    /* 设置内存寻址模式 */
    OLED_WriteCmd(0x02);    /* 页寻址模式 */
    OLED_WriteCmd(0xB0);    /* 起始页 */
    OLED_WriteCmd(0xC8);    /* COM 扫描方向: 从下到上 */
    OLED_WriteCmd(0x00);    /* 低列地址 */
    OLED_WriteCmd(0x10);    /* 高列地址 */
    OLED_WriteCmd(0x40);    /* 起始行 */
    OLED_WriteCmd(0x81);    /* 对比度 */
    OLED_WriteCmd(0x7F);    /* 对比度值 (0-255) */
    OLED_WriteCmd(0xA1);    /* 段重映射: 左右反一下 */
    OLED_WriteCmd(0xA6);    /* 正常显示(非反白) */
    OLED_WriteCmd(0xA8);    /* 多路复用比 */
    OLED_WriteCmd(0x3F);    /* 1/64 duty */
    OLED_WriteCmd(0xA4);    /* 全屏显示关闭(正常) */
    OLED_WriteCmd(0xD3);    /* 显示偏移 */
    OLED_WriteCmd(0x00);    /* 不偏移 */
    OLED_WriteCmd(0xD5);    /* 显示时钟分频 */
    OLED_WriteCmd(0x80);    /* 默认值 */
    OLED_WriteCmd(0xD9);    /* 预充电周期 */
    OLED_WriteCmd(0xF1);    /* Phase1=15, Phase2=1 */
    OLED_WriteCmd(0xDA);    /* COM 引脚配置 */
    OLED_WriteCmd(0x12);    /* 顺序, 禁COM左右交换 */
    OLED_WriteCmd(0xDB);    /* VCOMH 电压 */
    OLED_WriteCmd(0x40);
    OLED_WriteCmd(0x8D);    /* 电荷泵 */
    OLED_WriteCmd(0x14);    /* 打开电荷泵 */
    OLED_WriteCmd(0xAF);    /* 打开显示 */

    OLED_Clear();
    OLED_Refresh();
}

/* ==================== 缓冲区操作 ==================== */

void OLED_Clear(void)
{
    memset(OLED_Buffer, 0x00, sizeof(OLED_Buffer));
}

void OLED_Refresh(void)
{
    for (uint8_t page = 0; page < OLED_PAGES; page++) {
        OLED_WriteCmd(0xB0 + page);     /* 设置页 */
        OLED_WriteCmd(0x00);            /* 低列地址 */
        OLED_WriteCmd(0x10);            /* 高列地址 */
        /* 批量写一整页 128字节 */
        for (uint8_t col = 0; col < OLED_WIDTH; col++) {
            OLED_WriteData(OLED_Buffer[page][col]);
        }
    }
}

/* ==================== 像素操作 ==================== */

void OLED_DrawPixel(uint8_t x, uint8_t y, uint8_t color)
{
    if (x >= OLED_WIDTH || y >= OLED_HEIGHT) return;
    uint8_t page = y / 8;
    uint8_t bit = y % 8;
    if (color) {
        OLED_Buffer[page][x] |= (1 << bit);
    } else {
        OLED_Buffer[page][x] &= ~(1 << bit);
    }
}

/* ==================== 字符和字符串显示 ==================== */

void OLED_ShowChar(uint8_t x, uint8_t page, char ch)
{
    if (x > OLED_WIDTH - 6 || page >= OLED_PAGES) return;
    if (ch < 0x20 || ch > 0x7E) ch = ' ';   /* 不可打印字符 → 空格 */
    uint8_t idx = ch - 0x20;
    for (uint8_t i = 0; i < 6; i++) {
        OLED_Buffer[page][x + i] = Font5x7[idx][i];
    }
}

void OLED_ShowString(uint8_t x, uint8_t page, const char *str, uint8_t highlight)
{
    while (*str) {
        if (highlight) {
            /* 高亮：反白显示 */
            OLED_ShowChar(x, page, *str);
            for (uint8_t i = 0; i < 6; i++) {
                OLED_Buffer[page][x + i] = ~OLED_Buffer[page][x + i];
            }
        } else {
            OLED_ShowChar(x, page, *str);
        }
        x += 6;
        if (x > OLED_WIDTH - 6) break;  /* 超出屏幕 */
        str++;
    }
}

/* 显示数字（右对齐补齐空格） */
void OLED_ShowNum(uint8_t x, uint8_t page, int32_t num, uint8_t len)
{
    char buf[12];
    snprintf(buf, sizeof(buf), "%*ld", len, (long)num);
    OLED_ShowString(x, page, buf, 0);
}

/* 显示浮点数，如 26.5 */
void OLED_ShowFloat(uint8_t x, uint8_t page, float num, uint8_t intLen, uint8_t decLen)
{
    char buf[16];
    snprintf(buf, sizeof(buf), "%*.*f", intLen + decLen + 1, decLen, num);
    OLED_ShowString(x, page, buf, 0);
}

/* ==================== 图形绘制 ==================== */

void OLED_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1)
{
    int16_t dx = (int16_t)x1 - x0;
    int16_t dy = (int16_t)y1 - y0;
    int16_t sx = (dx > 0) ? 1 : -1;
    int16_t sy = (dy > 0) ? 1 : -1;
    dx = (dx > 0) ? dx : -dx;
    dy = (dy > 0) ? dy : -dy;
    int16_t err = dx - dy;

    while (1) {
        OLED_DrawPixel(x0, y0, 1);
        if (x0 == x1 && y0 == y1) break;
        int16_t e2 = 2 * err;
        if (e2 > -dy) { err -= dy; x0 += sx; }
        if (e2 < dx)  { err += dx; y0 += sy; }
    }
}

/* 进度条 */
void OLED_DrawProgressBar(uint8_t x, uint8_t page, uint8_t width, uint8_t percent)
{
    if (percent > 100) percent = 100;
    uint8_t fill = (uint8_t)((uint16_t)width * percent / 100);

    /* 空槽（底部一行） */
    for (uint8_t i = 0; i < width; i++) {
        OLED_DrawPixel(x + i, page * 8 + 7, 1);
    }
    /* 填充 */
    for (uint8_t i = 0; i < fill; i++) {
        for (uint8_t h = 0; h < 6; h++) {
            OLED_DrawPixel(x + i, page * 8 + 1 + h, 1);
        }
    }
}
