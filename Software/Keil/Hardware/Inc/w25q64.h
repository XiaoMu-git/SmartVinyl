#ifndef _W25Q64_H_
#define _W25Q64_H_

#include "spi.h"
#include "stdint.h"

#define W25Q64_CS_GPIO              GPIOA
#define W25Q64_CS_PIN               GPIO_PIN_4
#define W25Q64_SECTOR_SIZE          (uint32_t)0x1000
#define W25Q64_PAGE_SIZE            (uint32_t)0x100
#define W25Q64_WAIT_TIME            (uint32_t)(60 * 1000)

enum W25Q64_CMD_TYPE {
    W25Q64_CMD_PAGE_PROGRAM = 0x02,
    W25Q64_CMD_READ_DATA    = 0x03,
    W25Q64_CMD_READ_STATUS1 = 0x05,
    W25Q64_CMD_WRITE_ENABLE = 0x06,
    W25Q64_CMD_SECTOR_ERASE = 0x20,
    W25Q64_CMD_READ_UID     = 0x4B,
    W25Q64_CMD_READ_JEDECID = 0x9F,
    W25Q64_CMD_CHIP_ERASE   = 0xC7,
};

BaseType_t w25q64EraseSector(uint32_t address);
BaseType_t w25q64EraseChip(void);
uint32_t w25q64ReadJEDECID(void);
uint32_t w25q64ReadUID(void);
BaseType_t w25q64Read(uint32_t address, uint8_t *data, uint32_t length);
BaseType_t w25q64Write(uint32_t address, uint8_t *data, uint32_t length);

#endif // !_W25Q64_H_
