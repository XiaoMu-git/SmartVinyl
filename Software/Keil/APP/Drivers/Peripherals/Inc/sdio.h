#ifndef _SDIO_H_
#define _SDIO_H_

#include "stm32f1xx_hal.h"

extern SD_HandleTypeDef hsd;

void sdioInit(void);

#endif // !_SDIO_H_
