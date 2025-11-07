#include "dma.h"

/// @brief DMA 初始化
/// @param  
void dmaInit(void) {  
	__HAL_RCC_DMA1_CLK_ENABLE();
    __HAL_RCC_DMA2_CLK_ENABLE();
}
