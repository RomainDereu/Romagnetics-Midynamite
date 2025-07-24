/*
 * bootloader.h
 *
 *  Created on: Jul 23, 2025
 *      Author: Astaa
 */

#ifndef INC_BOOTLOADER_C_
#define INC_BOOTLOADER_C_

#define APP_START_ADDRESS      0x08010000
#define FLASH_SECTOR_FIRST     FLASH_SECTOR_1
#define FLASH_SECTOR_LAST      FLASH_SECTOR_6
#define FLASH_PAGE_SIZE        0x20000  // 128 KB per sector for sectors 4-6
#define FIRMWARE_MAX_SIZE      (320 * 1024)
#define UPDATE_FILENAME        "FIRMWIREBIN"  // 11-char 8.3 FAT name

#define MAX_FIRMWARE_SIZE_BYTES (320 * 1024)

#include "screen_driver.h"
#include "screen_driver_fonts.h"




uint8_t Bootloader_CheckFirmwareSize(uint32_t file_size_bytes);



uint8_t Bootloader_CheckFirmwareSize(uint32_t file_size_bytes);
void Bootloader_StartFirmwareUpdate(void);
uint8_t Bootloader_WriteFirmwareChunk(uint32_t address, const uint8_t *data, uint32_t length);
void Bootloader_EndFirmwareUpdate(void);



typedef void (*pFunction)(void);

// Function to jump to the main application
void Bootloader_JumpToApplication(void);

void screen_driver_SetCursor_WriteString(const char* str, screen_driver_Font_t font,
		 screen_driver_COLOR color,
		 uint8_t x_align,
		 uint8_t y_align);


#endif /* SRC_BOOTLOADER_H_ */
// Initialize GPIO for button input



