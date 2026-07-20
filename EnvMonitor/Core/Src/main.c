/**
 * @file    main.c
 * @brief   智能环境监测与物联网控制站 — FreeRTOS 版本
 * @note    6 个任务替代裸机超级循环:
 *          SensorTask(3) / AlarmTask(3) / DisplayTask(2) /
 *          ButtonTask(2) / BuzzerTask(2) / CloudTask(1)
 *
 *          资源保护: I2C_Mutex / Data_Mutex / UART_Mutex
 *          任务通信: ButtonQueue / BuzzerQueue
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

    /* 启动画面 */
    OLED_ShowString(0, 2, "Env Monitor v2.0", 0);
    OLED_ShowString(0, 4, "FreeRTOS Starting", 0);
    OLED_Refresh();
    HAL_Delay(1500);
    OLED_Clear();

    /* 共享资源初始化（互斥量 + 队列） */
    TaskShared_Init();

    /* ========== 创建 RTOS 任务 ========== */
    BaseType_t ret;

    /* --- SensorTask (prio=3, 5s周期) --- */
    ret = xTaskCreate(SensorTask_Run,
                      "Sensor",
                      SENSOR_TASK_STACK_SIZE,
                      NULL,
                      SENSOR_TASK_PRIORITY,
                      NULL);
    configASSERT(ret == pdPASS);

    /* --- AlarmTask (prio=3, 500ms周期) --- */
    ret = xTaskCreate(AlarmTask_Run,
                      "Alarm",
                      ALARM_TASK_STACK_SIZE,
                      NULL,
                      ALARM_TASK_PRIORITY,
                      NULL);
    configASSERT(ret == pdPASS);

    /* --- DisplayTask (prio=2, 1s周期) --- */
    ret = xTaskCreate(DisplayTask_Run,
                      "Display",
                      DISPLAY_TASK_STACK_SIZE,
                      NULL,
                      DISPLAY_TASK_PRIORITY,
                      NULL);
    configASSERT(ret == pdPASS);

    /* --- ButtonTask (prio=2, 20ms周期) --- */
    ret = xTaskCreate(ButtonTask_Run,
                      "Button",
                      BUTTON_TASK_STACK_SIZE,
                      NULL,
                      BUTTON_TASK_PRIORITY,
                      NULL);
    configASSERT(ret == pdPASS);

    /* --- BuzzerTask (prio=2, 队列驱动) --- */
    ret = xTaskCreate(BuzzerTask_Run,
                      "Buzzer",
                      BUZZER_TASK_STACK_SIZE,
                      NULL,
                      BUZZER_TASK_PRIORITY,
                      NULL);
    configASSERT(ret == pdPASS);

    /* --- CloudTask (prio=1, 最低, 60s周期) --- */
    ret = xTaskCreate(CloudTask_Run,
                      "Cloud",
                      CLOUD_TASK_STACK_SIZE,
                      NULL,
                      CLOUD_TASK_PRIORITY,
                      NULL);
    configASSERT(ret == pdPASS);

    /* ========== 启动调度器（永不返回） ========== */
    vTaskStartScheduler();

    /* 理论上不会到这里，但如果发生则进入错误处理 */
    Error_Handler();
    return 0;
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
