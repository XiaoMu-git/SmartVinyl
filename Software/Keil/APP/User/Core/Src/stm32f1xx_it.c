#include "main.h"

extern PCD_HandleTypeDef hpcd_USB_FS;

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

void USB_LP_CAN1_RX0_IRQHandler(void) {
    HAL_PCD_IRQHandler(&hpcd_USB_FS);
}
