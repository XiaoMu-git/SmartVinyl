#include "main.h"

/// @brief HAL 库用户自定义初始化回调函数
void HAL_MspInit(void) {
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_NVIC_SetPriority(SysTick_IRQn, 15, 0);
    HAL_NVIC_SetPriority(PendSV_IRQn, 15, 0);
    __HAL_AFIO_REMAP_SWJ_NOJTAG();
}

/// @brief TIM7 HAL滴答
/// @param htim 
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
    if (htim->Instance == TIM7) {
        HAL_IncTick();
    }
}

/// @brief UART 接收完成
/// @param huart 
/// @param size 
void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t size) {
    if (huart->Instance == USART1) {
        HAL_UARTEx_ReceiveToIdle_DMA(huart, uart1_rx_buff, UART1_BUFF_SIZE);
        __HAL_DMA_DISABLE_IT(huart->hdmarx, DMA_IT_HT);
        // 处理接收到的数据
        
    }
}

/// @brief uart 发送完成
/// @param huart 
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) xSemaphoreGive(uart1_tx_semaphore);
}

/// @brief spi 收发完成
/// @param hspi 
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) xSemaphoreGive(spi1_rxtx_semaphore);
}
