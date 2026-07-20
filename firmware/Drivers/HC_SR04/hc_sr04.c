/**
 * @file    hc_sr04.c
 * @brief   HC-SR04 超声波驱动实现
 * @note    工作原理:
 *           1. Trig 引脚给 >10us 高电平脉冲
 *           2. 模块自动发送 8 个 40kHz 超声波
 *           3. Echo 引脚变为高电平，持续时间为超声波往返时长
 *           4. 距离 = (高电平持续时间 × 声速) / 2
 *              声速 ≈ 340m/s = 0.034 cm/us
 *              距离(cm) = Echo高电平(us) × 0.034 / 2
 *                      = Echo高电平(us) / 58
 *         使用 TIM3_CH4 输入捕获测量 Echo 高电平脉宽
 */

#include "main.h"
#include "hc_sr04.h"

static volatile uint32_t sr04_capture_rise = 0;   /* 上升沿时刻 */
static volatile uint32_t sr04_capture_width = 0;  /* 脉宽 (us) */
static volatile uint8_t  sr04_capture_done = 0;   /* 捕获完成标志 */

void SR04_Init(void)
{
    /* 初始状态: Trig 拉低 */
    HAL_GPIO_WritePin(SR04_TRIG_PORT, SR04_TRIG_PIN, GPIO_PIN_RESET);

    /* 启动 TIM3 输入捕获中断 */
    HAL_TIM_IC_Start_IT(SR04_TIM, SR04_TIM_CHANNEL);
}

/**
 * @brief  触发一次超声波测量
 * @param  distance_cm  输出: 距离(厘米)，0=超时/无回波
 * @return 0=OK, 1=超时
 */
uint8_t SR04_Measure(float *distance_cm)
{
    /* 发送 Trig 脉冲: >10us 高电平 */
    HAL_GPIO_WritePin(SR04_TRIG_PORT, SR04_TRIG_PIN, GPIO_PIN_SET);
    for (volatile uint32_t i = 0; i < 200; i++) { __NOP(); }  /* ~20us */
    HAL_GPIO_WritePin(SR04_TRIG_PORT, SR04_TRIG_PIN, GPIO_PIN_RESET);

    /* 等待捕获完成，超时 50ms（对应最大距离约 8.5m） */
    uint32_t startTick = g_sysTick;
    sr04_capture_done = 0;
    while (!sr04_capture_done) {
        if (g_sysTick - startTick > 50) {
            *distance_cm = 0;
            return 1;   /* 超时 */
        }
    }

    /* 计算距离: 距离(cm) = 脉宽(us) / 58 */
    *distance_cm = (float)sr04_capture_width / 58.0f;

    return 0;
}

/**
 * @brief  TIM3 输入捕获中断回调
 * @note   由 HAL_TIM_IC_CaptureCallback 调用
 *         先用上升沿捕获起始时刻，切换下降沿后计算脉宽
 */
void HAL_TIM_IC_CaptureCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM3 && htim->Channel == HAL_TIM_ACTIVE_CHANNEL_4) {
        uint32_t capture_val = HAL_TIM_ReadCapturedValue(htim, TIM_CHANNEL_4);

        if (sr04_capture_rise == 0) {
            /* 第一次捕获：上升沿 */
            sr04_capture_rise = capture_val;
            /* 切换为下降沿捕获 */
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_4, TIM_INPUTCHANNELPOLARITY_FALLING);
        } else {
            /* 第二次捕获：下降沿 */
            if (capture_val > sr04_capture_rise) {
                sr04_capture_width = capture_val - sr04_capture_rise;
            } else {
                /* 计数器溢出回绕 */
                sr04_capture_width = (0xFFFF - sr04_capture_rise) + capture_val + 1;
            }
            sr04_capture_done = 1;
            sr04_capture_rise = 0;
            /* 切换回上升沿，准备下次测量 */
            __HAL_TIM_SET_CAPTUREPOLARITY(htim, TIM_CHANNEL_4, TIM_INPUTCHANNELPOLARITY_RISING);
        }
    }
}
