#include "main.h"
#include "config.h"
#include "string.h"
#include "stdio.h"

int main(void) {
    HAL_Init();

    /* -------- 初始化外设 -------- */
    clockInit();
    flashInit();
    gpioInit();
    uartInit();

    // 跳转程序
    SysTick->CTRL = 0;
    SysTick->LOAD = 0;
    SysTick->VAL = 0;
    __set_MSP(*(uint32_t*)APP1_ADDR);
    SCB->VTOR = APP1_ADDR;
    ((void (*)(void))(*(uint32_t*)(APP1_ADDR + 4)))();

    while (1);
}
