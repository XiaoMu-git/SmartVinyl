#ifndef _UART_H_
#define _UART_H_

#include "config.h"

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_uart1_tx;
extern DMA_HandleTypeDef hdma_uart1_rx;
extern uint8_t uart1_tx_buff[UART1_BUFF_SIZE];
extern uint8_t uart1_rx_buff[UART1_BUFF_SIZE];
extern SemaphoreHandle_t uart1_mutex;
extern SemaphoreHandle_t uart1_tx_semaphore;

void uartInit(void);
uint8_t uart1SendData(uint8_t *data, uint32_t length);

#endif // !_UART_H_
