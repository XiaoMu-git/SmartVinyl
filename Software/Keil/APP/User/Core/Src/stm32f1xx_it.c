#include "main.h"

extern PCD_HandleTypeDef hpcd_USB_FS;

void NMI_Handler(void) {
    while (1);
}

void HardFault_Handler(void) {
    while (1);
}

void MemManage_Handler(void) {
    while (1);
}

void BusFault_Handler(void) {
    while (1);
}

void UsageFault_Handler(void) {
    while (1);
}

void DebugMon_Handler(void) {
    
}

void TIM7_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim7);
}

void USB_LP_CAN1_RX0_IRQHandler(void) {
    HAL_PCD_IRQHandler(&hpcd_USB_FS);
}

void USART1_IRQHandler(void) {
    HAL_UART_IRQHandler(&huart1);
}

// void I2C1_EV_IRQHandler(void) {
//     HAL_I2C_EV_IRQHandler(&hi2c1);
// }

// void I2C1_ER_IRQHandler(void) {
//     HAL_I2C_ER_IRQHandler(&hi2c1);
// }

// void DMA1_Channel2_IRQHandler(void) {
//     HAL_DMA_IRQHandler(&hdma_spi1_rx);
// }

// void DMA1_Channel3_IRQHandler(void) {
//     HAL_DMA_IRQHandler(&hdma_spi1_tx);
// }

void DMA1_Channel4_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_uart1_tx);
}

void DMA1_Channel5_IRQHandler(void) {
    HAL_DMA_IRQHandler(&hdma_uart1_rx);
}

// void DMA1_Channel6_IRQHandler(void) {
//     HAL_DMA_IRQHandler(&hdma_i2c1_tx);
// }

// void DMA1_Channel7_IRQHandler(void) {
//     HAL_DMA_IRQHandler(&hdma_i2c1_rx);
// }
