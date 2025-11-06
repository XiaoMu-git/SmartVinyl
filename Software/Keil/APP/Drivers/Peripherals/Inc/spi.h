#ifndef _SPI_H_
#define _SPI_H_

#include "config.h"

#define SPI_BUFF_SIZE      (uint32_t)512

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;
extern uint8_t buff_spi1_tx[SPI_BUFF_SIZE];
extern uint8_t buff_spi1_rx[SPI_BUFF_SIZE];
extern SemaphoreHandle_t semaphore_spi1_rxtx;
extern SemaphoreHandle_t mutex_spi1_rxtx;

void spiInit(void);
void spiRxTxDmaCompare(SPI_HandleTypeDef *hspi);
BaseType_t spi1SendRecvData(GPIO_TypeDef *gpio, uint16_t pin, uint8_t *data, uint32_t length);

#endif // !_SPI_H_
