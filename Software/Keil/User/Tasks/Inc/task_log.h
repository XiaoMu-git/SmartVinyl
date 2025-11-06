#ifndef _TASK_LOG_H_
#define _TASK_LOG_H_

#include "config.h"

#define LOG_BUFF_SIZE       (uint32_t)512

#define LOG(fmt, ...)       logSendData(fmt, ##__VA_ARGS__)
#define LOGI(fmt, ...)      logSendData("(%07.3f)[INFO]<%s> " fmt, (float)xTaskGetTickCount() / configTICK_RATE_HZ, pcTaskGetName(NULL), ##__VA_ARGS__)
#define LOGW(fmt, ...)      logSendData("(%07.3f)[WARN]<%s> " fmt, (float)xTaskGetTickCount() / configTICK_RATE_HZ, pcTaskGetName(NULL), ##__VA_ARGS__)
#define LOGE(fmt, ...)      logSendData("(%07.3f)[ERRO]<%s> " fmt, (float)xTaskGetTickCount() / configTICK_RATE_HZ, pcTaskGetName(NULL), ##__VA_ARGS__)

typedef struct {
    TaskHandle_t htask;
    QueueHandle_t queue;
} LogResource;

typedef struct {
    char *content;
} LogPacket;

void createLogTask(void);
BaseType_t logSendData(const char *fmt, ...);

#endif // !_TASK_LOG_H_
