#ifndef _AT24C64_H_
#define _AT24C64_H_

#include "i2c.h"

#define AT24C64_PAGE_SIZE      (uint32_t)0x0020
#define AT24C64_TOTAL_SIZE     (uint32_t)0x2000
#define AT24C64_I2C_ADDR       (uint32_t)0x0050

void at24c64Init(I2C_HandleTypeDef *hi2c);
uint8_t at24c64WriteData(uint32_t addr, uint8_t *data, uint32_t length);
uint8_t at24c64ReadData(uint32_t addr, uint8_t *data, uint32_t length);

#endif // !_AT24C64_H_
