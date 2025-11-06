#include "spi.h"
#include "string.h"

SPI_HandleTypeDef hspi1;
DMA_HandleTypeDef hdma_spi1_rx;
DMA_HandleTypeDef hdma_spi1_tx;
uint8_t buff_spi1_tx[SPI_BUFF_SIZE];
uint8_t buff_spi1_rx[SPI_BUFF_SIZE];
SemaphoreHandle_t semaphore_spi1_rxtx;
SemaphoreHandle_t mutex_spi1_rxtx;

void spiInit(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_SPI1_CLK_ENABLE();

    // SPI1 SCK PIN
    GPIO_InitStruct.Pin = GPIO_PIN_5;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // SPI1 MISO PIN
    GPIO_InitStruct.Pin = GPIO_PIN_6;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // SPI1 MOSI PIN
    GPIO_InitStruct.Pin = GPIO_PIN_7;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // SPI1 CS PIN
    GPIO_InitStruct.Pin = GPIO_PIN_4;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, GPIO_PIN_SET);
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // SPI1 RX DMA
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

    // SPI1 TX DMA
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

    // SPI1 INIT
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

    // 申请 IPC 资源
    semaphore_spi1_rxtx = xSemaphoreCreateBinary();
    if (semaphore_spi1_rxtx == NULL) {
        __disable_irq();
        while (1);
    }
    mutex_spi1_rxtx = xSemaphoreCreateMutex();
    if (mutex_spi1_rxtx == NULL) {
        __disable_irq();
        while (1);
    }
}

/// @brief spi1 DMA　收发完成
/// @param hspi 
void spiRxTxDmaCompare(SPI_HandleTypeDef *hspi) {
    if (hspi->Instance == SPI1) {
        xSemaphoreGive(semaphore_spi1_rxtx);
    }
}

/// @brief 使用 SPI1 收发数据
/// @param data 
/// @param length 
/// @param gpio 
/// @param pin 
/// @return 
BaseType_t spi1SendRecvData(GPIO_TypeDef *gpio, uint16_t pin, uint8_t *data, uint32_t length) {
    if (gpio == NULL || pin == 0 || data == NULL || length == 0) return pdFALSE;
    BaseType_t result = pdFALSE;
    // 互斥锁上锁
    if (xSemaphoreTake(mutex_spi1_rxtx, TIME_WAIT_MEDIUM) == pdTRUE) {
        // 下拉片选线
        HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_RESET);
        // 使用循环，直到当前包内所有数据发送完成
        for (uint32_t offset = 0; offset < length; offset += SPI_BUFF_SIZE) {
            // 计算可拷贝长度
            uint32_t copy_len = length - offset > SPI_BUFF_SIZE ? SPI_BUFF_SIZE : length - offset;
            // 拷贝数据
            memcpy(buff_spi1_tx, data + offset, copy_len);
            // 清空 DMA 收发完成信号
            xSemaphoreTake(semaphore_spi1_rxtx, 0);
            // 开启 DMA 发送
            result = pdFALSE;
            if (HAL_SPI_TransmitReceive_DMA(&hspi1, buff_spi1_tx, buff_spi1_rx, copy_len) == HAL_OK) {
                // 等待 DMA 收发完成
                if (xSemaphoreTake(semaphore_spi1_rxtx, TIME_WAIT_SHORT) == pdTRUE) result = pdTRUE;
            }
            // 有一次发送失败就停止
            if (result != pdTRUE) break;
            // 拷贝接收到的数据
            memcpy(data + offset, buff_spi1_rx, copy_len);
        }
        // 释放片选线
        HAL_GPIO_WritePin(gpio, pin, GPIO_PIN_SET);
        // 互斥锁解锁
        xSemaphoreGive(mutex_spi1_rxtx);
    }
    return result;
}
