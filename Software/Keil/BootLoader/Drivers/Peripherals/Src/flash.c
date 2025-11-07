#include "flash.h"
#include "config.h"
#include "string.h"

/// @brief 检查读写位置是否合法
/// @param addr 
/// @param length 
/// @return 
int8_t flashRangeCheck(uint32_t addr, uint32_t length) {
    int8_t ret = RET_FAIL;
    uint32_t begin = addr;
    uint32_t end = addr + length - 1;
    
    if (FLASH_BASE_ADDR <= begin && end <= FLASH_BASE_ADDR + FLASH_SIZE - 1) {
        if (BOOT_ADDR <= begin && end <= BOOT_ADDR + BOOT_SIZE - 1) ret = RET_FAIL; // 禁止
        if (APP1_ADDR <= begin && end <= APP1_ADDR + APP1_SIZE - 1) ret = RET_DONE;
        if (APP2_ADDR <= begin && end <= APP2_ADDR + APP2_SIZE - 1) ret = RET_DONE;
        if (DATA_ADDR <= begin && end <= DATA_ADDR + DATA_SIZE - 1) ret = RET_DONE;
    }

    return ret;
}

void flashInit(void) {
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
    __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_2);
}

/// @brief 内部 flash 读操作
/// @param offset 
/// @param addr 
/// @param buff 
/// @param length 
/// @return 
uint8_t flashRead(uint32_t offset, uint32_t addr, uint8_t *buff, uint32_t length) {
    if (buff == NULL) return RET_FAIL;
    uint8_t ret = RET_FAIL;

    // 检查区域是否合法
    if (flashRangeCheck(offset + addr, length) == RET_DONE) {
        // 拷贝数据
        memcpy(buff, (const void*)(offset + addr), length);
        ret = RET_DONE;
    }

    return ret;
}

/// @brief 内部 flash 写操作
/// @param offset 
/// @param addr 
/// @param data 
/// @param length 
/// @return 
uint8_t flashWrite(uint32_t offset, uint32_t addr, uint8_t *data, uint32_t length) {
    if (data == NULL) return RET_FAIL;
    if (length == 0) return RET_DONE;
    uint8_t ret = RET_FAIL;
    static uint8_t page_buff[FLASH_PAGE_SIZE];

    // 检查区域是否合法
    if (flashRangeCheck(offset + addr, length) == RET_DONE) {
        // 计算页偏移量
        uint32_t page_begin = (offset + addr) / FLASH_PAGE_SIZE * FLASH_PAGE_SIZE;
        uint32_t page_offset = offset + addr - page_begin;
        uint32_t page_room = FLASH_PAGE_SIZE - page_offset;
        uint32_t copy_length = length < page_room ? length : page_room;

        // 缓存当前页面
        memcpy(page_buff, (const void*)page_begin, FLASH_PAGE_SIZE);
        // 覆盖修改的数据
        memcpy(page_buff + page_offset, data, copy_length);

        // 准备写入数据
        FLASH_EraseInitTypeDef erase_init_struct = {0};
        uint32_t page_error = 0xFFFFFFFF;
        erase_init_struct.TypeErase   = FLASH_TYPEERASE_PAGES;
        erase_init_struct.PageAddress = page_begin;
        erase_init_struct.NbPages     = 1;
        if (HAL_FLASH_Unlock() == HAL_OK) {
            if (HAL_FLASHEx_Erase(&erase_init_struct, &page_error) == HAL_OK && page_error == 0xFFFFFFFF) {
                // 写入数据
                for (uint32_t i = 0; i < FLASH_PAGE_SIZE; i += 2) {
                    uint16_t half_word = (uint16_t)page_buff[i] | (page_buff[i + 1] << 8);
                    if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, page_begin + i, half_word) != HAL_OK) {
                        HAL_FLASH_Lock();
                        return RET_FAIL;
                    }
                }
                // 递归修改下一个
                if (copy_length < length) ret = flashWrite(offset, addr + copy_length, data + copy_length, length - copy_length);
                else ret = RET_DONE;
            }
            HAL_FLASH_Lock();
        }
        else ret = RET_FAIL;
    }

    return ret;
}
