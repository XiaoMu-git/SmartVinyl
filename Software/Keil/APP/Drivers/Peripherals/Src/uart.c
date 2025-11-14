#include "uart.h"
#include "string.h"

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_uart1_tx;
DMA_HandleTypeDef hdma_uart1_rx;
uint8_t uart1_tx_buff[UART1_BUFF_SIZE];
uint8_t uart1_rx_buff[UART1_BUFF_SIZE];
SemaphoreHandle_t uart1_mutex;
SemaphoreHandle_t uart1_tx_semaphore;

/// @brief 串口初始化函数
void uartInit(void) {
    GPIO_InitTypeDef GPIO_InitStruct = {0};
    __HAL_RCC_USART1_CLK_ENABLE();

    // UART1 引脚
    GPIO_InitStruct.Pin = GPIO_PIN_9;
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

    // 配置参数
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

    // 配置 DMA
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

    // 配置并开启空闲中断
    HAL_NVIC_SetPriority(USART1_IRQn, 5, 0);
    HAL_NVIC_EnableIRQ(USART1_IRQn);
    __HAL_UART_ENABLE_IT(&huart1, UART_IT_IDLE);

    // 申请 IPC 资源
    uart1_mutex = xSemaphoreCreateMutex();
    uart1_tx_semaphore = xSemaphoreCreateBinary();
    if (uart1_mutex == NULL || uart1_tx_semaphore == NULL) {
        __disable_irq();
        while (1);
    }
    xSemaphoreGive(uart1_tx_semaphore);

    // 启动 DMA 接收
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart1_rx_buff, UART1_BUFF_SIZE);
    __HAL_DMA_DISABLE_IT(huart1.hdmarx, DMA_IT_HT);
}

/// @brief 使用 UART 发送数据
/// @param huart 
/// @param data 
/// @param length 
/// @return 实际发送的数据长度
uint8_t uartSendData(UART_HandleTypeDef *huart, uint8_t *data, uint32_t length) {
    if (huart == NULL || data == NULL) return RET_FAIL;
    uint8_t ret = RET_DONE;
    // 判断是哪个 UART
    if (huart->Instance == USART1) {
        // 抢占 UART1
        if (xSemaphoreTake(uart1_mutex, TIME_WAIT_MEDIUM) == pdTRUE) {
            while (length > 0) {
                uint32_t copy_length = length <= UART1_BUFF_SIZE ? length : UART1_BUFF_SIZE;
                // 等待发送空闲
                if (xSemaphoreTake(uart1_tx_semaphore, TIME_WAIT_SHORT) != pdTRUE) {
                    ret = RET_FAIL;
                    break;
                }
                // 拷贝待发送数据
                memcpy(uart1_tx_buff, data, copy_length);
                // 启动 DMA 发送
                if (HAL_UART_Transmit_DMA(&huart1, uart1_tx_buff, copy_length) != HAL_OK) {
                    xSemaphoreGive(uart1_tx_semaphore);
                    ret = RET_FAIL;
                    break;
                }
                // 发送成功处理
                data += copy_length;
                length -= copy_length;
            }
            xSemaphoreGive(uart1_mutex);
        }
        else ret = RET_FAIL;
    }
    return ret;
}
