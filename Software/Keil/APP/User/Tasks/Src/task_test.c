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
        for (uint8_t i = 0; i < 5; i++) {
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
            vTaskDelay(pdMS_TO_TICKS(200));
            HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
            vTaskDelay(pdMS_TO_TICKS(800));
        }
        vTaskList(info);
        LOGI("\nname\t\tstate\tprio\tstack\tid\n%sstack space remain %u byte.\n", info, (unsigned int)xPortGetFreeHeapSize());
        
    }
}

/// @brief 创建测试任务
/// @param  
void createTestTask(void) {
    xTaskCreate(test1CoreTask, "test1_core", TASK_STACK_MEDIUM, &test1_res, TASK_PRIORITY_LOW, &test1_res.htask);
    xTaskCreate(test2CoreTask, "test2_core", TASK_STACK_SMALL, &test2_res, TASK_PRIORITY_NORMAL, &test2_res.htask);
}
