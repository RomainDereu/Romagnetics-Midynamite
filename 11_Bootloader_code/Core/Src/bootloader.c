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
#include "usb_device.h"


extern USBD_HandleTypeDef hUsbDeviceFS;


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
    // 1) Unlock flash & clear any existing error flags
    HAL_FLASH_Unlock();
    __HAL_FLASH_CLEAR_FLAG(
        FLASH_FLAG_EOP    |
        FLASH_FLAG_OPERR  |
        FLASH_FLAG_WRPERR |
        FLASH_FLAG_PGAERR |
        FLASH_FLAG_PGPERR |
        FLASH_FLAG_PGSERR
    );

    // 2) Erase sectors 4..7 (first app sector is 4)
    FLASH_EraseInitTypeDef eraseInit = {0};
    uint32_t sectorError = 0;

    eraseInit.TypeErase   = FLASH_TYPEERASE_SECTORS;
    eraseInit.VoltageRange= FLASH_VOLTAGE_RANGE_3;
    eraseInit.Sector      = FLASH_SECTOR_4;   // starts at APP_START_ADDRESS
    eraseInit.NbSectors   = 3;               // only sectors 4,5,6

    if (HAL_FLASHEx_Erase(&eraseInit, &sectorError) != HAL_OK)
    {
        char err[32];
        snprintf(err, sizeof(err), "Erase err %lu", sectorError);
        screen_driver_SetCursor_WriteString(err, Font_6x8, White, 0,40);
        screen_driver_UpdateScreen();
        HAL_Delay(600);
        // hang or return
    }
    current_flash_write_addr = APP_START_ADDRESS;
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



uint8_t Bootloader_WriteFirmwareChunk(uint32_t address, const uint8_t *data, uint32_t length)
{
    uint32_t error_code;
    // Round up to a multiple of 4 bytes
    uint32_t padded_len = (length + 3) & ~3u;

    for (uint32_t offset = 0; offset < padded_len; offset += 4)
    {
        uint32_t word = 0xFFFFFFFF;
        uint32_t copy = (offset + 4 <= length) ? 4 : (length - offset);
        memcpy(&word, data + offset, copy);

        // Clear any previous errors
        __HAL_FLASH_CLEAR_FLAG(
            FLASH_FLAG_EOP    |
            FLASH_FLAG_OPERR  |
            FLASH_FLAG_WRPERR |
            FLASH_FLAG_PGAERR |
            FLASH_FLAG_PGPERR |
            FLASH_FLAG_PGSERR
        );

        // Program one word
        __disable_irq();
        HAL_StatusTypeDef st = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, address + offset, word);
        __enable_irq();

        if (st != HAL_OK)
        {
            error_code = HAL_FLASH_GetError();
            char err[32];
            // Show the error code on your display
            snprintf(err, sizeof(err), "Prog err 0x%08lX", error_code);
            screen_driver_SetCursor_WriteString(err, Font_6x8, White, 0, 50);
            screen_driver_UpdateScreen();
            return 0;  // fail
        }
    }

    return 1;  // success
}


void Bootloader_EndFirmwareUpdate(void)
{
    HAL_FLASH_Lock();
}



// Define the main application entry point
typedef void (*pFunction)(void);

// Function to jump to the main application
void Bootloader_JumpToApplication(void) {
    uint32_t *vectors    = (uint32_t*)APP_START_ADDRESS;
    uint32_t  appStack   = vectors[0];
    uint32_t  appEntry   = vectors[1];

    // Sanity check: make sure stack & entry aren't 0xFFFFFFFF
    if (appStack == 0xFFFFFFFF || appEntry == 0xFFFFFFFF) {
        return;
    }

    // 1) Disable all interrupts
    __disable_irq();

    // 2) Relocate vector table
    SCB->VTOR = APP_START_ADDRESS;
    __DSB();  // Data Synchronization Barrier
    __ISB();  // Instruction Synchronization Barrier

    // 3) Set the MSP to the application's stack pointer
    __set_MSP(appStack);

    // 4) (Optionally) re‑enable interrupts here if your app needs them right away
    __enable_irq();

    // 5) Jump to the Reset handler
    ((pFunction)appEntry)();

    // Should never get here
    for (;;);
}





void screen_driver_SetCursor_WriteString(const char* str, screen_driver_Font_t font,
										 screen_driver_COLOR color,
										 uint8_t x_align,
										 uint8_t y_align){
	screen_driver_SetCursor(x_align, y_align);
	screen_driver_WriteString(str, font , color);
}

