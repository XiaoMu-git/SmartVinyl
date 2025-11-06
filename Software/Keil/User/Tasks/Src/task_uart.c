#include "task_uart.h"
#include "string.h"
#include "uart.h"

UartResource uart1_res;

/// @brief uart1 接收数据任务函数
/// @param param 
void uart1RxTask(void *param) {
    UartResource *uart_res = (UartResource*)param;
    UartPacket packet = { 0 };

    while (1) {
        xQueueReceive(uart_res->queue_rx, &packet, portMAX_DELAY);
        if (packet.payload == NULL) continue;
        // uartSendData(&huart1, packet.payload, packet.length);
        vPortFree(packet.payload);
        packet.payload = NULL;
    }
}

/// @brief 创建串口任务
void createUartTask(void) {
    // 分配资源
    uart1_res.queue_rx = xQueueCreate(QUEUE_SIZE_LARGE, sizeof(UartPacket));
    // 创建任务
    xTaskCreate(uart1RxTask, "uart1_rx", TASK_STACK_TINY, &uart1_res, TASK_PRIORITY_REALTIME, &uart1_res.htask_rx);
}

/// @brief 在中断中调用接收数据
/// @param data 
/// @param length 
BaseType_t uart1RecvData(uint8_t *data, uint32_t length) {
    if (data == NULL || length == 0) return pdFALSE;
    UartPacket packet = {
        .length = length,
        .payload = NULL
    };
    // 分配内存
    packet.payload = (uint8_t*)pvPortMalloc(length * sizeof(uint8_t));
    if (packet.payload != NULL) {
        // 拷贝数据
        memcpy(packet.payload, data, length);
        // 发送到接收队列
        BaseType_t high_task = pdFALSE;
        if (xQueueSendFromISR(uart1_res.queue_rx, &packet, &high_task) != pdTRUE) vPortFree(packet.payload);
        else {
            portYIELD_FROM_ISR(high_task);
            return pdTRUE;
        }
    }
    return pdFALSE;
}
