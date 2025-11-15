#include "task_test.h"
#include "task_log.h"
#include "tb6612.h"

TestResource test1_res;
TestResource test2_res;

/// @brief 测试各种模块
/// @param param 
void test1CoreTask(void *param) {
    TestResource *test_res = (TestResource*)param;
    UNUSED(test_res);

    for (int i = -100; i <= 100; i++) {
        if (i < 0) tb6612SetSpeed(-i);
        else  tb6612SetSpeed(i);
        tb6612SetDirection(i);
        vTaskDelay(100);
    }
    tb6612SetSpeed(0);

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
