#include "main.h"

int main(void) {
    HAL_Init();

    /* -------- 外设初始化 -------- */
    clockInit();
    dmaInit();
    flashInit();
    // i2cInit();
    gpioInit();
    sdioInit();
    spiInit();
    timInit();
    uartInit();
    usbInit();

    /* -------- 硬件初始化 -------- */
    // AT24C64_Init(&hi2c1);
    w25q64Init(&hspi1, GPIOA, GPIO_PIN_4);
    // TF_Init(&hsd);

    /* --------- 创建任务 --------- */
    createLogTask();
    createTestTask();
    vTaskStartScheduler();

    return 0;
}
