#include "task_test.h"
#include "task_log.h"
#include "string.h"
#include "w25q64.h"

TestResource test1_res;
TestResource test2_res;

/// @brief 测试各种模块
/// @param param 
void test1CoreTask(void *param) {
    TestResource *test_res = (TestResource*)param;
    UNUSED(test_res);

    const uint32_t addr = 0x08010000;

    HAL_FLASH_Unlock(); // 解锁Flash
    // 1️⃣ 读取原始数据
    uint16_t read_data = *(__IO uint16_t*)addr;
    LOGI("Before erase, data = 0x%04X\r\n", read_data);

    // 2️⃣ 擦除页面
    FLASH_EraseInitTypeDef EraseInitStruct;
    uint32_t PageError = 0;
    EraseInitStruct.TypeErase = FLASH_TYPEERASE_PAGES;
    EraseInitStruct.PageAddress = addr;
    EraseInitStruct.NbPages = 1;

    if(HAL_FLASHEx_Erase(&EraseInitStruct, &PageError) != HAL_OK)
    {
        LOGI("Flash erase error at page: 0x%08lX\r\n", PageError);
        HAL_FLASH_Lock();
        return;
    }

    // 3️⃣ 写入数据
    uint16_t write_data = 0xAAAA;
    if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD, addr, write_data) != HAL_OK)
    {
        LOGI("Flash write error!\r\n");
        HAL_FLASH_Lock();
        return;
    }

    HAL_FLASH_Lock(); // 锁定Flash

    // 4️⃣ 读取验证
    uint16_t verify_data = *(__IO uint16_t*)addr;
    LOGI("After write, data = 0x%04X\r\n", verify_data);

    while (1) {
        vTaskDelay(TIME_WAIT_LONG);
    }
}

/// @brief led 闪烁提示并打印任务信息和剩余堆资源
/// @param param 
void test2CoreTask(void *param) {
    TestResource *test_res = (TestResource*)param;
    UNUSED(test_res);
    char info[512];

    while (1) {
        // 获取任务状态和剩余堆栈
        vTaskList(info);
        // LOGI("\nname\t\tstate\tprio\tstack\tid\n%sstack space remain %u byte.\n", info, (unsigned int)xPortGetFreeHeapSize());
        
        // LED 闪烁
        for (uint8_t i = 0; i < 10; i++) {
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
    xTaskCreate(test1CoreTask, "test1_core", TASK_STACK_MEDIUM, &test1_res, TASK_PRIORITY_LOW, &test1_res.htask);
    xTaskCreate(test2CoreTask, "test2_core", TASK_STACK_SMALL, &test2_res, TASK_PRIORITY_NORMAL, &test2_res.htask);
}
