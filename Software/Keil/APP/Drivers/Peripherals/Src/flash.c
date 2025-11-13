#include "flash.h"
#include "config.h"
#include "string.h"

SemaphoreHandle_t flash_mutex;

/// @brief 检查读写位置是否合法
/// @param addr 
/// @param length 
/// @return 
uint8_t flashRangeCheck(uint32_t addr, uint32_t length) {
    int8_t ret = RET_FAIL;
    uint32_t begin = addr;
    uint32_t end = addr + length - 1;
    
    // 判断是否落在独立的区间内
    if (FLASH_BASE_ADDR <= begin && end <= FLASH_BASE_ADDR + FLASH_SIZE - 1) {
        if (BOOT_ADDR <= begin && end <= BOOT_ADDR + BOOT_SIZE - 1) ret = RET_FAIL; // 禁止
        if (APP1_ADDR <= begin && end <= APP1_ADDR + APP1_SIZE - 1) ret = RET_DONE;
        if (APP2_ADDR <= begin && end <= APP2_ADDR + APP2_SIZE - 1) ret = RET_DONE;
        if (DATA_ADDR <= begin && end <= DATA_ADDR + DATA_SIZE - 1) ret = RET_DONE;
    }

    return ret;
}

/// @brief flash 初始化函数
/// @param  
void flashInit(void) {
    __HAL_FLASH_PREFETCH_BUFFER_ENABLE();
    __HAL_FLASH_SET_LATENCY(FLASH_LATENCY_2);
    flash_mutex = xSemaphoreCreateMutex();
}

/// @brief 内部 flash 读操作
/// @param offset 
/// @param addr 
/// @param buff 
/// @param length 
/// @return 
uint8_t flashReadData(uint32_t offset, uint32_t addr, uint8_t *buff, uint32_t length) {
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
uint8_t flashWriteData(uint32_t offset, uint32_t addr, uint8_t *data, uint32_t length) {
    if (data == NULL) return RET_FAIL;
    if (length == 0) return RET_DONE;
    uint8_t ret = RET_DONE;

    // 等待互斥锁
    if (xSemaphoreTake(flash_mutex, TIME_WAIT_MEDIUM) == pdTRUE) {
        // 检查区域是否合法
        if (flashRangeCheck(offset + addr, length) == RET_DONE) {
            uint8_t *page_buff = (uint8_t*)pvPortMalloc(FLASH_PAGE_SIZE);
            if (page_buff != NULL) {
                uint32_t first_page = (offset + addr) / FLASH_PAGE_SIZE;
                uint32_t last_page = (offset + addr + length - 1) / FLASH_PAGE_SIZE;
                for (uint32_t page = first_page; page <= last_page; page++) {
                    // 计算拷贝首尾地址
                    uint32_t page_addr = page * FLASH_PAGE_SIZE;
                    uint32_t page_begin = 0, page_end = FLASH_PAGE_SIZE - 1;
                    if (page == first_page) page_begin = offset + addr - page_addr;
                    if (page == last_page) page_end = (offset + addr + length - 1) - page_addr;

                    // 读取并修改数据
                    memcpy(page_buff, (const void*)page_addr, FLASH_PAGE_SIZE);
                    memcpy(page_buff + page_begin, data, page_end - page_begin + 1);

                    // 准备写入数据
                    FLASH_EraseInitTypeDef erase_init_struct = { 0 };
                    uint32_t page_error = 0xFFFFFFFF;
                    erase_init_struct.TypeErase   = FLASH_TYPEERASE_PAGES;
                    erase_init_struct.PageAddress = page_addr;
                    erase_init_struct.NbPages     = 1;
                    // flash 解锁
                    if (HAL_FLASH_Unlock() == HAL_OK) {
                        if (HAL_FLASHEx_Erase(&erase_init_struct, &page_error) == HAL_OK && page_error == 0xFFFFFFFF) {
                            // 写入数据
                            for (uint32_t i = 0; i < FLASH_PAGE_SIZE; i += 2) {
                                uint16_t half_word = (uint16_t)page_buff[i] | (page_buff[i + 1] << 8);
                                if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, page_addr + i, half_word) != HAL_OK) {
                                    ret = RET_FAIL;
                                    break;
                                }
                            }
                            
                            if (ret == RET_FAIL) break;
                            else {
                                length -= page_end - page_begin + 1;
                                data += page_end - page_begin + 1;
                            }
                        }
                        else {
                            ret = RET_FAIL;
                            break;
                        }
                        HAL_FLASH_Lock();
                    }
                    else {
                        ret = RET_FAIL;
                        break;
                    }
                }
                vPortFree(page_buff);
            }
        }
        xSemaphoreGive(flash_mutex);
    }

    return ret;
}
