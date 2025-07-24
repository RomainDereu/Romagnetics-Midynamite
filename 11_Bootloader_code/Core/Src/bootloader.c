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

    screen_driver_SetCursor_WriteString("Start Update", Font_6x8, White, 10,60);
    screen_driver_UpdateScreen();
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
        uint32_t word = 0xFFFFFFFF;  // Padding default
        memcpy(&word, data + i, MIN(4, length - i));

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



void Bootloader_FormatFlashFAT16(void)
{
    const uint16_t BYTES_PER_SECTOR     = 512;
    const uint8_t  SECTORS_PER_CLUSTER  = 1;
    const uint16_t RESERVED_SECTORS     = 1;
    const uint8_t  NUM_FATS             = 1;
    const uint16_t ROOT_ENTRIES         = 64;
    const uint16_t ROOT_DIR_SECTORS     = ((ROOT_ENTRIES * 32) + (BYTES_PER_SECTOR - 1)) / BYTES_PER_SECTOR;
    const uint16_t TOTAL_SECTORS        = (320 * 1024) / BYTES_PER_SECTOR;
    const uint16_t SECTORS_PER_FAT      = 4;
    const uint16_t FAT_START_SECTOR     = RESERVED_SECTORS;
    const uint16_t ROOT_DIR_START       = FAT_START_SECTOR + NUM_FATS * SECTORS_PER_FAT;
    const uint16_t DATA_START_SECTOR    = ROOT_DIR_START + ROOT_DIR_SECTORS;

    const uint16_t FILE_SIZE            = 64 * 1024; // 64KB dummy file
    const uint16_t FILE_CLUSTER_COUNT   = FILE_SIZE / BYTES_PER_SECTOR; // 128 clusters

    uint8_t sector[512];
    memset(sector, 0x00, sizeof(sector));

    // -------------------
    // BOOT SECTOR
    // -------------------
    sector[0x0B] = BYTES_PER_SECTOR & 0xFF;
    sector[0x0C] = BYTES_PER_SECTOR >> 8;
    sector[0x0D] = SECTORS_PER_CLUSTER;
    sector[0x0E] = RESERVED_SECTORS & 0xFF;
    sector[0x0F] = RESERVED_SECTORS >> 8;
    sector[0x10] = NUM_FATS;
    sector[0x11] = ROOT_ENTRIES & 0xFF;
    sector[0x12] = ROOT_ENTRIES >> 8;
    sector[0x13] = TOTAL_SECTORS & 0xFF;
    sector[0x14] = TOTAL_SECTORS >> 8;
    sector[0x15] = 0xF8;  // media descriptor
    sector[0x16] = SECTORS_PER_FAT & 0xFF;
    sector[0x17] = SECTORS_PER_FAT >> 8;
    sector[0x18] = 0x3F;  // sectors per track
    sector[0x1A] = 0xFF;  // number of heads
    sector[0x1C] = 0x00;  // hidden sectors
    sector[0x1FE] = 0x55;
    sector[0x1FF] = 0xAA;

    HAL_FLASH_Unlock();
    for (uint16_t i = 0; i < TOTAL_SECTORS; i++) {
        if (i == 0) {
            Bootloader_WriteFirmwareChunk(APP_START_ADDRESS + i * BYTES_PER_SECTOR, sector, BYTES_PER_SECTOR);
        } else {
            memset(sector, 0x00, BYTES_PER_SECTOR);
            Bootloader_WriteFirmwareChunk(APP_START_ADDRESS + i * BYTES_PER_SECTOR, sector, BYTES_PER_SECTOR);
        }
    }

    // -------------------
    // FAT TABLE
    // -------------------
    uint16_t FAT[SECTORS_PER_FAT * BYTES_PER_SECTOR / 2];
    memset(FAT, 0x00, sizeof(FAT));
    FAT[0] = 0xFFF8; // media descriptor
    FAT[1] = 0xFFFF; // reserved

    for (uint16_t i = 2; i < 2 + FILE_CLUSTER_COUNT - 1; i++) {
        FAT[i] = i + 1;
    }
    FAT[2 + FILE_CLUSTER_COUNT - 1] = 0xFFFF; // end of file

    // Write FAT sectors
    for (uint16_t i = 0; i < SECTORS_PER_FAT; i++) {
        memcpy(sector, &FAT[i * (BYTES_PER_SECTOR / 2)], BYTES_PER_SECTOR);
        Bootloader_WriteFirmwareChunk(APP_START_ADDRESS + (FAT_START_SECTOR + i) * BYTES_PER_SECTOR, sector, BYTES_PER_SECTOR);
    }

    // -------------------
    // ROOT DIRECTORY
    // -------------------
    memset(sector, 0x00, BYTES_PER_SECTOR);
    uint8_t *entry = sector;

    // 8.3 Filename: "FIRMWAREBIN"
    memcpy(&entry[0], "FIRMWARE", 8);    // filename
    memcpy(&entry[8], "BIN", 3);         // extension
    entry[11] = 0x20;                    // attribute: archive
    entry[26] = 2 & 0xFF;                // first cluster low
    entry[27] = 2 >> 8;
    entry[28] = FILE_SIZE & 0xFF;        // file size
    entry[29] = (FILE_SIZE >> 8) & 0xFF;
    entry[30] = (FILE_SIZE >> 16) & 0xFF;
    entry[31] = (FILE_SIZE >> 24) & 0xFF;

    // Write root dir sector(s)
    for (uint16_t i = 0; i < ROOT_DIR_SECTORS; i++) {
        if (i == 0) {
            Bootloader_WriteFirmwareChunk(APP_START_ADDRESS + (ROOT_DIR_START + i) * BYTES_PER_SECTOR, sector, BYTES_PER_SECTOR);
        } else {
            memset(sector, 0x00, BYTES_PER_SECTOR);
            Bootloader_WriteFirmwareChunk(APP_START_ADDRESS + (ROOT_DIR_START + i) * BYTES_PER_SECTOR, sector, BYTES_PER_SECTOR);
        }
    }

    HAL_FLASH_Lock();
}






void screen_driver_SetCursor_WriteString(const char* str, screen_driver_Font_t font,
										 screen_driver_COLOR color,
										 uint8_t x_align,
										 uint8_t y_align){
	screen_driver_SetCursor(x_align, y_align);
	screen_driver_WriteString(str, font , color);
}
