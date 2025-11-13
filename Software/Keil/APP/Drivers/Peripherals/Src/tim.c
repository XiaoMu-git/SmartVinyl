#include "tim.h"

TIM_HandleTypeDef htim7;

void timInit(void) {
    
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
