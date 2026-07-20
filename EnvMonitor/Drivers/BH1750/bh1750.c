/**
 * @file    bh1750.c
 * @brief   BH1750 I2C 驱动实现
 * @note    BH1750 指令集:
 *           0x01: 上电
 *           0x07: 复位
 *           0x10: 连续高分辨率模式 (1lux精度, 120ms测量时间)
 *           0x20: 单次高分辨率模式 (上电后自动断电)
 *         数据格式: 2字节, 光照值 = (高字节<<8|低字节) / 1.2
 */

#include "main.h"
#include "bh1750.h"

void BH1750_Init(void)
{
    uint8_t cmd;

    /* 上电 */
    cmd = 0x01;
    HAL_I2C_Master_Transmit(&I2C_BUS, BH1750_ADDR, &cmd, 1, 10);

    /* 连续高分辨率模式 */
    cmd = 0x10;
    HAL_I2C_Master_Transmit(&I2C_BUS, BH1750_ADDR, &cmd, 1, 10);

    HAL_Delay(180);  /* 等待首次测量完成 */
}

uint8_t BH1750_ReadLight(float *lux)
{
    uint8_t buf[2] = {0};

    /* 读取 2 字节测量值 */
    if (HAL_I2C_Master_Receive(&I2C_BUS, BH1750_ADDR, buf, 2, 100) != HAL_OK) {
        return 1;   /* I2C 通信失败 */
    }

    uint16_t raw = ((uint16_t)buf[0] << 8) | buf[1];

    /* 转换: Lux = raw / 1.2 */
    *lux = (float)raw / 1.2f;

    return 0;   /* OK */
}
