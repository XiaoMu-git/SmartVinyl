#include "task_file.h"
#include "task_log.h"
#include "ff_gen_drv.h"
#include "sd_diskio.h"

FileResource file_res;

/// @brief 操作文件的任务
/// @param param 
void fileCoreTask(void *param) {
    FileResource *file_res = (LogResource*)param;
    FilePacket packet = { 0 };

    char fatfs_path[] = "";
    FATFS fatfs;
    FIL file;
    FRESULT fatfs_ret;
    UINT read_len, write_len;

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

    while (1) {
        xQueueReceive(file_res->queue, &packet, portMAX_DELAY);
        if (packet.option == FILE_OPTION_WRITE) {
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
        }
        else if (packet.option == FILE_OPTION_READ) {

        }
    }
}

/// @brief 创建文件操作任务
/// @param  
void createFileTask(void) {
    file_res.htask = NULL;
    file_res.queue = xQueueCreate(QUEUE_SIZE_LARGE, sizeof(FilePacket));
    file_res.mutex = xSemaphoreCreateMutex();
    file_res.buff = (uint8_t*)pvPortMalloc(FILE_BUFF_SIZE * sizeof(uint8_t));
    xTaskCreate(fileCoreTask, "file_core", TASK_STACK_LARGE, &file_res, TASK_PRIORITY_NORMAL, &file_res.htask);
}

/// @brief 写入数据
/// @param path 
/// @param data 
/// @param length 
/// @param offset 
/// @return 
uint8_t fileWrite(const char *path, uint8_t *data, uint32_t length, uint32_t offset) {

}

/// @brief 读取数据
/// @param path 
/// @param data 
/// @param length 
/// @param offset 
/// @return 
uint8_t fileRead(const char *path, uint8_t *data, uint32_t length, uint32_t offset) {

}
