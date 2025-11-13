#include "usb.h"

USBD_HandleTypeDef husbd;
uint8_t usb_tx_buff[USB_BUFF_SIZE];
uint8_t usb_tx_buff[USB_BUFF_SIZE];
SemaphoreHandle_t usb_tx_sem;
SemaphoreHandle_t usb_tx_mutex;

extern uint8_t cdcInit(void);
extern uint8_t cdcDeInit(void);
extern uint8_t cdcControl(uint8_t cmd, uint8_t *pbuf, uint16_t length);
extern uint8_t cdcReceive(uint8_t *data, uint32_t *length);
extern void cdcTxCompare(void);

USBD_CDC_ItfTypeDef usbd_cdc_fops = {
    cdcInit,
    cdcDeInit,
    cdcControl,
    cdcReceive,
    cdcTxCompare
};

/// @brief USB 初始化函数
void usbInit(void) {
    GPIO_InitTypeDef GPIO_InitStruct = { 0 };
    __HAL_RCC_USB_CLK_ENABLE();

    // 拉低引脚，让主机识别
    GPIO_InitStruct.Pin = GPIO_PIN_12;
    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
    HAL_GPIO_WritePin(GPIOA, GPIO_PIN_12, GPIO_PIN_RESET);
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    HAL_Delay(10);

    // 初始化 USB 设备核心
    if (USBD_Init(&husbd, &FS_Desc, DEVICE_FS) != USBD_OK) {
        __disable_irq();
        while (1);
    }

    // 注册 USB 类
    if (USBD_RegisterClass(&husbd, &USBD_CDC) != USBD_OK) {
        __disable_irq();
        while (1);
    }

    // 注册 CDC 接口回调函数
    if (USBD_CDC_RegisterInterface(&husbd, &usbd_cdc_fops) != USBD_OK) {
        __disable_irq();
        while (1);
    }

    // 启动 USB 设备
    if (USBD_Start(&husbd) != USBD_OK) {
        __disable_irq();
        while (1);
    }

    // 申请 IPC 资源
    usb_tx_sem = xSemaphoreCreateBinary();
    usb_tx_mutex = xSemaphoreCreateMutex();
    if (usb_tx_sem == NULL || usb_tx_mutex == NULL) {
        __disable_irq();
        while (1);
    }
}

/// @brief 发送 usb 数据
/// @param data 
/// @param length 
/// @return 
uint8_t usbSendData(uint8_t *data, uint32_t length) {
    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef*)husbd.pClassData;
    if (data == NULL || length == 0 || hcdc == NULL || hcdc->TxState != 0) return RET_FAIL;
    uint8_t result = RET_FAIL;
    // 互斥锁上锁
    if (xSemaphoreTake(usb_tx_mutex, TIME_WAIT_MEDIUM) == pdTRUE) {
        // 使用循环，直到当前包内所有数据发送完成
        for (uint32_t offset = 0; offset < length; offset += USB_BUFF_SIZE) {
            // 计算可拷贝长度
            uint32_t copy_len = length - offset > USB_BUFF_SIZE ? USB_BUFF_SIZE : length - offset;
            // 拷贝数据
            memcpy(usb_tx_buff, data + offset, copy_len);
            // 清空发送完成信号量
            xSemaphoreTake(usb_tx_sem, 0);
            // 使用 cdc 发送数据
            USBD_CDC_SetTxBuffer(&husbd, usb_tx_buff, copy_len);
            if (USBD_CDC_TransmitPacket(&husbd) == USBD_OK) {
                // 等待发送完成
                if (xSemaphoreTake(usb_tx_sem, TIME_WAIT_SHORT) == pdTRUE) result = RET_DONE;
            }
            // 有一次发送失败就停止
            if (result != RET_DONE) break;
        }
        // 互斥锁解锁
        xSemaphoreGive(usb_tx_mutex);
    }
    return result;
}

/// @brief 接收 usb 数据
/// @param data 
/// @param length 
/// @return 
uint8_t usbRecvData(uint8_t *data, uint32_t length) {
    if (data == NULL || length == 0) return RET_FAIL;
    uint8_t result = RET_FAIL;
    return result;
}

/// @brief cdc 初始化
/// @return 
uint8_t cdcInit(void) {
    USBD_CDC_SetRxBuffer(&husbd, usb_tx_buff);
    return USBD_OK;
}

/// @brief cdc 反初始化
/// @return 
uint8_t cdcDeInit(void) {
    return USBD_OK;
}

/// @brief cdc 控制命令
/// @param cmd 
/// @param pbuf 
/// @param length 
/// @return 
uint8_t cdcControl(uint8_t cmd, uint8_t *pbuf, uint16_t length) {
    switch (cmd) {
        case CDC_SEND_ENCAPSULATED_COMMAND:
            break;
        case CDC_GET_ENCAPSULATED_RESPONSE:
            break;
        case CDC_SET_COMM_FEATURE:
            break;
        case CDC_GET_COMM_FEATURE:
            break;
        case CDC_CLEAR_COMM_FEATURE:
            break;
        case CDC_SET_LINE_CODING:
            break;
        case CDC_GET_LINE_CODING:
            break;
        case CDC_SET_CONTROL_LINE_STATE:
            break;
        case CDC_SEND_BREAK:
            break;
        default:
            break;
    }
    return USBD_OK;
}

/// @brief 数据接收回调
/// @param data 
/// @param length 
/// @return 
uint8_t cdcReceive(uint8_t *data, uint32_t *length) {
    usbRecvData(data, *length);
    USBD_CDC_SetRxBuffer(&husbd, data);
    USBD_CDC_ReceivePacket(&husbd);
    return USBD_OK;
}

/// @brief cdc 发送完成
void cdcTxCompare(void) {
    BaseType_t urgent_task = pdFALSE;
    xSemaphoreGiveFromISR(usb_tx_sem, &urgent_task);
    portYIELD_FROM_ISR(urgent_task);
}
