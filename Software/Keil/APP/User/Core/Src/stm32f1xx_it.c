#include "main.h"

void NMI_Handler(void) {
    while (1);
}

void HardFault_Handler(void) {
    while (1);
}

void MemManage_Handler(void) {
    while (1);
}

void BusFault_Handler(void) {
    while (1);
}

void UsageFault_Handler(void) {
    while (1);
}

void DebugMon_Handler(void) {
    
}

void TIM7_IRQHandler(void) {
    HAL_TIM_IRQHandler(&htim7);
}

void USB_LP_CAN1_RX0_IRQHandler(void) {
    HAL_PCD_IRQHandler(&hpcd_USB_FS);
}
