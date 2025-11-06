#ifndef __AT24C64_H__
#define __AT24C64_H__

#include "stdint.h"
#include "stdbool.h"
#include "stm32f1xx_hal.h"  // 根据实际MCU型号修改

#define AT24C64_PAGE_SIZE      32
#define AT24C64_TOTAL_SIZE     8192  // 8KB
#define AT24C64_I2C_ADDR       0xA0  // 默认器件地址（实际应左移1位并附带R/W位）

void AT24C64_Init(I2C_HandleTypeDef *hi2c);
bool AT24C64_WriteByte(uint16_t addr, uint8_t data);
bool AT24C64_ReadByte(uint16_t addr, uint8_t *data);
bool AT24C64_Write(uint16_t addr, const uint8_t *data, uint16_t len);
bool AT24C64_Read(uint16_t addr, uint8_t *data, uint16_t len);

#endif
