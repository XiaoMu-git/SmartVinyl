#ifndef _USB_H_
#define _USB_H_

#include "config.h"
#include "usbd_core.h"
#include "usbd_cdc.h"
#include "usbd_desc.h"

extern USBD_HandleTypeDef husbd;

void usbInit(void);
uint8_t usbSendData(uint8_t *data, uint32_t length);
uint8_t usbRecvData(uint8_t *data, uint32_t length);

#endif // !_USB_H_
