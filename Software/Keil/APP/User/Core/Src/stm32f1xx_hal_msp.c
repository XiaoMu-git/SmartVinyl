#include "main.h"

/// @brief HAL 库用户自定义初始化回调函数
void HAL_MspInit(void) {
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();

    HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);

    __HAL_AFIO_REMAP_SWJ_NOJTAG();
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM7) HAL_IncTick();
}
