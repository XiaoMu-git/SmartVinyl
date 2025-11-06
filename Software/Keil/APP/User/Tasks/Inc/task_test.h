#ifndef _TASK_TEST_H_
#define _TASK_TEST_H_

#include "config.h"

typedef struct {
    TaskHandle_t htask;
} TestResource;

void createTestTask(void);

#endif // !_TASK_TEST_H_
