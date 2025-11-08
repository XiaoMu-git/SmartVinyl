#include "dma.h"

/// @brief DMA 初始化
/// @param  
void dmaInit(void) {
    // 开启 DMA1/2 时钟
	__HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
}
