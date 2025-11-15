#include "at24c64.h"
#include "string.h"

I2C_HandleTypeDef *at24c64_hi2c;
SemaphoreHandle_t at24c64_mutex;

/// @brief AT24C64 初始化
/// @param hi2c I2C 句柄
void at24c64Init(I2C_HandleTypeDef *hi2c) {
    if (hi2c == NULL || at24c64_hi2c == hi2c) return;
    // 绑定外设
    at24c64_hi2c = hi2c;
    // 申请 IPC 资源
    at24c64_mutex = xSemaphoreCreateMutex();
    if (at24c64_mutex == NULL) {
        __disable_irq();
        while (1);
    }
}

/// @brief 读取指定地址数据
/// @param addr 
/// @param data 
/// @param length 
/// @return 
uint8_t at24c64ReadData(uint32_t addr, uint8_t *data, uint32_t length) {
    if (data == NULL || length == 0) return RET_FAIL;
    uint8_t ret = RET_FAIL;
    // 抢占 EEPROM
    if (xSemaphoreTake(at24c64_mutex, TIME_WAIT_MEDIUM) == pdTRUE) {
        // 设置起始地址
        uint8_t addr_buf[2] = { (addr >> 8) & 0xFF, addr & 0xFF };
        // 发送地址指针
        if (HAL_I2C_Master_Transmit(at24c64_hi2c, AT24C64_I2C_ADDR << 1, addr_buf, 2, 100) == HAL_OK) {
            // 连续读取 length 字节
            if (HAL_I2C_Master_Receive(at24c64_hi2c, AT24C64_I2C_ADDR << 1, data, length, 100) == HAL_OK) ret = RET_DONE;
        }
        xSemaphoreGive(at24c64_mutex);
    }
    return ret;
}

/// @brief 写入任意长度数据
/// @param addr 
/// @param data 
/// @param length 
/// @return 
uint8_t at24c64WriteData(uint32_t addr, uint8_t *data, uint32_t length) {
    if (data == NULL || length == 0) return RET_FAIL;
    uint8_t ret = RET_DONE;
    // 抢占 EEPROM
    if (xSemaphoreTake(at24c64_mutex, TIME_WAIT_MEDIUM) == pdTRUE) {
        while (length > 0) {
            // 当前页剩余空间
            uint8_t page_offset = addr % AT24C64_PAGE_SIZE;
            uint8_t space_in_page = AT24C64_PAGE_SIZE - page_offset;
            // 本次写入字节数
            uint8_t write_length = (length < space_in_page) ? length : space_in_page;
            // 组织地址 + 数据
            uint8_t tx_buf[2 + AT24C64_PAGE_SIZE];
            tx_buf[0] = (addr >> 8) & 0xFF;
            tx_buf[1] = addr & 0xFF;
            memcpy(&tx_buf[2], data, write_length);
            // 页写入：地址高低字节 + write_length 数据
            if (HAL_I2C_Master_Transmit(at24c64_hi2c, AT24C64_I2C_ADDR << 1, tx_buf, 2 + write_length, 100) != HAL_OK) {
                ret = RET_FAIL;
                break;
            }
            // 更新写位置
            addr += write_length;
            data += write_length;
            length -= write_length;
            // 等待写入完成
            vTaskDelay(10);
        }
        xSemaphoreGive(at24c64_mutex);
    }
    else ret = RET_FAIL;
    return ret;
}
