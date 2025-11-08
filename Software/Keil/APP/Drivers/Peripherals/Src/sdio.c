#include "sdio.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"

SD_HandleTypeDef hsd;
char fatfs_path[] = "";
FATFS fatfs;

void sdioInit(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_SDIO_CLK_ENABLE();

    // SDIO 引脚
    GPIO_InitStruct.Pin = GPIO_PIN_8 | GPIO_PIN_9 | GPIO_PIN_10 | GPIO_PIN_11 | GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

    GPIO_InitStruct.Pin = GPIO_PIN_2;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOD, &GPIO_InitStruct);

    // SDIO 参数配置
    hsd.Instance = SDIO;
    hsd.Init.ClockEdge = SDIO_CLOCK_EDGE_RISING;
    hsd.Init.ClockBypass = SDIO_CLOCK_BYPASS_DISABLE;
    hsd.Init.ClockPowerSave = SDIO_CLOCK_POWER_SAVE_DISABLE;
    hsd.Init.BusWide = SDIO_BUS_WIDE_1B;
    hsd.Init.HardwareFlowControl = SDIO_HARDWARE_FLOW_CONTROL_DISABLE;
    hsd.Init.ClockDiv = 12 - 1;

    // 初始化 SD 卡
    if (HAL_SD_Init(&hsd) != HAL_OK) {
        __disable_irq();
        while(1);
    }

    // 切换 4 线模式
    if (HAL_SD_ConfigWideBusOperation(&hsd, SDIO_BUS_WIDE_4B) != HAL_OK) {
        __disable_irq();
        while(1);
    }

    // 初始化 FATFS
    if (FATFS_LinkDriver(&SD_Driver, fatfs_path) != HAL_OK) {
        __disable_irq();
        while(1);
    }
}
