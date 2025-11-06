#include "task_log.h"
#include "stdarg.h"
#include "string.h"
#include "usb.h"

LogResource log_res;

/// @brief log 发送任务
/// @param param 
void logCoreTask(void *param) {
    LogResource *log_res = (LogResource*)param;
    LogPacket packet = { 0 };

    while (1) {
        xQueueReceive(log_res->queue, &packet, portMAX_DELAY);
        if (packet.content == NULL) continue;
        usbSendData((uint8_t*)packet.content, strlen(packet.content));
        vPortFree(packet.content);
        packet.content = NULL;
    }
}

/// @brief 创建日志任务
void createLogTask(void) {
    // 分配资源
    log_res.queue = xQueueCreate(QUEUE_SIZE_LARGE, sizeof(LogPacket));
    // 创建任务
    xTaskCreate(logCoreTask, "log_core", TASK_STACK_TINY, &log_res, TASK_PRIORITY_HIGH, &log_res.htask);
}

/// @brief 封装发送日志接口
/// @param fmt 
/// @param  
/// @return 
BaseType_t logSendData(const char *fmt, ...) {
    if (fmt == NULL) return pdFALSE;
    BaseType_t ret = pdFALSE;
    LogPacket packet = { 0 };
    packet.content = (char*)pvPortMalloc(LOG_BUFF_SIZE * sizeof(char));
    if (packet.content != NULL) {
        va_list args;
        va_start(args, fmt);
        vsnprintf(packet.content, LOG_BUFF_SIZE, fmt, args);
        va_end(args);
        ret = xQueueSend(log_res.queue, &packet, TIME_WAIT_SHORT);
        if (ret == pdFALSE) vPortFree(packet.content);
    }
    return ret;
}
