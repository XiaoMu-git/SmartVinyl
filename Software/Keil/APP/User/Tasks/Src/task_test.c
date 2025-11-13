#include "task_test.h"
#include "task_log.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"

TestResource test1_res;
TestResource test2_res;

char fatfs_path[] = "";
FATFS fatfs;
FIL file;
FRESULT fatfs_ret;
UINT read_len, write_len;

/// @brief 测试各种模块
/// @param param 
void test1CoreTask(void *param) {
    TestResource *test_res = (TestResource*)param;
    UNUSED(test_res);

    // 初始化 FATFS
    if (FATFS_LinkDriver(&SD_Driver, fatfs_path) != HAL_OK) {
        __disable_irq();
        while(1);
    }

    // 挂载 FATFS
    if (f_mount(&fatfs, fatfs_path, 1) != FR_OK) {
        __disable_irq();
        while(1);
    }

    /* ---------------- 写入测试 ---------------- */
    for(int i = 0; i < 5; i++) {
        const char* fname = "test_64KB.bin";
        uint32_t filesize = 64 * 1024;

        fatfs_ret = f_open(&file, fname, FA_CREATE_ALWAYS | FA_WRITE);
        if(fatfs_ret != FR_OK) {
            LOGI("Open %s Failed: %d", fname, fatfs_ret);
            while (1) {
                vTaskDelay(TIME_WAIT_LONG);
            }
        }

        uint8_t buffer[1024];
        memset(buffer, 0xAA, sizeof(buffer));

        UINT total_written = 0;
        uint32_t start_tick = HAL_GetTick();

        while(total_written < filesize) {
            uint32_t write_size = (filesize - total_written > sizeof(buffer)) ? sizeof(buffer) : (filesize - total_written);
            fatfs_ret = f_write(&file, buffer, write_size, &write_len);
            if(fatfs_ret != FR_OK || write_len != write_size) {
                LOGI("Write error in %s", fname);
            while (1) {
                vTaskDelay(TIME_WAIT_LONG);
            }
            }
            total_written += write_len;
        }

        uint32_t end_tick = HAL_GetTick();
        f_close(&file);

        float elapsed_s = (end_tick - start_tick) / 1000.0f;
        float speed_kb_s = (total_written / 1024.0f) / elapsed_s;

        LOGI("%s write done: %u bytes, time %.2fs, speed %.2f KB/s",
            fname, total_written, elapsed_s, speed_kb_s);
    }

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
    xTaskCreate(test1CoreTask, "test1_core", TASK_STACK_LARGE, &test1_res, TASK_PRIORITY_NORMAL, &test1_res.htask);

    test2_res.htask = NULL;
    xTaskCreate(test2CoreTask, "test2_core", TASK_STACK_SMALL, &test2_res, TASK_PRIORITY_NORMAL, &test2_res.htask);
}
