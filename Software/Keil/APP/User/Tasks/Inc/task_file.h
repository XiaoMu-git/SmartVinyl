#ifndef _TASK_FILE_H_
#define _TASK_FILE_H_

#include "config.h"

#define FILE_BUFF_SIZE      (uint32_t)1024

enum FILE_OPT {
    FILE_OPTION_WRITE = 0x00,
    FILE_OPTION_READ
};

typedef struct {
    TaskHandle_t htask;
    QueueHandle_t queue;
    SemaphoreHandle_t mutex;
    uint8_t *buff;
} FileResource;

typedef struct {
    QueueHandle_t queue;
    uint8_t option;
    uint8_t *payload;
    uint32_t length;
    uint8_t result;
    char path[16];
} FilePacket;

void createFileTask(void);
uint8_t fileWrite(const char *path, uint8_t *data, uint32_t length, uint32_t offset);
uint8_t fileRead(const char *path, uint8_t *data, uint32_t length, uint32_t offset);

#endif // !_TASK_FILE_H_
