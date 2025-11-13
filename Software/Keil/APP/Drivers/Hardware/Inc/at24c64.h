#ifndef __AT24C64_H__
#define __AT24C64_H__

#include "i2c.h"
#include "stdint.h"
#include "stdbool.h"

#define AT24C64_PAGE_SIZE      32
#define AT24C64_TOTAL_SIZE     8192
#define AT24C64_I2C_ADDR       0xA0

void AT24C64_Init(I2C_HandleTypeDef *hi2c);
bool AT24C64_WriteByte(uint16_t addr, uint8_t data);
bool AT24C64_ReadByte(uint16_t addr, uint8_t *data);
bool AT24C64_Write(uint16_t addr, const uint8_t *data, uint16_t len);
bool AT24C64_Read(uint16_t addr, uint8_t *data, uint16_t len);

#endif
