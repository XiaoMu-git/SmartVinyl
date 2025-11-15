#include "task_test.h"
#include "task_log.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"

TestResource test1_res;
TestResource test2_res;

void fatfsTest(void) {
    FIL file;
    FRESULT fatfs_ret;
    UINT read_len, write_len;
    
    for(int i = 0; i < 5; i++) {
        const char* fname = "test_64KB.bin";
        uint32_t filesize = 64 * 1024;

        fatfs_ret = f_open(&file, fname, FA_CREATE_ALWAYS | FA_WRITE);
        if(fatfs_ret != FR_OK) {
            LOGI("Open %s Failed: %d", fname, fatfs_ret);
            continue;
        }

        uint8_t buffer[1024];
        memset(buffer, 0xAA, sizeof(buffer));

        UINT total_written = 0;
        uint32_t start_tick = HAL_GetTick();

        while(total_written < filesize) {
            uint32_t write_size = filesize - total_written > sizeof(buffer) ? sizeof(buffer) : filesize - total_written;
            fatfs_ret = f_write(&file, buffer, write_size, &write_len);
            if(fatfs_ret != FR_OK || write_len != write_size) {
                LOGI("Write error in %s", fname);
                continue;
            }
            total_written += write_len;
        }

        uint32_t end_tick = HAL_GetTick();
        f_close(&file);

        float elapsed_s = (end_tick - start_tick) / 1000.0f;
        float speed_kb_s = (total_written / 1024.0f) / elapsed_s;

        LOGI("%s write done: %u bytes, time %.2fs, speed %.2f KB/s", fname, total_written, elapsed_s, speed_kb_s);
    }
}

void at24c64Test(void) {
    const uint32_t test_addr = 0x0123;
    const uint32_t test_len  = 64;    // 测试 64 字节
    uint8_t *write_buf = (uint8_t*)pvPortMalloc(test_len);
    uint8_t *read_buf = (uint8_t*)pvPortMalloc(test_len);

    // 生成测试数据
    for (uint32_t i = 0; i < test_len; i++) {
        write_buf[i] = (uint8_t)(0x5A + i);   // 任意模式数据
    }

    LOGI("=== AT24C64 Test Start ===");
    LOGI("Write Addr: 0x%04X, Size: %d bytes", test_addr, test_len);

    // 写入测试
    if (at24c64WriteData(test_addr, write_buf, test_len) == RET_DONE) {
        LOGI("Write: OK");
    } else {
        LOGI("Write: FAIL");
        return;
    }
    // 读取测试
    if (at24c64ReadData(test_addr, read_buf, test_len) == RET_DONE) {
        LOGI("Read: OK");
    } else {
        LOGI("Read: FAIL");
        return;
    }

    // 校验数据
    LOGI("Verifying...");

    for (uint32_t i = 0; i < test_len; i++) {
        if (write_buf[i] != read_buf[i]) {
            LOGI("Verify FAIL at index %lu: W=0x%02X, R=0x%02X",
                i, write_buf[i], read_buf[i]);
            LOGI("=== AT24C64 Test End ===");
            return;
        }
    }

    LOGI("DATA VERIFIED OK");
    LOGI("=== AT24C64 Test End ===");
}

/// @brief 测试各种模块
/// @param param 
void test1CoreTask(void *param) {
    TestResource *test_res = (TestResource*)param;
    UNUSED(test_res);

    for (uint8_t addr = 0; addr < 128; addr++) {
        if (HAL_I2C_IsDeviceReady(&hi2c1, addr << 1, 1, 10) == HAL_OK) {
            LOGI("Found device at 0x%02X", addr);
        }
    }

    // fatfsTest();
    at24c64Test();

    while (1) {
        vTaskDelay(TIME_WAIT_LONG);
    }
}
 
/// @brief led 闪烁提示并打印任务信息和剩余堆资源
/// @param param 
void test2CoreTask(void *param) {
    TestResource *test_res = (TestResource*)param;
    UNUSED(test_res);
    char info[256];

    while (1) {
        vTaskList(info);
        // LOGI("name\t\tstate\tprio\tstack\tid%s", info);
        LOGI("Stack space remain %u Byte.", (unsigned int)xPortGetFreeHeapSize());
        for (uint8_t i = 0; i < 5; i++) {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
            vTaskDelay(pdMS_TO_TICKS(200));
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
            vTaskDelay(pdMS_TO_TICKS(800));
        }
    }
}

/// @brief 创建测试任务
/// @param  
void createTestTask(void) {
    test1_res.htask = NULL;
    xTaskCreate(test1CoreTask, "test1_core", TASK_STACK_LARGE, &test1_res, TASK_PRIORITY_NORMAL, &test1_res.htask);

    test2_res.htask = NULL;
    xTaskCreate(test2CoreTask, "test2_core", TASK_STACK_SMALL, &test2_res, TASK_PRIORITY_NORMAL, &test2_res.htask);
}
