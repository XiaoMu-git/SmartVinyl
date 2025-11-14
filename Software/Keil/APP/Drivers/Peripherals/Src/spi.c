#include "spi.h"
#include "string.h"

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;
uint8_t spi1_tx_buff[SPI1_BUFF_SIZE];
uint8_t spi1_rx_buff[SPI1_BUFF_SIZE];
SemaphoreHandle_t spi1_mutex;
SemaphoreHandle_t spi1_rxtx_semaphore;

/// @brief SPI 初始化函数
void spiInit(void) {
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    __HAL_RCC_SPI1_CLK_ENABLE();

    // SPI1 引脚
    GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 配置参数
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    hspi1.Init.TIMode = SPI_TIMODE_DISABLE;
    hspi1.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
    hspi1.Init.CRCPolynomial = 10;
    if (HAL_SPI_Init(&hspi1) != HAL_OK) {
        __disable_irq();
        while (1);
    }

    // 配置 DMA
    hdma_spi1_rx.Instance = DMA1_Channel2;
    hdma_spi1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_spi1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi1_rx.Init.Mode = DMA_NORMAL;
    hdma_spi1_rx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_spi1_rx) != HAL_OK) {
        __disable_irq();
        while (1);
    }
    __HAL_LINKDMA(&hspi1, hdmarx, hdma_spi1_rx);

    hdma_spi1_tx.Instance = DMA1_Channel3;
    hdma_spi1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_spi1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_spi1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_spi1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_spi1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_spi1_tx.Init.Mode = DMA_NORMAL;
    hdma_spi1_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_spi1_tx) != HAL_OK) {
        __disable_irq();
        while (1);
    }
    __HAL_LINKDMA(&hspi1, hdmatx, hdma_spi1_tx);

    // 申请 IPC 资源
    spi1_mutex = xSemaphoreCreateMutex();
    spi1_rxtx_semaphore = xSemaphoreCreateBinary();
    if (spi1_mutex == NULL || spi1_rxtx_semaphore == NULL) {
        __disable_irq();
        while (1);
    }
}

/// @brief 使用 SPI 收发数据
/// @param data 
/// @param length 
/// @param gpio 
/// @param pin 
/// @return 
uint8_t spiSendRecvData(SPI_HandleTypeDef *hspi, GPIO_TypeDef *gpio, uint16_t pin, uint8_t *data, uint32_t length) {
    if (gpio == NULL || pin == 0 || data == NULL) return pdFALSE;
    uint8_t ret = RET_DONE;
    // 判断是哪个 SPI
    if (hspi->Instance == SPI1) {
        // 抢占 SPI1
        if (xSemaphoreTake(spi1_mutex, TIME_WAIT_MEDIUM) == pdTRUE) {
            // 下拉片选线
            HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_RESET);
            while (length > 0) {
                uint32_t copy_length = length <= SPI1_BUFF_SIZE ? length : SPI1_BUFF_SIZE;
                // 拷贝待发送数据
                memcpy(spi1_tx_buff, data, copy_length);
                // 启动 DMA 发送
                if (HAL_SPI_TransmitReceive_DMA(&hspi1, spi1_tx_buff, spi1_rx_buff, copy_length) != HAL_OK) {
                    ret = RET_FAIL;
                    break;
                }
                // 等待收发完成
                if (xSemaphoreTake(spi1_rxtx_semaphore, TIME_WAIT_SHORT) != pdTRUE) {
                    ret = RET_FAIL;
                    break;
                }
                // 拷贝接收到的数据
                memcpy(data, spi1_rx_buff, copy_length);
                // 收发成功处理
                data += copy_length;
                length -= copy_length;
            }
            // 释放片选线
            HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_SET);
            xSemaphoreGive(spi1_mutex);
        }
        else ret = RET_FAIL;
    }
    return ret;
}
