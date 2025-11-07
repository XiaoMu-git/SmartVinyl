#ifndef _UART_H_
#define _UART_H_

#include "stm32f1xx_hal.h"

extern UART_HandleTypeDef huart1;

void uartInit(void);

#endif // !_UART_H_
