#ifndef _UART_H_
#define _UART_H_

#include "config.h"

#define UART_BUFF_SIZE      (uint32_t)512

extern UART_HandleTypeDef huart1;
extern DMA_HandleTypeDef hdma_uart1_tx;
extern DMA_HandleTypeDef hdma_uart1_rx;
extern uint8_t buff_uart1_tx[UART_BUFF_SIZE];
extern uint8_t buff_uart1_rx[UART_BUFF_SIZE];
extern SemaphoreHandle_t semaphore_uart1_tx;

void uartInit(void);
void uartRxDmaCompare(UART_HandleTypeDef *huart);
void uartTxDmaCompare(UART_HandleTypeDef *huart);
void uartRxIdleIrq(UART_HandleTypeDef *huart);
BaseType_t uartSendData(UART_HandleTypeDef *huart, uint8_t *data, uint32_t length);

#endif // !_UART_H_
