/*
 * bootloader.c
 *
 *  Created on: Jul 23, 2025
 *      Author: Romain Dereu
 */


#include <stdio.h>
#include <string.h>

#include "main.h"
#include "bootloader.h"
#include "stm32f4xx_hal.h"

#include "screen_driver.h"
#include "screen_driver_fonts.h"

#define MAIN_APP_ADDRESS 0x08010000



// Define the main application entry point
typedef void (*pFunction)(void);

// Function to jump to the main application
void Bootloader_JumpToApplication(void)
{
    // 1. Check if the main application exists at the specified address
    uint32_t app_start_address = MAIN_APP_ADDRESS;

    // Get the stack pointer value from the main application's start address
    uint32_t stack_pointer = *(volatile uint32_t*) app_start_address;


    // Check if the stack pointer is valid (not 0xFFFFFFFF, which indicates invalid memory)
    if (stack_pointer == 0xFFFFFFFF)
    {
        // Invalid application, stay in the bootloader
        while (1);  // Enter infinite loop for error handling
    }

    stack_pointer &= ~0x03;


    // 2. Set the Main Stack Pointer (MSP) to the application's stack pointer
    SCB->VTOR = 0x08010000;
    __set_MSP(stack_pointer);





    // 3. Get the Reset Handler address (entry point of the main application)
    uint32_t reset_handler_address = *(volatile uint32_t*) (app_start_address + 4);

    // Check if the reset handler is valid (not 0xFFFFFFFF)
    if (reset_handler_address == 0xFFFFFFFF)
    {
        // Invalid reset handler, stay in the bootloader
        while (1);  // Enter infinite loop for error handling

    }

    // 4. Define a pointer to the Reset Handler function
    pFunction reset_handler = (pFunction) reset_handler_address;
    // 5. Jump to the main application



    reset_handler();  // Jump to the main application

}



void screen_driver_SetCursor_WriteString(const char* str, screen_driver_Font_t font,
										 screen_driver_COLOR color,
										 uint8_t x_align,
										 uint8_t y_align){
	screen_driver_SetCursor(x_align, y_align);
	screen_driver_WriteString(str, font , color);
}
