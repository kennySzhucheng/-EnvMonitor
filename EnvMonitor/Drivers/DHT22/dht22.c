/**
 * @file    dht22.c
 * @brief   DHT22 单总线驱动实现
 * @note    DHT22 数据格式: 40bit = 湿度高8 + 湿度低8 + 温度高8 + 温度低8 + 校验8
 *         通讯时序（主机发起）:
 *           1. 主机拉低 >1ms 启动信号
 *           2. 主机释放总线，等待 DHT22 响应
 *           3. DHT22 拉低 80us，再拉高 80us 表示就绪
 *           4. DHT22 连续发送 40bit 数据
 *              - 每bit以 50us 低电平开始
 *              - 高电平 26-28us = 0,  70us = 1
 *         ⚠️ 写这段代码需要关中断，防止时序被打断
 */

#include "main.h"
#include "dht22.h"

static void DHT22_SetOutput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT22_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(DHT22_PORT, &GPIO_InitStruct);
}

static void DHT22_SetInput(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    GPIO_InitStruct.Pin = DHT22_PIN;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    HAL_GPIO_Init(DHT22_PORT, &GPIO_InitStruct);
}

#define DHT22_DELAY_US(us)  do { \
    for (volatile uint32_t _d = 0; _d < (us) * 12; _d++) { __NOP(); } \
} while(0)


void DHT22_Init(void)
{
    DHT22_SetOutput();
    HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_SET);  /* 总线空闲 = 高 */
}

/**
 * @brief  读取一次 DHT22 数据
 * @param  temperature  输出: 温度(°C)
 * @param  humidity     输出: 湿度(%)
 * @return DHT22_OK / DHT22_ERR_CHECK / DHT22_ERR_TIMEO
 */
uint8_t DHT22_Read(float *temperature, float *humidity)
{
    uint8_t data[5] = {0};
    uint16_t timeout = 0;

    __disable_irq();  /* ⚠️ 关中断，时序不能被打断 */

    /* --- 1. 主机发送启动信号: 拉低 >1ms --- */
    DHT22_SetOutput();
    HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_RESET);
    HAL_Delay(2);   /* 拉低 2ms（安全余量） */
    HAL_GPIO_WritePin(DHT22_PORT, DHT22_PIN, GPIO_PIN_SET);
    DHT22_DELAY_US(30);  /* 拉高 30us 后释放 */

    /* --- 2. 切换到输入，等待 DHT22 响应 --- */
    DHT22_SetInput();

    /* 等待 DHT22 拉低（响应信号开始，应在 20-40us 内出现） */
    timeout = 0;
    while (HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN) == GPIO_PIN_SET) {
        if (++timeout > 500) { __enable_irq(); return DHT22_ERR_TIMEO; }
    }

    /* 等待 DHT22 拉高（响应信号结束） */
    timeout = 0;
    while (HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN) == GPIO_PIN_RESET) {
        if (++timeout > 500) { __enable_irq(); return DHT22_ERR_TIMEO; }
    }
    timeout = 0;
    while (HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN) == GPIO_PIN_SET) {
        if (++timeout > 500) { __enable_irq(); return DHT22_ERR_TIMEO; }
    }

    /* --- 3. 读取 40bit 数据 --- */
    for (uint8_t i = 0; i < 5; i++) {
        for (uint8_t j = 0; j < 8; j++) {
            /* 等待低电平结束（每bit以 ~50us 低电平开头） */
            timeout = 0;
            while (HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN) == GPIO_PIN_RESET) {
                if (++timeout > 500) { __enable_irq(); return DHT22_ERR_TIMEO; }
            }
            /* 测量高电平持续时间 */
            DHT22_DELAY_US(40);  /* 等 40us，如果还是高就是 bit=1 */
            data[i] <<= 1;
            if (HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN) == GPIO_PIN_SET) {
                data[i] |= 0x01;
            }
            /* 等待高电平结束 */
            timeout = 0;
            while (HAL_GPIO_ReadPin(DHT22_PORT, DHT22_PIN) == GPIO_PIN_SET) {
                if (++timeout > 500) { __enable_irq(); return DHT22_ERR_TIMEO; }
            }
        }
    }

    __enable_irq();  /* 恢复中断 */

    /* --- 4. 校验 --- */
    uint8_t checksum = data[0] + data[1] + data[2] + data[3];
    if (checksum != data[4]) {
        return DHT22_ERR_CHECK;
    }

    /* --- 5. 解析数据 --- */
    /* DHT22: 湿度 = (高字节<<8 | 低字节) / 10.0 */
    uint16_t hum_raw = ((uint16_t)data[0] << 8) | data[1];
    /* 温度: 最高位=1 表示负温 */
    uint16_t temp_raw = ((uint16_t)(data[2] & 0x7F) << 8) | data[3];

    *humidity = hum_raw / 10.0f;
    *temperature = (data[2] & 0x80) ? -(temp_raw / 10.0f) : (temp_raw / 10.0f);

    return DHT22_OK;
}
