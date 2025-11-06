#include "dma.h"

void dmaInit(void) {  
	__HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();

    // spi1 rx
    HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 8, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
    
    // spi1 tx
    HAL_NVIC_SetPriority(DMA1_Channel3_IRQn, 8, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel3_IRQn);
    
    // uart1 tx
    HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 8, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
    
    // uart1 rx
    HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 8, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

    // i2c tx
    HAL_NVIC_SetPriority(DMA1_Channel6_IRQn, 8, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel6_IRQn);
    
    // i2c rx
    HAL_NVIC_SetPriority(DMA1_Channel7_IRQn, 8, 0);
    HAL_NVIC_EnableIRQ(DMA1_Channel7_IRQn);
}
