#include "main.h"

int main(void) {
    HAL_Init();

    /* -------- 初始化外设 -------- */
    clockInit();
    dmaInit();
    flashInit();
    gpioInit();
    sdioInit();
    spiInit();
    timInit();
    uartInit();
    usbInit();

    /* --------- 创建任务 --------- */
    createFileTask();
    createLogTask();
    createTestTask();
    vTaskStartScheduler();

    return 0;
}
