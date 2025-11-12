#include "task_test.h"
#include "task_log.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"

TestResource test1_res;
TestResource test2_res;

#define TEST_FILE_NAME      "test_64KB.bin"
#define TEST_FILE_SIZE      (64 * 1024)     // 64KB
#define TEST_WRITE_COUNT    5
#define BUFFER_SIZE         1024

#define TIME_WAIT_LONG      pdMS_TO_TICKS(1000)

extern FATFS fatfs;

void fileTestTask(void *param)
{
    FIL file;
    FRESULT fatfs_ret;
    UINT write_len, read_len;
    uint8_t buffer[BUFFER_SIZE];
    uint8_t verify_buf[BUFFER_SIZE];

    /* ---------------- 写入测试 ---------------- */
    for (int i = 0; i < TEST_WRITE_COUNT; i++) {
        const char *fname = TEST_FILE_NAME;
        uint32_t filesize = TEST_FILE_SIZE;

        LOGI("\n[%d/%d] 开始写入文件: %s", i + 1, TEST_WRITE_COUNT, fname);

        fatfs_ret = f_open(&file, fname, FA_CREATE_ALWAYS | FA_WRITE);
        if (fatfs_ret != FR_OK) {
            LOGE("打开文件失败 (%d)", fatfs_ret);
            vTaskDelay(TIME_WAIT_LONG);
            continue;
        }

        memset(buffer, 0xAA, sizeof(buffer));

        UINT total_written = 0;
        uint32_t start_tick = HAL_GetTick();

        while (total_written < filesize) {
            uint32_t write_size = MIN(filesize - total_written, sizeof(buffer));

            fatfs_ret = f_write(&file, buffer, write_size, &write_len);
            if (fatfs_ret != FR_OK || write_len != write_size) {
                LOGE("写入错误: ret=%d, write_len=%u", fatfs_ret, write_len);
                break;
            }

            total_written += write_len;
        }

        f_close(&file);

        uint32_t end_tick = HAL_GetTick();
        float elapsed_s = (end_tick - start_tick) / 1000.0f;
        float speed_kb_s = (total_written / 1024.0f) / elapsed_s;

        LOGI("写入完成: %u 字节, 用时 %.2fs, 速度 %.2f KB/s",
             total_written, elapsed_s, speed_kb_s);
    }

    /* ---------------- 读取测试 ---------------- */
    LOGI("\n开始读取测试...");

    fatfs_ret = f_open(&file, TEST_FILE_NAME, FA_READ);
    if (fatfs_ret != FR_OK) {
        LOGE("打开文件读取失败 (%d)", fatfs_ret);
        vTaskDelay(TIME_WAIT_LONG);
        vTaskDelete(NULL);
    }

    uint32_t total_read = 0;
    uint32_t start_tick = HAL_GetTick();

    memset(verify_buf, 0, sizeof(verify_buf));

    while (1) {
        fatfs_ret = f_read(&file, verify_buf, sizeof(verify_buf), &read_len);
        if (fatfs_ret != FR_OK) {
            LOGE("读取错误: %d", fatfs_ret);
            break;
        }

        if (read_len == 0) {
            // EOF
            break;
        }

        total_read += read_len;
    }

    f_close(&file);
    uint32_t end_tick = HAL_GetTick();

    float elapsed_s = (end_tick - start_tick) / 1000.0f;
    float speed_kb_s = (total_read / 1024.0f) / elapsed_s;

    LOGI("读取完成: %u 字节, 用时 %.2fs, 速度 %.2f KB/s",
         total_read, elapsed_s, speed_kb_s);

    LOGI("文件读写测试结束。");
    vTaskDelete(NULL);
}

/// @brief 测试各种模块
/// @param param 
void test1CoreTask(void *param) {
    TestResource *test_res = (TestResource*)param;
    UNUSED(test_res);

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
        // LOGI("\nname\t\tstate\tprio\tstack\tid\n%s", info);
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
    xTaskCreate(test1CoreTask, "test1_core", TASK_STACK_SMALL, &test1_res, TASK_PRIORITY_NORMAL, &test1_res.htask);

    test2_res.htask = NULL;
    xTaskCreate(test2CoreTask, "test2_core", TASK_STACK_SMALL, &test2_res, TASK_PRIORITY_NORMAL, &test2_res.htask);
}
