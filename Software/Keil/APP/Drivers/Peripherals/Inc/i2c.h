#ifndef _I2C_H_
#define _I2C_H_

#include "stm32f1xx_hal.h"

#define I2C1_BUFF_SIZE      (uint32_t)512

extern I2C_HandleTypeDef hi2c1;
extern DMA_HandleTypeDef hdma_i2c1_rx;
extern DMA_HandleTypeDef hdma_i2c1_tx;
extern uint8_t buff_i2c1_tx[I2C1_BUFF_SIZE];
extern uint8_t buff_i2c1_rx[I2C1_BUFF_SIZE];

void i2cInit(void);
void i2cRxDmaCompare(I2C_HandleTypeDef *hi2c);
void i2cTxDmaCompare(I2C_HandleTypeDef *hi2c);

#endif // !_I2C_H_
