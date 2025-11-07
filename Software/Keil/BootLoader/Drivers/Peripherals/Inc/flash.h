#ifndef _FLASH_H_
#define _FLASH_H_

#include "stm32f1xx_hal.h"

#define FLASH_BASE_ADDR     FLASH_BASE
#define FLASH_SIZE          (uint32_t)0x00080000

#define BOOT_SIZE           (uint32_t)0x00010000
#define APP1_SIZE           (uint32_t)0x00020000
#define APP2_SIZE           (uint32_t)0x00020000
#define DATA_SIZE           (uint32_t)FLASH_SIZE - BOOT_SIZE - APP1_SIZE - APP2_SIZE

#define BOOT_ADDR           (uint32_t)FLASH_BASE_ADDR
#define APP1_ADDR           (uint32_t)BOOT_ADDR + BOOT_SIZE
#define APP2_ADDR           (uint32_t)APP1_ADDR + APP1_SIZE
#define DATA_ADDR           (uint32_t)APP2_ADDR + APP2_SIZE

void flashInit(void);
uint8_t flashRead(uint32_t offset, uint32_t addr, uint8_t *buff, uint32_t length);
uint8_t flashWrite(uint32_t offset, uint32_t addr, uint8_t *data, uint32_t length);

#endif // !_FLASH_H_
