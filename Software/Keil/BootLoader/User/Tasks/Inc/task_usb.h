#ifndef _TASK_USB_H_
#define _TASK_USB_H_

#include "config.h"

typedef struct {
    TaskHandle_t htask_rx;
    QueueHandle_t queue_rx;
} UsbResource;

typedef struct {
    uint32_t length;
    uint8_t *payload;
} UsbPacket;

extern UsbResource usb_res;

void createUsbTask(void);
BaseType_t usbRecvData(uint8_t *data, uint32_t length);

#endif // !_TASK_USB_H_
