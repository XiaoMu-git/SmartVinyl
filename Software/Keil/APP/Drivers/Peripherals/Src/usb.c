#include "usb.h"

void usbInit(void) {
    
}



// #include "usbd_core.h"
// #include "usbd_cdc.h"
// #include "usbd_desc.h"
// #include "task_usb.h"
// #include "task_log.h"

// USBD_HandleTypeDef husbd;
// uint8_t buff_usb_tx[USB_BUFF_SIZE];
// uint8_t buff_usb_rx[USB_BUFF_SIZE];
// SemaphoreHandle_t semaphore_usb_tx;
// SemaphoreHandle_t mutex_usb_tx;

// extern uint8_t cdcInit(void);
// extern uint8_t cdcDeInit(void);
// extern uint8_t cdcControl(uint8_t cmd, uint8_t *pbuf, uint16_t length);
// extern uint8_t cdcReceive(uint8_t *data, uint32_t *length);
// extern void cdcTxCompare(void);

// USBD_CDC_ItfTypeDef usbd_cdc_fops = {
//     cdcInit,
//     cdcDeInit,
//     cdcControl,
//     cdcReceive,
//     cdcTxCompare
// };

// /// @brief USB 初始化函数
// void usbInit(void) {
//     // 初始化 USB 设备核心
//     if (USBD_Init(&husbd, &FS_Desc, DEVICE_FS) != USBD_OK) {
//         __disable_irq();
//         while (1);
//     }

//     // 注册 USB 类
//     if (USBD_RegisterClass(&husbd, &USBD_CDC) != USBD_OK) {
//         __disable_irq();
//         while (1);
//     }

//     // 注册 CDC 接口回调函数
//     if (USBD_CDC_RegisterInterface(&husbd, &usbd_cdc_fops) != USBD_OK) {
//         __disable_irq();
//         while (1);
//     }

//     // 启动 USB 设备
//     if (USBD_Start(&husbd) != USBD_OK) {
//         __disable_irq();
//         while (1);
//     }

//     // 申请 IPC 资源
//     semaphore_usb_tx = xSemaphoreCreateBinary();
//     if (semaphore_usb_tx == NULL) {
//         __disable_irq();
//         while (1);
//     }
//     mutex_usb_tx = xSemaphoreCreateMutex();
//     if (mutex_usb_tx == NULL) {
//         __disable_irq();
//         while (1);
//     }
// }

// /// @brief 发送 usb 数据
// /// @param data 
// /// @param length 
// /// @return 
// BaseType_t usbSendData(uint8_t *data, uint32_t length) {
//     USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)husbd.pClassData;
//     if (data == NULL || length == 0 || hcdc->TxState != 0) return pdFALSE;
//     BaseType_t result = pdFALSE;
//     // 互斥锁上锁
//     if (xSemaphoreTake(mutex_usb_tx, TIME_WAIT_MEDIUM) == pdTRUE) {
//         // 使用循环，直到当前包内所有数据发送完成
//         for (uint32_t offset = 0; offset < length; offset += USB_BUFF_SIZE) {
//             // 计算可拷贝长度
//             uint32_t copy_len = length - offset > USB_BUFF_SIZE ? USB_BUFF_SIZE : length - offset;
//             // 拷贝数据
//             memcpy(buff_usb_tx, data + offset, copy_len);
//             // 清空发送完成信号量
//             xSemaphoreTake(semaphore_usb_tx, 0);
//             // 使用 cdc 发送数据
//             USBD_CDC_SetTxBuffer(&husbd, buff_usb_tx, copy_len);
//             if (USBD_CDC_TransmitPacket(&husbd) == USBD_OK) {
//                 // 等待发送完成
//                 if (xSemaphoreTake(semaphore_usb_tx, TIME_WAIT_SHORT) == pdTRUE) result = pdTRUE;
//             }
//             // 有一次发送失败就停止
//             if (result != pdTRUE) break;
//         }
//         // 互斥锁解锁
//         xSemaphoreGive(mutex_usb_tx);
//     }
//     return result;
// }

// /// @brief cdc 初始化
// /// @return 
// uint8_t cdcInit(void) {
//     USBD_CDC_SetRxBuffer(&husbd, buff_usb_rx);
//     return USBD_OK;
// }

// /// @brief cdc 反初始化
// /// @return 
// uint8_t cdcDeInit(void) {
//     return USBD_OK;
// }

// /// @brief cdc 控制命令
// /// @param cmd 
// /// @param pbuf 
// /// @param length 
// /// @return 
// uint8_t cdcControl(uint8_t cmd, uint8_t *pbuf, uint16_t length) {
//     switch (cmd) {
//         case CDC_SEND_ENCAPSULATED_COMMAND:
//             break;
//         case CDC_GET_ENCAPSULATED_RESPONSE:
//             break;
//         case CDC_SET_COMM_FEATURE:
//             break;
//         case CDC_GET_COMM_FEATURE:
//             break;
//         case CDC_CLEAR_COMM_FEATURE:
//             break;
//         case CDC_SET_LINE_CODING:
//             break;
//         case CDC_GET_LINE_CODING:
//             break;
//         case CDC_SET_CONTROL_LINE_STATE:
//             break;
//         case CDC_SEND_BREAK:
//             break;
//         default:
//             break;
//     }
//     return USBD_OK;
// }

// /// @brief 数据接收回调
// /// @param data 
// /// @param length 
// /// @return 
// uint8_t cdcReceive(uint8_t *data, uint32_t *length) {
//     usbRecvData(data, *length);
//     USBD_CDC_SetRxBuffer(&husbd, data);
//     USBD_CDC_ReceivePacket(&husbd);
//     return USBD_OK;
// }

// /// @brief cdc 发送完成
// void cdcTxCompare(void) {
//     // 释放 cdc 发送完成的信号量
//     xSemaphoreGive(semaphore_usb_tx);
// }
