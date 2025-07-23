/*
 * bootloader.c
 *
 *  Created on: Jul 23, 2025
 *      Author: Romain Dereu
 */



#include <string.h>
#include "main.h"
#include "bootloader.h"
#include "stm32f4xx_hal.h"


#define FIRMWARE_FILENAME   "FIRMWAREBIN"
#define FIRMWARE_EXT        ""

#define FAT16_ROOT_DIR_OFFSET 0x2000
#define FAT16_DIR_ENTRY_SIZE  32
#define FAT16_ROOT_DIR_SIZE   (512 * 16)
#define MAX_FIRMWARE_SIZE   (448 * 1024)

extern uint8_t MSC_RamDisk[];



uint8_t check_bootloader_button(void) {
    return (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_12) == GPIO_PIN_RESET) &&
           (HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_13) == GPIO_PIN_RESET);
}

void jump_to_application(void) {
    // The main application start address (e.g., 0x08010000 if you reserved 64K for bootloader)
    uint32_t appAddress = 0x08010000;
    uint32_t appStack = *(volatile uint32_t*)appAddress;
    uint32_t appEntry = *(volatile uint32_t*)(appAddress + 4);

    if (appStack == 0xFFFFFFFF || appEntry == 0xFFFFFFFF) return; // No app

    __disable_irq();
    SCB->VTOR = appAddress;
    __set_MSP(appStack);
    ((void (*)(void))appEntry)();
}

void poll_mass_storage_for_firmware_update(void)
{
    static uint8_t already_flashed = 0;

    if (already_flashed) return;

    // 1. Search MSC_RamDisk for "FIRMWARE.BIN"
    uint32_t fw_offset, fw_size;
    if (find_firmware_file_in_ramdisk(&fw_offset, &fw_size)) {
        // 2. (Optionally) verify file checksum or signature

        // 3. Flash erase & write
        if (fw_size < MAX_FIRMWARE_SIZE) {
            flash_erase_application_area();
            flash_write_application_area(&MSC_RamDisk[fw_offset], fw_size);
            already_flashed = 1;
            // (Optionally: blink LED or set some flag for success)
            HAL_Delay(200);
            NVIC_SystemReset(); // or jump_to_application()
        }
    }
}


uint8_t find_firmware_file_in_ramdisk(uint32_t *fw_offset, uint32_t *fw_size)
{
    uint32_t dir_start = FAT16_ROOT_DIR_OFFSET;
    uint32_t dir_end   = FAT16_ROOT_DIR_OFFSET + FAT16_ROOT_DIR_SIZE;

    for (uint32_t i = dir_start; i < dir_end; i += FAT16_DIR_ENTRY_SIZE) {
        uint8_t *entry = &MSC_RamDisk[i];
        if (entry[0] == 0x00 || entry[0] == 0xE5) continue; // Unused/deleted
        if (memcmp(entry, FIRMWARE_FILENAME, (size_t)11) == 0) {    // FAT16: 8 chars + 3 ext, upper-case, no dot
            uint16_t first_cluster = *(uint16_t *)(entry + 26);
            uint32_t file_size = *(uint32_t *)(entry + 28);
            // FAT16 data region offset: (adjust as needed for your FS)
            *fw_offset = 0x4000 + (first_cluster - 2) * 512;
            *fw_size = file_size;
            return 1;
        }
    }
    return 0;
}


void flash_write_application_area(uint8_t *data, uint32_t size)
{
    HAL_FLASH_Unlock();

    uint32_t dest = 0x08010000;
    for (uint32_t i = 0; i < size; i += 4) {
        uint32_t word = 0xFFFFFFFF;
        memcpy(&word, data + i, (size - i >= 4) ? 4 : (size - i));
        HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, dest + i, word);
    }

    HAL_FLASH_Lock();
}

void flash_erase_application_area(void)
{
    HAL_FLASH_Unlock();

    FLASH_EraseInitTypeDef eraseInitStruct = {0};
    uint32_t sectorError = 0;
    eraseInitStruct.TypeErase = FLASH_TYPEERASE_SECTORS;
    eraseInitStruct.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    eraseInitStruct.Sector = FLASH_SECTOR_4;    // start at sector 4 (64K in)
    eraseInitStruct.NbSectors = 4;              // 4 sectors: 4,5,6,7 (up to 512K)

    if (HAL_FLASHEx_Erase(&eraseInitStruct, &sectorError) != HAL_OK) {
        // Handle error (optional: blink LED, reset, etc.)
    }

    HAL_FLASH_Lock();
}
