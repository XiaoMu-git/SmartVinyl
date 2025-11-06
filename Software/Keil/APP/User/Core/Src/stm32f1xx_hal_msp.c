#include "stm32f1xx_hal.h"
#include "uart.h"
#include "spi.h"
#include "i2c.h"

/// @brief HAL 库用户自定义初始化回调函数
void HAL_MspInit(void) {
    __HAL_RCC_AFIO_CLK_ENABLE();
    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_AFIO_REMAP_SWJ_NOJTAG();
}

/// @brief uart 接收完成回调函数
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->hdmarx != NULL) uartRxDmaCompare(huart);
}

/// @brief uart 发送完成回调函数
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart) {
    if (huart->hdmatx != NULL) uartTxDmaCompare(huart);
}

/// @brief spi 收发完成回调函数
/// @param hspi 
void HAL_SPI_TxRxCpltCallback(SPI_HandleTypeDef *hspi) {
    if (hspi->hdmarx != NULL) spiRxTxDmaCompare(hspi);
}

/// @brief i2c 接收完成回调函数
void HAL_I2C_RxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c->hdmarx != NULL) i2cRxDmaCompare(hi2c);
}

/// @brief i2c 发送完成回调函数
void HAL_I2C_TxCpltCallback(I2C_HandleTypeDef *hi2c) {
    if (hi2c->hdmatx != NULL) i2cTxDmaCompare(hi2c);
}
