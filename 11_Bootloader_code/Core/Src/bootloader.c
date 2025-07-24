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




static uint32_t current_flash_write_addr = APP_START_ADDRESS;


uint8_t Bootloader_CheckFirmwareSize(uint32_t file_size_bytes)
{
    if (file_size_bytes > MAX_FIRMWARE_SIZE_BYTES)
    {
        // File is too large
        return 0; // invalid
    }
    return 1; // valid
}


void Bootloader_StartFirmwareUpdate(void)
{
    // Erase all sectors for the main app
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef eraseInit;
    uint32_t sectorError = 0;

    eraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInit.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    eraseInit.Sector = FLASH_SECTOR_1;
    eraseInit.NbSectors = FLASH_SECTOR_6 - FLASH_SECTOR_1 + 1;

    if (HAL_FLASHEx_Erase(&eraseInit, &sectorError) != HAL_OK)
    {
        // Handle erase error
    }

    current_flash_write_addr = APP_START_ADDRESS;
}

uint8_t Bootloader_WriteFirmwareChunk(uint32_t address, const uint8_t *data, uint32_t length)
{
    for (uint32_t i = 0; i < length; i += 4)
    {
        uint32_t word = *(uint32_t*)(data + i);
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address, word) != HAL_OK)
        {
            return 0; // Fail
        }
        address += 4;
    }
    return 1; // Success
}


void Bootloader_EndFirmwareUpdate(void)
{
    HAL_FLASH_Lock();
}



// Define the main application entry point
typedef void (*pFunction)(void);

// Function to jump to the main application
void  Bootloader_JumpToApplication(void)
{
    // 1. Check if the main application exists at the specified address
    uint32_t app_start_address = APP_START_ADDRESS;

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
