#ifndef _USB_H_
#define _USB_H_

#include "config.h"
#include "usbd_def.h"

#define USB_BUFF_SIZE 128

extern USBD_HandleTypeDef husbd;
extern uint8_t buff_usb_tx[USB_BUFF_SIZE];
extern uint8_t buff_usb_rx[USB_BUFF_SIZE];

void usbInit(void);
BaseType_t usbSendData(uint8_t *data, uint32_t length);

#endif // !_USB_H_
