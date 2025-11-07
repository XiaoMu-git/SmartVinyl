#include "task_log.h"
#include "stdarg.h"
#include "stdio.h"
#include "string.h"
#include "uart.h"

LogResource log_res;

/// @brief log 发送任务
/// @param param 
void logCoreTask(void *param) {
    LogResource *log_res = (LogResource*)param;
    LogPacket packet = { 0 };

    while (1) {
        xQueueReceive(log_res->queue, &packet, portMAX_DELAY);
        if (packet.content == NULL) continue;
        HAL_UART_Transmit(&huart1, (uint8_t*)packet.content, packet.length, 100);
        vPortFree(packet.content);
        packet.content = NULL;
    }
}

/// @brief 创建日志任务
void createLogTask(void) {
    log_res.htask = NULL;
    log_res.queue = xQueueCreate(QUEUE_SIZE_LARGE, sizeof(LogPacket));
    log_res.mutex = xSemaphoreCreateMutex();
    log_res.buff = (uint8_t*)pvPortMalloc(LOG_BUFF_SIZE * sizeof(uint8_t));
    xTaskCreate(logCoreTask, "log_core", TASK_STACK_TINY, &log_res, TASK_PRIORITY_HIGH, &log_res.htask);
}

/// @brief 封装发送日志接口
/// @param fmt 
/// @param  
/// @return 
uint8_t logSendData(const char *fmt, ...) {
    if (fmt == NULL) return pdFALSE;
    uint8_t ret = RET_FAIL;
    LogPacket packet = { 0 };

    if (xSemaphoreTake(log_res.mutex, TIME_WAIT_MEDIUM) == pdTRUE) {
        va_list args;
        va_start(args, fmt);
        packet.length = vsnprintf((char*)log_res.buff, LOG_BUFF_SIZE, fmt, args);
        va_end(args);

        packet.content = (char*)pvPortMalloc(packet.length);
        if (packet.content != NULL) {
            memcpy(packet.content, log_res.buff, packet.length);
            if (xQueueSend(log_res.queue, &packet, TIME_WAIT_SHORT) == pdTRUE) ret = RET_DONE;
            else vPortFree(packet.content);
        }

        xSemaphoreGive(log_res.mutex);
    }
    
    return ret;
}
