#include "main.h"

int main(void) {
    HAL_Init();

    /* -------- 外设初始化 -------- */
    clockInit();
    dmaInit();
    flashInit();
    i2cInit();
    gpioInit();
    sdioInit();
    spiInit();
    timInit();
    uartInit();
    usbInit();

    /* -------- 硬件初始化 -------- */
    at24c64Init(&hi2c1);
    w25q64Init(&hspi1, GPIOA, GPIO_PIN_4);

    /* --------- 创建任务 --------- */
    createLogTask();
    createTestTask();
    vTaskStartScheduler();

    return 0;
}
