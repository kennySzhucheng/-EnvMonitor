/**
 * @file    main.c
 * @brief   智能环境监测与物联网控制站 — 主程序
 * @note    CubeMX 生成工程骨架，此文件为手动整合版本
 *          SystemClock_Config() 和 MX_* 函数由 CubeMX 生成
 */

#include "main.h"

/* ==================== 全局变量 ==================== */
SensorData_t g_sensorData = {0};
volatile uint32_t g_sysTick = 0;

/* ==================== 主函数 ==================== */
int main(void)
{
    /* HAL 初始化 */
    HAL_Init();

    /* 系统时钟 72MHz */
    SystemClock_Config();

    /* 外设初始化（CubeMX 生成） */
    MX_GPIO_Init();
    MX_I2C1_Init();
    MX_TIM3_Init();
    MX_USART1_UART_Init();

    /* 驱动层初始化 */
    OLED_Init();
    DHT22_Init();
    BH1750_Init();
    SR04_Init();
    ESP8266_Init();
    Button_Init();
    Buzzer_Init();
    LED_Init();

    /* 应用层初始化 */
    APP_Init();

    /* 启动画面 */
    OLED_ShowString(0, 2, "Env Monitor v1.0", 0);
    OLED_Refresh();
    HAL_Delay(1500);
    OLED_Clear();

    /* ==================== 主循环 ==================== */
    uint32_t lastSensorTick  = 0;
    uint32_t lastCloudTick   = 0;
    uint32_t lastDisplayTick = 0;
    uint32_t lastAlarmTick   = 0;

    while (1)
    {
        /* --- 5秒采样 --- */
        if (g_sysTick - lastSensorTick >= SENSOR_SAMPLE_INTERVAL_MS) {
            lastSensorTick = g_sysTick;
            APP_SampleSensors();
        }

        /* --- 60秒上云 --- */
        if (g_sysTick - lastCloudTick >= CLOUD_UPLOAD_INTERVAL_MS) {
            lastCloudTick = g_sysTick;
            APP_UploadToCloud();
        }

        /* --- 1秒刷新屏幕 --- */
        if (g_sysTick - lastDisplayTick >= OLED_REFRESH_INTERVAL_MS) {
            lastDisplayTick = g_sysTick;
            APP_UpdateDisplay();
        }

        /* --- 0.5秒报警检查 --- */
        if (g_sysTick - lastAlarmTick >= ALARM_CHECK_INTERVAL_MS) {
            lastAlarmTick = g_sysTick;
            ALARM_Check(&g_sensorData);
        }

        /* --- 按键扫描 --- */
        Button_Scan();

        /* --- 蜂鸣器状态机 --- */
        Buzzer_Tick();
    }
}

/* ==================== 系统时钟配置 ==================== */
/**
 * @brief  HSE(8MHz) → PLL(×9) → SYSCLK(72MHz)
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK) {
        Error_Handler();
    }

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK) {
        Error_Handler();
    }
}

/* ==================== 错误处理 ==================== */
void Error_Handler(void)
{
    __disable_irq();
    while (1) {
        HAL_GPIO_TogglePin(LED_RED_PORT, LED_RED_PIN);
        for (volatile uint32_t i = 0; i < 500000; i++) { __NOP(); }
    }
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    printf("Assert failed: %s, line %lu\r\n", file, line);
    Error_Handler();
}
#endif
