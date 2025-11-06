#ifndef _TASK_UART_H_
#define _TASK_UART_H_

#include "config.h"

typedef struct {
    TaskHandle_t htask_rx;
    QueueHandle_t queue_rx;
} UartResource;

typedef struct {
    uint32_t length;
    uint8_t *payload;
} UartPacket;

extern UartResource uart1_res;

void createUartTask(void);
BaseType_t uart1RecvData(uint8_t *data, uint32_t length);

#endif // !_TASK_UART_H_
