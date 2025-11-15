#include "tim.h"

TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim7;

/// @brief 定时器初始化函数
void timInit(void) {
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    TIM_MasterConfigTypeDef sMasterConfig = { 0 };
    TIM_OC_InitTypeDef sConfigOC = { 0 };
    __HAL_RCC_TIM2_CLK_ENABLE();

    // TIM2 PWM1 引脚
    GPIO_InitStruct.Pin = GPIO_PIN_0;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    
    // TIM2 参数配置
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 72 - 1;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 1000 - 1;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;
    if (HAL_TIM_PWM_Init(&htim2) != HAL_OK) {
        __disable_irq();
        while (1);
    }

    // 触发输出参数
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK) {
        __disable_irq();
        while (1);
    }

    // 配置 TIM2 PWM1 参数
    sConfigOC.OCMode = TIM_OCMODE_PWM1;
    sConfigOC.Pulse = 0;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    if (HAL_TIM_PWM_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_1) != HAL_OK) {
        __disable_irq();
        while (1);
    }

    // 开启 TIM2 PWM1
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_1);
}

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority) {
    RCC_ClkInitTypeDef clk_config = { 0 };
    uint32_t apb1_clk = 0, apb1_prescaler = 0;
    uint32_t flash_latency = 0;
    HAL_StatusTypeDef ret = HAL_OK;
    __HAL_RCC_TIM7_CLK_ENABLE();

    // 获取 APB 时钟
    HAL_RCC_GetClockConfig(&clk_config, &flash_latency);
    apb1_prescaler = clk_config.APB1CLKDivider;
    if (apb1_prescaler == RCC_HCLK_DIV1) apb1_clk = HAL_RCC_GetPCLK1Freq();
    else apb1_clk = 2U * HAL_RCC_GetPCLK1Freq();

    // 配置定时器参数
    htim7.Instance = TIM7;
    htim7.Init.Period = (1000000U / 1000U) - 1U;
    htim7.Init.Prescaler = (apb1_clk / 1000000U) - 1U;
    htim7.Init.ClockDivision = 0;
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    // 开启定时器和中断
    ret = HAL_TIM_Base_Init(&htim7);
    if (ret == HAL_OK) {
        ret = HAL_TIM_Base_Start_IT(&htim7);
        if (ret == HAL_OK) {
            HAL_NVIC_EnableIRQ(TIM7_IRQn);
            if (TickPriority < (1UL << __NVIC_PRIO_BITS)) {
                HAL_NVIC_SetPriority(TIM7_IRQn, TickPriority, 0U);
                uwTickPrio = TickPriority;
            }
            else ret = HAL_ERROR;
        }
    }

    return ret;
}
