#include "w25q64.h"
#include "string.h"

SPI_HandleTypeDef *w25q64_hspi;
GPIO_TypeDef *w25q64_gpio;
uint16_t w25q64_pin;
SemaphoreHandle_t w25q64_mutex;

/// @brief 等待芯片空闲
/// @param wait_time 
/// @return 返回剩余等待时间
uint32_t w25q64WaitIdle(uint32_t wait_time) {
    uint8_t buff[2] = { 0 };
    while (1) {
        // 读取寄存器
        buff[0] = W25Q64_CMD_READ_STATUS1;
        spiSendRecvData(w25q64_hspi, w25q64_gpio, w25q64_pin, buff, 1 + 1);
        if ((buff[1] & 0x01) == 0x00 || wait_time == 0) break;
        wait_time--;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    return wait_time;
}

/// @brief 等待芯片写使能
/// @param wait_time 
/// @return 返回剩余等待时间
uint32_t w25q64WaitWriteEnable(uint32_t wait_time) {
    uint8_t buff[2] = { 0 };
    // 等待芯片空闲
    wait_time = w25q64WaitIdle(wait_time);
    // 请求进入写使能
    buff[0] = W25Q64_CMD_WRITE_ENABLE;
    spiSendRecvData(w25q64_hspi, w25q64_gpio, w25q64_pin, buff, 1);
    while (1) {
        // 读取寄存器
        buff[0] = W25Q64_CMD_READ_STATUS1;
        spiSendRecvData(w25q64_hspi, w25q64_gpio, w25q64_pin, buff, 2);
        if ((buff[1] & 0x02) == 0x02 || wait_time == 0) break;
        wait_time--;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    return wait_time;
}

/// @brief w25q64 初始化
/// @param  
void w25q64Init(SPI_HandleTypeDef *hspi, GPIO_TypeDef *gpio, uint16_t pin) {
    if (hspi == NULL || gpio == NULL || w25q64_hspi == hspi) return;
    // 绑定外设
    w25q64_hspi = hspi;
    w25q64_gpio = gpio;
    w25q64_pin = pin;
    // 申请 IPC 资源
    w25q64_mutex = xSemaphoreCreateMutex();
    if (w25q64_mutex == NULL) {
        __disable_irq();
        while (1);
    }
}

/// @brief 读取厂商信息
/// @return 
uint32_t w25q64ReadJEDECID(void) {
    uint8_t buff[4] = { 0 };
    uint32_t info = 0;
    // 抢占 w25q64
    if (xSemaphoreTake(w25q64_mutex, TIME_WAIT_MEDIUM) == pdTRUE) {
        // 读取厂商信息
        buff[0] = W25Q64_CMD_READ_JEDECID;
        spiSendRecvData(w25q64_hspi, w25q64_gpio, w25q64_pin, buff, 1 + 3);
        for (int i = 1; i < 4; i++) info = info << 8 | buff[i];
        xSemaphoreGive(w25q64_mutex);
    }
    return info;
}

/// @brief 擦除指定扇区
/// @param address 
/// @return 
uint8_t w25q64EraseSector(uint32_t address) {
    uint8_t buff[4] = { 0 };
    uint8_t ret = RET_FAIL;
    // 抢占 w25q64
    if (xSemaphoreTake(w25q64_mutex, TIME_WAIT_MEDIUM) == pdTRUE) {
        // 等待写使能
        if (w25q64WaitWriteEnable(W25Q64_WAIT_TIME) > 0) {
            // 发送擦除命令
            buff[0] = W25Q64_CMD_SECTOR_ERASE;
            buff[1] = (address >> 16) & 0xFF;
            buff[2] = (address >> 8) & 0xFF;
            buff[3] = (address >> 0) & 0xFF;
            ret = spiSendRecvData(w25q64_hspi, w25q64_gpio, w25q64_pin, buff, 1 + 3);
        }
        xSemaphoreGive(w25q64_mutex);
    }
    return ret;
}

/// @brief 擦除整颗芯片
/// @return 
uint8_t w25q64EraseChip(void) {
    uint8_t buff[1] = { 0 };
    uint8_t ret = RET_FAIL;
    // 抢占 w25q64
    if (xSemaphoreTake(w25q64_mutex, TIME_WAIT_MEDIUM) == pdTRUE) {
        // 等待写使能
        if (w25q64WaitWriteEnable(W25Q64_WAIT_TIME) > 0) {
            // 发送擦除命令
            buff[0] = W25Q64_CMD_CHIP_ERASE;
            ret = spiSendRecvData(w25q64_hspi, w25q64_gpio, w25q64_pin, buff, 1);
        }
        xSemaphoreGive(w25q64_mutex);
    }
    return ret;
}

/// @brief 读取指定地址的数据
/// @param address 
/// @param data 
/// @param length 
/// @return 
uint8_t w25q64Read(uint32_t address, uint8_t *data, uint32_t length) {
    if (data == NULL) return RET_FAIL;
    uint8_t ret = RET_FAIL;
    // 抢占 w25q64
    if (xSemaphoreTake(w25q64_mutex, TIME_WAIT_MEDIUM) == pdTRUE) {
        // 申请缓存区
        uint8_t *buff = (uint8_t*)pvPortMalloc(W25Q64_PAGE_SIZE + 4);
        if (buff != NULL) {
            while (length > 0) {
                buff[0] = W25Q64_CMD_READ_DATA;
                buff[1] = (address >> 16) & 0xFF;
                buff[2] = (address >> 8) & 0xFF;
                buff[3] = (address >> 0) & 0xFF;
                uint32_t read_length = length <= W25Q64_PAGE_SIZE ? length : W25Q64_PAGE_SIZE;
                // 等待设备空闲
                if (w25q64WaitIdle(W25Q64_WAIT_TIME) > 0) {
                    // 收发数据
                    ret = spiSendRecvData(w25q64_hspi, w25q64_gpio, w25q64_pin, buff, read_length + 4);
                    // 拷贝数据
                    if (ret == RET_DONE) memcpy(data, buff + 4, read_length);
                    else break;
                    // 更新参数
                    address += read_length;
                    data += read_length;
                    length -= read_length;
                }
                else {
                    ret = RET_FAIL;
                    break;
                }
            }
            vPortFree(buff);
        }
        xSemaphoreGive(w25q64_mutex);
    }
    return ret;
}

/// @brief 向指定地址写入数据
/// @param address 起始地址
/// @param data    数据缓冲区
/// @param length  数据长度
/// @return RET_DONE 成功, RET_FAIL 失败
uint8_t w25q64Write(uint32_t address, uint8_t *data, uint32_t length) {
    if (data == NULL || length == 0) return RET_FAIL;
    uint8_t ret = RET_FAIL;
    // 抢占 w25q64
    if (xSemaphoreTake(w25q64_mutex, TIME_WAIT_MEDIUM) == pdTRUE) {
        // 申请缓存区: 扇区缓存 + 页缓存
        uint8_t *sector_buff = (uint8_t*)pvPortMalloc(W25Q64_SECTOR_SIZE + 4);
        uint8_t *page_buff = (uint8_t*)pvPortMalloc(W25Q64_PAGE_SIZE + 4);
        if (sector_buff != NULL && page_buff != NULL) {
            // 计算涉及的扇区范围
            uint32_t sector_first = address / W25Q64_SECTOR_SIZE;
            uint32_t sector_last = (address + length - 1) / W25Q64_SECTOR_SIZE;
            ret = RET_DONE;
            for (uint32_t sector = sector_first; sector <= sector_last; sector++) {
                // 计算当前扇区首地址
                uint32_t sector_address = sector * W25Q64_SECTOR_SIZE;
                // 读取整个扇区数据到缓存 (sector_buff[4..] 为有效数据)
                sector_buff[0] = W25Q64_CMD_READ_DATA;
                sector_buff[1] = (sector_address >> 16) & 0xFF;
                sector_buff[2] = (sector_address >> 8)  & 0xFF;
                sector_buff[3] = (sector_address >> 0)  & 0xFF;
                if (w25q64WaitIdle(W25Q64_WAIT_TIME) == 0) {
                    ret = RET_FAIL;
                    break;
                }
                if (spiSendRecvData(w25q64_hspi, w25q64_gpio, w25q64_pin, sector_buff, W25Q64_SECTOR_SIZE + 4) != RET_DONE) {
                    ret = RET_FAIL;
                    break;
                }
                // 擦除当前扇区
                if (w25q64WaitWriteEnable(W25Q64_WAIT_TIME) == 0) {
                    ret = RET_FAIL;
                    break;
                }
                page_buff[0] = W25Q64_CMD_SECTOR_ERASE;
                page_buff[1] = (sector_address >> 16) & 0xFF;
                page_buff[2] = (sector_address >> 8)  & 0xFF;
                page_buff[3] = (sector_address >> 0)  & 0xFF;
                if (spiSendRecvData(w25q64_hspi, w25q64_gpio, w25q64_pin, page_buff, 4) != RET_DONE) {
                    ret = RET_FAIL;
                    break;
                }
                if (w25q64WaitIdle(W25Q64_WAIT_TIME) == 0) {
                    ret = RET_FAIL;
                    break;
                }
                // 计算当前扇区内写入的起止位置
                uint32_t write_begin = sector == sector_first ? address % W25Q64_SECTOR_SIZE : 0;
                uint32_t write_end = sector == sector_last ? (address + length - 1) % W25Q64_SECTOR_SIZE : W25Q64_SECTOR_SIZE - 1;
                uint32_t write_length = write_end - write_begin + 1;
                // 将用户数据覆盖到扇区缓存
                memcpy(sector_buff + 4 + write_begin, data, write_length);
                data += write_length;
                // 将整个扇区按页写回 Flash
                for (uint32_t page = 0; page < (W25Q64_SECTOR_SIZE / W25Q64_PAGE_SIZE); page++) {
                    uint32_t page_address = sector_address + page * W25Q64_PAGE_SIZE;
                    page_buff[0] = W25Q64_CMD_PAGE_PROGRAM;
                    page_buff[1] = (page_address >> 16) & 0xFF;
                    page_buff[2] = (page_address >> 8)  & 0xFF;
                    page_buff[3] = (page_address >> 0)  & 0xFF;
                    // 拷贝一页数据到发送缓冲区
                    memcpy(page_buff + 4, sector_buff + 4 + page * W25Q64_PAGE_SIZE, W25Q64_PAGE_SIZE);
                    // 使能写入并发送页编程指令
                    if (w25q64WaitWriteEnable(W25Q64_WAIT_TIME) == 0) {
                        ret = RET_FAIL;
                        break;
                    }
                    if (spiSendRecvData(w25q64_hspi, w25q64_gpio, w25q64_pin, page_buff, W25Q64_PAGE_SIZE + 4) != RET_DONE) {
                        ret = RET_FAIL;
                        break;
                    }
                    // 等待当前页编程完成
                    if (w25q64WaitIdle(W25Q64_WAIT_TIME) == 0) {
                        ret = RET_FAIL;
                        break;
                    }
                }
                if (ret != RET_DONE) break;
            }
        }
        // 释放缓存区
        if (page_buff != NULL) vPortFree(page_buff);
        if (sector_buff != NULL) vPortFree(sector_buff);
        // 释放 w25q64
        xSemaphoreGive(w25q64_mutex);
    }

    return ret;
}
