#ifndef _FLASH_H_
#define _FLASH_H_

#include "config.h"

#define FLASH_BASE_ADDR     FLASH_BASE
#define FLASH_SIZE          (uint32_t)0x00080000

#define FLASH_BOOT_SIZE     (uint32_t)0x00010000
#define FLASH_APP1_SIZE     (uint32_t)0x00020000
#define FLASH_APP2_SIZE     (uint32_t)0x00020000
#define FLASH_DATA_SIZE     (uint32_t)FLASH_SIZE - FLASH_BOOT_SIZE - FLASH_APP1_SIZE - FLASH_APP2_SIZE

#define FLASH_BOOT_ADDR     (uint32_t)FLASH_BASE_ADDR
#define FLASH_APP1_ADDR     (uint32_t)FLASH_BOOT_ADDR + FLASH_BOOT_SIZE
#define FLASH_APP2_ADDR     (uint32_t)FLASH_APP1_ADDR + FLASH_APP1_SIZE
#define FLASH_DATA_ADDR     (uint32_t)FLASH_APP2_ADDR + FLASH_APP2_SIZE

void flashInit(void);
uint8_t flashReadData(uint32_t offset, uint32_t addr, uint8_t *buff, uint32_t length);
uint8_t flashWriteData(uint32_t offset, uint32_t addr, uint8_t *data, uint32_t length);

#endif // !_FLASH_H_
