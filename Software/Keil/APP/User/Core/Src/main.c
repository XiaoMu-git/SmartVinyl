#include "stm32f1xx_hal.h"
#include "clock.h"
#include "dma.h"
#include "gpio.h"
#include "usb.h"
#include "uart.h"
#include "spi.h"
#include "i2c.h"
#include "task_usb.h"
#include "task_log.h"
#include "task_uart.h"
#include "task_test.h"

int main(void) {
    HAL_Init();

    /* -------- 初始化外设 -------- */
    clockInit();
    // dmaInit();
    gpioInit();
    // usbInit();
    // uartInit();
    // spiInit();
    // i2cInit();

    /* --------- 创建任务 --------- */
    // createUsbTask();
    // createLogTask();
    // createUartTask();
    createTestTask();
    vTaskStartScheduler();

    return 0;
}
