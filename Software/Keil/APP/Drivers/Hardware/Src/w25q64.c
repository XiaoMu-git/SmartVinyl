#include "w25q64.h"
#include "string.h"

/// @brief 等待芯片空闲
/// @param wait_time 
/// @return 返回剩余等待时间
uint32_t w25q64WaitIdle(uint32_t wait_time) {
    uint8_t buff[2] = { 0 };
    while (1) {
        buff[0] = W25Q64_CMD_READ_STATUS1;
        // 发送 1 字节命令, 接收 1 字节状态
        spi1SendRecvData(W25Q64_CS_GPIO, GPIO_PIN_4, buff, 1 + 1);
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
    buff[0] = W25Q64_CMD_WRITE_ENABLE;
    // 发送 1 字节命令
    spi1SendRecvData(W25Q64_CS_GPIO, GPIO_PIN_4, buff, 1);
    while (1) {
        buff[0] = W25Q64_CMD_READ_STATUS1;
        // 发送 1 字节命令, 接收 1 字节状态
        spi1SendRecvData(W25Q64_CS_GPIO, GPIO_PIN_4, buff, 2);
        if ((buff[1] & 0x02) == 0x02 || wait_time == 0) break;
        wait_time--;
        vTaskDelay(pdMS_TO_TICKS(1));
    }
    return wait_time;
}

/// @brief 擦除指定扇区
/// @param address 
/// @return 
BaseType_t w25q64EraseSector(uint32_t address) {
    uint8_t buff[4] = { 0 };
    // 等待写使能
    if (w25q64WaitWriteEnable(W25Q64_WAIT_TIME) > 0) {
        buff[0] = W25Q64_CMD_SECTOR_ERASE;
        buff[1] = (address >> 16) & 0xFF;
        buff[2] = (address >> 8) & 0xFF;
        buff[3] = (address >> 0) & 0xFF;
        // 发送 1 字节命令, 接收 3 字节地址
        return spi1SendRecvData(W25Q64_CS_GPIO, GPIO_PIN_4, buff, 1 + 3);
    }
    return pdFALSE;
}

/// @brief 擦除整颗芯片
/// @return 
BaseType_t w25q64EraseChip(void) {
    uint8_t buff[1] = { 0 };
    // 等待写使能
    if (w25q64WaitWriteEnable(W25Q64_WAIT_TIME) > 0) {
        buff[0] = W25Q64_CMD_CHIP_ERASE;
        // 发送 1 字节命令
        return spi1SendRecvData(W25Q64_CS_GPIO, GPIO_PIN_4, buff, 1);
    }
    return pdFALSE;
}

/// @brief 向指定块写入数据
/// @param address 
/// @param data 
/// @param length 
/// @return 
BaseType_t w25q64WritePage(uint32_t address, uint8_t *data, uint32_t length) {
    if (data == NULL || length == 0) return pdFALSE;
    uint32_t write_len = length > W25Q64_PAGE_SIZE ? W25Q64_PAGE_SIZE : length;
    // 申请内存
    uint8_t *buff = (uint8_t*)pvPortMalloc((write_len + 4) * sizeof(uint8_t));
    if (buff == NULL) return pdFALSE;
    BaseType_t result = pdFALSE;
    // 拷贝数据
    buff[0] = W25Q64_CMD_PAGE_PROGRAM;
    buff[1] = (address >> 16) & 0xFF;
    buff[2] = (address >> 8) & 0xFF;
    buff[3] = address & 0xFF;
    memcpy(buff + 4, data, write_len);
    // 等待写使能
    if (w25q64WaitWriteEnable(W25Q64_WAIT_TIME) > 0) {
        // 发送 1 字节命令, 发送 3 字节地址, 发送 length 字节数据
        result = spi1SendRecvData(W25Q64_CS_GPIO, GPIO_PIN_4, buff, write_len + 4);
    }
    vPortFree(buff);
    return result;
}

/// @brief 读取厂商信息
/// @return 
uint32_t w25q64ReadJEDECID(void) {
    uint8_t buff[4] = { 0 };
    buff[0] = W25Q64_CMD_READ_JEDECID;
    // 发送 1 字节命令, 接收 3 字节信息
    spi1SendRecvData(W25Q64_CS_GPIO, GPIO_PIN_4, buff, 1 + 3);
    uint32_t info = 0;
    for (int i = 1; i < 4; i++) info = info << 8 | buff[i];
    return info;
}

/// @brief 读取芯片唯一 ID
/// @return 
uint32_t w25q64ReadUID(void) {
    uint8_t buff[13] = { 0 };
    buff[0] = W25Q64_CMD_READ_UID;
    // 发送 1 字节命令, 发送 4 字节填充, 接收 8 字节 ID
    spi1SendRecvData(W25Q64_CS_GPIO, GPIO_PIN_4, buff, 1 + 4 + 8);
    uint32_t id = 0;
    for (int i = 9; i < 13; i++) id = id << 8 | buff[i];
    return id;
}

/// @brief 读取指定地址的数据
/// @param address 
/// @param data 
/// @param length 
/// @return 
BaseType_t w25q64Read(uint32_t address, uint8_t *data, uint32_t length) {
    if (data == NULL || length == 0) return pdFALSE;
    // 申请内存
    uint8_t *buff = (uint8_t*)pvPortMalloc((W25Q64_PAGE_SIZE + 4) * sizeof(uint8_t));
    if (buff == NULL) return pdFALSE;
    BaseType_t result = pdFALSE;
    while (length > 0) {
        // 计算数据长度
        uint32_t read_len = length > W25Q64_PAGE_SIZE ? W25Q64_PAGE_SIZE : length;
        buff[0] = W25Q64_CMD_READ_DATA;
        buff[1] = (address >> 16) & 0xFF;
        buff[2] = (address >> 8) & 0xFF;
        buff[3] = (address >> 0) & 0xFF;
        result = pdFALSE;
        // 等待设备空闲
        if (w25q64WaitIdle(W25Q64_WAIT_TIME) > 0) {
            // 发送 1 字节命令, 发送 3 字节地址, 接收 read_len 字节数据
            result = spi1SendRecvData(W25Q64_CS_GPIO, GPIO_PIN_4, buff, read_len + 4);
            // 拷贝数据
            if (result == pdTRUE) memcpy(data, buff + 4, read_len);
            else break;
            // 更新参数
            address += read_len;
            data += read_len;
            length -= read_len;
        }
        else break;
    }
    vPortFree(buff);
    return result;
}

/// @brief 向指定地址写入数据
/// @param address 
/// @param data 
/// @param length 
/// @return 
BaseType_t w25q64Write(uint32_t address, uint8_t *data, uint32_t length) {
    if (data == NULL || length == 0) return pdFALSE;
    // 申请内存
    uint8_t *buff = (uint8_t*)pvPortMalloc(W25Q64_SECTOR_SIZE);
    if (buff == NULL) return pdFALSE;
    BaseType_t result = pdFALSE;
    uint32_t offset = 0;
    // 计算涉及的扇区
    uint32_t sector_first = address / W25Q64_SECTOR_SIZE;
    uint32_t sector_last = (address + length - 1) / W25Q64_SECTOR_SIZE;
    for (uint32_t sector = sector_first; sector <= sector_last; sector++) {
        // 计算扇区首地址
        uint32_t sector_address = sector * W25Q64_SECTOR_SIZE;
        // 读取该扇区
        if (w25q64Read(sector_address, buff, W25Q64_SECTOR_SIZE) != pdTRUE) break;
        // 擦除该扇区
        if (w25q64EraseSector(sector_address) != pdTRUE) break;
        // 计算写入范围
        uint32_t write_start = (sector == sector_first) ? address % W25Q64_SECTOR_SIZE : 0;
        uint32_t write_end = (sector == sector_last) ? (address + length - 1) % W25Q64_SECTOR_SIZE : W25Q64_SECTOR_SIZE - 1;
        uint32_t write_len = write_end - write_start + 1;
        // 拷贝数据
        memcpy(buff + write_start, data + offset, write_len);
        result = pdFALSE;
        // 循环该扇区所有页面
        for (uint32_t page = 0; page < W25Q64_SECTOR_SIZE / W25Q64_PAGE_SIZE; page++) {
            uint32_t page_address = sector_address + page * W25Q64_PAGE_SIZE;
            // 写入页数据
            if (w25q64WritePage(page_address, buff + page * W25Q64_PAGE_SIZE, W25Q64_PAGE_SIZE) == pdTRUE) result = pdTRUE;
            else break;
        }
        if (result != pdTRUE) break;
        offset += write_len;
    }
    vPortFree(buff);
    return result;
}
