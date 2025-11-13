#include "main.h"

int main(void) {
    HAL_Init();

    /* -------- 外设初始化 -------- */
    clockInit();
    dmaInit();
    // flashInit();
    // i2cInit();
    gpioInit();
    sdioInit();
    spiInit();
    timInit();
    uartInit();
    usbInit();

    /* -------- 硬件初始化 -------- */
    // AT24C64_Init(&hi2c1);
    // W25Q64_Init(&hspi1);
    // TF_Init(&hsd);

    /* --------- 创建任务 --------- */
    createLogTask();
    createTestTask();
    vTaskStartScheduler();

    return 0;
}
