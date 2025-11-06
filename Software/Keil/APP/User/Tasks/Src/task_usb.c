#include "task_usb.h"
#include "string.h"
#include "usb.h"

UsbResource usb_res;

/// @brief usb 接收数据任务函数
/// @param param 
void usbRxTask(void *param) {
    UsbResource *usb_res = (UsbResource*)param;
    UsbPacket packet = { 0 };

    while (1) {
        xQueueReceive(usb_res->queue_rx, &packet, portMAX_DELAY);
        if (packet.payload == NULL) continue;
        usbSendData(packet.payload, packet.length);
        vPortFree(packet.payload);
        packet.payload = NULL;
    }
}

/// @brief 创建 usb 任务
void createUsbTask(void) {
    // 分配资源
    usb_res.queue_rx = xQueueCreate(QUEUE_SIZE_LARGE, sizeof(UsbPacket));
    // 创建任务
    xTaskCreate(usbRxTask, "usb_rx", TASK_STACK_TINY, &usb_res, TASK_PRIORITY_REALTIME, &usb_res.htask_rx);
}

/// @brief 在中断中调用接收数据
/// @param data 
/// @param length 
BaseType_t usbRecvData(uint8_t *data, uint32_t length) {
    if (data == NULL || length == 0) return pdFALSE;
    UsbPacket packet = {
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
        if (xQueueSendFromISR(usb_res.queue_rx, &packet, &high_task) != pdTRUE) vPortFree(packet.payload);
        else {
            portYIELD_FROM_ISR(high_task);
            return pdTRUE;
        }
    }
    return pdFALSE;
}
