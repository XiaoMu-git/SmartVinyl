#include "uart.h"
#include "string.h"
#include "task_uart.h"

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_uart1_tx;
DMA_HandleTypeDef hdma_uart1_rx;
uint8_t buff_uart1_tx[UART_BUFF_SIZE];
uint8_t buff_uart1_rx[UART_BUFF_SIZE];
SemaphoreHandle_t semaphore_uart1_tx;
SemaphoreHandle_t mutex_uart1_tx;

/// @brief UART 初始化函数
void uartInit(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_USART1_CLK_ENABLE();

    // UART1 TX PIN
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // UART1 RX PIN
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // UART1 TX DMA
    hdma_uart1_tx.Instance = DMA1_Channel4;
    hdma_uart1_tx.Init.Direction = DMA_MEMORY_TO_PERIPH;
    hdma_uart1_tx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart1_tx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart1_tx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart1_tx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart1_tx.Init.Mode = DMA_NORMAL;
    hdma_uart1_tx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_uart1_tx) != HAL_OK) {
        __disable_irq();
        while (1);
    }
    __HAL_LINKDMA(&huart1, hdmatx, hdma_uart1_tx);

    // UART1 RX DMA
    hdma_uart1_rx.Instance = DMA1_Channel5;
    hdma_uart1_rx.Init.Direction = DMA_PERIPH_TO_MEMORY;
    hdma_uart1_rx.Init.PeriphInc = DMA_PINC_DISABLE;
    hdma_uart1_rx.Init.MemInc = DMA_MINC_ENABLE;
    hdma_uart1_rx.Init.PeriphDataAlignment = DMA_PDATAALIGN_BYTE;
    hdma_uart1_rx.Init.MemDataAlignment = DMA_MDATAALIGN_BYTE;
    hdma_uart1_rx.Init.Mode = DMA_NORMAL;
    hdma_uart1_rx.Init.Priority = DMA_PRIORITY_LOW;
    if (HAL_DMA_Init(&hdma_uart1_rx) != HAL_OK) {
        __disable_irq();
        while (1);
    }
    __HAL_LINKDMA(&huart1, hdmarx, hdma_uart1_rx);

    // UART1 INIT
    huart1.Instance = USART1;
    huart1.Init.BaudRate = 115200;
    huart1.Init.WordLength = UART_WORDLENGTH_8B;
    huart1.Init.StopBits = UART_STOPBITS_1;
    huart1.Init.Parity = UART_PARITY_NONE;
    huart1.Init.Mode = UART_MODE_TX_RX;
    huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;
    if (HAL_UART_Init(&huart1) != HAL_OK) {
        __disable_irq();
        while (1);
    }

    // UART1 NVIC
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

    // 申请 IPC 资源
    semaphore_uart1_tx = xSemaphoreCreateBinary();
    if (semaphore_uart1_tx == NULL) {
        __disable_irq();
        while (1);
    }
    mutex_uart1_tx = xSemaphoreCreateMutex();
    if (mutex_uart1_tx == NULL) {
        __disable_irq();
        while (1);
    }

    // 启动 DMA 接收
    if (HAL_UART_Receive_DMA(&huart1, buff_uart1_rx, UART_BUFF_SIZE) != HAL_OK) {
        __disable_irq();
        while (1);
    }
}

/// @brief uart DMA　接收完成
void uartRxDmaCompare(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        // 接收数据
        uart1RecvData(buff_uart1_rx, UART_BUFF_SIZE);
        // 重启接收 DMA 
        HAL_UART_Receive_DMA(huart, buff_uart1_rx, UART_BUFF_SIZE);
    }
}

/// @brief uart DMA 发送完成
void uartTxDmaCompare(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        // 释放 DMA 发送完成的信号量
        xSemaphoreGive(semaphore_uart1_tx);
    }
}

/// @brief uart 接收空闲中断
void uartRxIdleIrq(UART_HandleTypeDef *huart) {
    if (huart->Instance == USART1) {
        // 关闭接收 DMA
        ATOMIC_CLEAR_BIT(huart->Instance->CR3, USART_CR3_DMAR);
        HAL_DMA_Abort(huart->hdmarx);
        huart->RxState = HAL_UART_STATE_READY;
        huart->ReceptionType = HAL_UART_RECEPTION_STANDARD;
        // 计算接收数据长度
        uint32_t len_rx = UART_BUFF_SIZE - __HAL_DMA_GET_COUNTER(huart->hdmarx);
        // 接收数据
        uart1RecvData(buff_uart1_rx, len_rx);
        // 重启接收 DMA 
        HAL_UART_Receive_DMA(huart, buff_uart1_rx, UART_BUFF_SIZE);
    }
}

/// @brief 使用 uart 发送数据
/// @param huart 
/// @param data 
/// @param length 
/// @return 
BaseType_t uartSendData(UART_HandleTypeDef *huart, uint8_t *data, uint32_t length) {
    if (huart == NULL || data == NULL || length == 0) return pdFALSE;
    BaseType_t result = pdFALSE;
    // 互斥锁上锁
    if (xSemaphoreTake(mutex_uart1_tx, TIME_WAIT_MEDIUM) == pdTRUE) {
        // 使用循环，直到当前包内所有数据发送完成
        for (uint32_t offset = 0; offset < length; offset += UART_BUFF_SIZE) {
            // 计算可拷贝长度
            uint32_t copy_len = length - offset > UART_BUFF_SIZE ? UART_BUFF_SIZE : length - offset;
            // 拷贝数据
            memcpy(buff_uart1_tx, data + offset, copy_len);
            // 清空 DMA 发送完成信号量
            xSemaphoreTake(semaphore_uart1_tx, 0);
            // 开启 DMA 发送
            result = pdFALSE;
            if (HAL_UART_Transmit_DMA(huart, buff_uart1_tx, copy_len) == HAL_OK) {
                // 等待 DMA 发送完成
                if (xSemaphoreTake(semaphore_uart1_tx, TIME_WAIT_SHORT) == pdTRUE) result = pdTRUE;
            }
            // 有一次发送失败就停止
            if (result != pdTRUE) break;
        }
        // 互斥锁解锁
        xSemaphoreGive(mutex_uart1_tx);
    }
    return result;
}
