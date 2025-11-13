#ifndef _I2C_H_
#define _I2C_H_

#include "config.h"

#define I2C1_BUFF_SIZE              (uint32_t)64

extern I2C_HandleTypeDef hi2c1;
extern DMA_HandleTypeDef hdma_i2c1_rx;
extern DMA_HandleTypeDef hdma_i2c1_tx;
extern uint8_t i2c1_tx_buff[I2C1_BUFF_SIZE];
extern uint8_t i2c1_rx_buff[I2C1_BUFF_SIZE];

void i2cInit(void);

#endif // !_I2C_H_
