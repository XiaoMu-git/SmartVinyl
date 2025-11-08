#include "tim.h"

TIM_HandleTypeDef htim7;

void timInit(void) {
    
}

HAL_StatusTypeDef HAL_InitTick(uint32_t TickPriority) {
    RCC_ClkInitTypeDef    clkconfig;
    uint32_t              uwTimclock, uwAPB1Prescaler = 0U;
    uint32_t              uwPrescalerValue = 0U;
    uint32_t              pFLatency;
    HAL_StatusTypeDef     status = HAL_OK;
    __HAL_RCC_TIM7_CLK_ENABLE();

    HAL_RCC_GetClockConfig(&clkconfig, &pFLatency);
    uwAPB1Prescaler = clkconfig.APB1CLKDivider;
    if (uwAPB1Prescaler == RCC_HCLK_DIV1) uwTimclock = HAL_RCC_GetPCLK1Freq();
    else uwTimclock = 2UL * HAL_RCC_GetPCLK1Freq();
    uwPrescalerValue = (uint32_t) ((uwTimclock / 1000000U) - 1U);

    htim7.Instance = TIM7;
    htim7.Init.Period = (1000000U / 1000U) - 1U;
    htim7.Init.Prescaler = uwPrescalerValue;
    htim7.Init.ClockDivision = 0;
    htim7.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim7.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;

    status = HAL_TIM_Base_Init(&htim7);
    if (status == HAL_OK) {
        status = HAL_TIM_Base_Start_IT(&htim7);
        if (status == HAL_OK) {
            HAL_NVIC_EnableIRQ(TIM7_IRQn);
            if (TickPriority < (1UL << __NVIC_PRIO_BITS)) {
                HAL_NVIC_SetPriority(TIM7_IRQn, TickPriority, 0U);
                uwTickPrio = TickPriority;
            }
            else status = HAL_ERROR;
        }
    }

    return status;
}
