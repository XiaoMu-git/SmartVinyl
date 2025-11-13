#ifndef _TASK_CONFIG_H_
#define _TASK_CONFIG_H_

#include "stm32f1xx_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "semphr.h"

#define UART1_BUFF_SIZE             (uint32_t)64
#define I2C1_BUFF_SIZE              (uint32_t)64
#define SPI1_BUFF_SIZE              (uint32_t)64
#define USB_BUFF_SIZE               (uint32_t)64

// 函数返回值
#define RET_DONE                    (uint8_t)1
#define RET_FAIL                    (uint8_t)0

// 任务栈大小(单位:word)
#define TASK_STACK_TINY             128
#define TASK_STACK_SMALL            256
#define TASK_STACK_MEDIUM           512
#define TASK_STACK_LARGE            1024

// 任务优先级([0, 4])
#define TASK_PRIORITY_REALTIME      4
#define TASK_PRIORITY_HIGH          3
#define TASK_PRIORITY_NORMAL        2
#define TASK_PRIORITY_LOW           1
#define TASK_PRIORITY_IDLE          0

// 队列大小
#define QUEUE_SIZE_SMALL            4
#define QUEUE_SIZE_MEDIUM           8
#define QUEUE_SIZE_LARGE            16

// 操作等待时间
#define TIME_WAIT_SHORT             pdMS_TO_TICKS(10)
#define TIME_WAIT_MEDIUM            pdMS_TO_TICKS(100)
#define TIME_WAIT_LONG              pdMS_TO_TICKS(1000)

#endif // !_TASK_CONFIG_H_
