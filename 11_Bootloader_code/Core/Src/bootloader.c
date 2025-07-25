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
static uint32_t crc32_table[256];



void Bootloader_StartFirmwareUpdate(void)
{

	g_crc_failed = 0;
	g_update_complete = 0;
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
        screen_driver_SetCursor_WriteString("ERASE ERROR", Font_6x8, White, 0,30);
        screen_driver_UpdateScreen();
        HAL_Delay(600);
        // hang or return
    }
    current_flash_write_addr = APP_START_ADDRESS;
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
            screen_driver_SetCursor_WriteString("FLASH WRITE ERROR", Font_6x8, White, 0, 40);
            screen_driver_UpdateScreen();
            return 0;  // fail
        }
    }

    return 1;  // success
}


uint8_t Bootloader_EndFirmwareUpdate(void)
{
    uint32_t fw_size    = g_expected_length;
    uint32_t crc_stored = *(uint32_t*)(APP_START_ADDRESS + fw_size - 4);
    uint32_t crc_calc   = Bootloader_ComputeCRC32(APP_START_ADDRESS, fw_size - 4);

    if (crc_calc != crc_stored) {
        g_crc_failed = 1;
        return 0;
    }

    HAL_FLASH_Lock();
    return 1;
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


void Bootloader_InitCRC32(void) {
    const uint32_t poly = 0xEDB88320;
    for (uint32_t i = 0; i < 256; i++) {
        uint32_t crc = i;
        for (uint32_t j = 0; j < 8; j++) {
            if (crc & 1)
                crc = (crc >> 1) ^ poly;
            else
                crc >>= 1;
        }
        crc32_table[i] = crc;
    }
}


uint32_t Bootloader_ComputeCRC32(uint32_t addr, uint32_t size) {
    uint32_t crc = 0xFFFFFFFF;
    for (uint32_t i = 0; i < size; i++) {
        uint8_t byte = *(uint8_t*)(addr + i);
        crc = (crc >> 8) ^ crc32_table[(crc ^ byte) & 0xFF];
    }
    return crc ^ 0xFFFFFFFF;
}



void Bootloader_HandleFatalError(const char* message)
{
    screen_driver_SetCursor_WriteString(message, Font_6x8, White, 10, 30);
    screen_driver_UpdateScreen();
    USBD_Stop(&hUsbDeviceFS);  // Cleanly cut USB
    while (1);  // Lock up safely
}
