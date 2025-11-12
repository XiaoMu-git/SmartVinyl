#include "at24c64.h"
#include "string.h"

static I2C_HandleTypeDef *i2c_handle = NULL;

void AT24C64_Init(I2C_HandleTypeDef *hi2c) {
    i2c_handle = hi2c;
}

bool AT24C64_WriteByte(uint16_t addr, uint8_t data) {
    return AT24C64_Write(addr, &data, 1);
}

bool AT24C64_ReadByte(uint16_t addr, uint8_t *data) {
    return AT24C64_Read(addr, data, 1);
}

static bool AT24C64_WritePage(uint16_t addr, const uint8_t *data, uint8_t len) {
    uint8_t buf[2 + AT24C64_PAGE_SIZE];

    buf[0] = (addr >> 8) & 0xFF;
    buf[1] = addr & 0xFF;
    memcpy(&buf[2], data, len);

    if (HAL_I2C_Master_Transmit(i2c_handle, AT24C64_I2C_ADDR, buf, len + 2, HAL_MAX_DELAY) != HAL_OK) {
        return false;
    }

    HAL_Delay(5);  // 写入完成延时
    return true;
}

bool AT24C64_Write(uint16_t addr, const uint8_t *data, uint16_t len) {
    while (len > 0) {
        uint8_t page_offset = addr % AT24C64_PAGE_SIZE;
        uint8_t space_in_page = AT24C64_PAGE_SIZE - page_offset;
        uint8_t write_len = (len < space_in_page) ? len : space_in_page;

        if (!AT24C64_WritePage(addr, data, write_len)) {
            return false;
        }

        addr += write_len;
        data += write_len;
        len  -= write_len;
    }
    return true;
}

bool AT24C64_Read(uint16_t addr, uint8_t *data, uint16_t len) {
    uint8_t addr_buf[2] = { (uint8_t)(addr >> 8), (uint8_t)(addr & 0xFF) };

    if (HAL_I2C_Master_Transmit(i2c_handle, AT24C64_I2C_ADDR, addr_buf, 2, HAL_MAX_DELAY) != HAL_OK) {
        return false;
    }

    if (HAL_I2C_Master_Receive(i2c_handle, AT24C64_I2C_ADDR, data, len, HAL_MAX_DELAY) != HAL_OK) {
        return false;
    }

    return true;
}
