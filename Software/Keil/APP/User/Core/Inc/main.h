#ifndef _MAIN_H_
#define _MAIN_H_

#include "stm32f1xx_hal.h"

#include "clock.h"
#include "dma.h"
#include "flash.h"
#include "i2c.h"
#include "gpio.h"
#include "sdio.h"
#include "spi.h"
#include "tim.h"
#include "uart.h"
#include "usb.h"

#include "at24c64.h"
#include "w25q64.h"

#include "task_log.h"
#include "task_test.h"

extern PCD_HandleTypeDef hpcd_USB_FS;

#endif // !_MAIN_H_
