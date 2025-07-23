/*
 * bootloader.c
 *
 *  Created on: Jul 23, 2025
 *      Author: Romain Dereu
 */

#include "main.h"


// Button check: adjust pins to your hardware!
int check_upgrade_buttons(void) {
    return (HAL_GPIO_ReadPin(GPIOB, Btn3_Pin) == GPIO_PIN_RESET) &&
           (HAL_GPIO_ReadPin(GPIOB, Btn4_Pin) == GPIO_PIN_RESET);
}

void jump_to_application(void) {
    uint32_t appStack = *(volatile uint32_t*)APP_ADDRESS;
    uint32_t appEntry = *(volatile uint32_t*)(APP_ADDRESS + 4);

    if (appStack == 0xFFFFFFFF || appEntry == 0xFFFFFFFF) return; // No app flashed

    __disable_irq();
    SCB->VTOR = APP_ADDRESS;
    __set_MSP(appStack);
    ((void (*)(void))appEntry)();
}
