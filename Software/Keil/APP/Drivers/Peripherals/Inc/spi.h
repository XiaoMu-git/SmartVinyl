#ifndef _SPI_H_
#define _SPI_H_

#include "config.h"

extern SPI_HandleTypeDef hspi1;
extern DMA_HandleTypeDef hdma_spi1_rx;
extern DMA_HandleTypeDef hdma_spi1_tx;
extern uint8_t spi1_tx_buff[SPI1_BUFF_SIZE];
extern uint8_t spi1_rx_buff[SPI1_BUFF_SIZE];
extern SemaphoreHandle_t spi1_mutex;
extern SemaphoreHandle_t spi1_rxtx_semaphore;

void spiInit(void);
uint8_t spi1SendRecvData(GPIO_TypeDef *gpio, uint16_t pin, uint8_t *data, uint32_t length);

#endif // !_SPI_H_
