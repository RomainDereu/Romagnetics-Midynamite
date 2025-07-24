/*
 * bootloader.h
 *
 *  Created on: Jul 23, 2025
 *      Author: Astaa
 */

#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#define APP_START_ADDRESS      0x08010000
#define FIRMWARE_MAX_SIZE      (320 * 1024)
#define UPDATE_FILENAME        "FIRMWIREBIN"  // 11-char 8.3 FAT name

#define MAX_FIRMWARE_SIZE_BYTES (320 * 1024)


#define FAT_BOOT_SECTOR_ADDR       APP_START_ADDRESS
#define FAT_BYTES_PER_SECTOR       512
#define FAT_RESERVED_SECTORS       1
#define FAT_NUM_FATS               2
#define FAT_ROOT_ENTRIES           16
#define FAT_SECTORS_PER_FAT        8
#define FAT_SECTORS_PER_CLUSTER    1

#define UPDATE_FILENAME  "FIRMWIREBIN"  // 11 chars, no dot


#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif


#include <stdint.h>

#include "screen_driver.h"
#include "screen_driver_fonts.h"



extern volatile uint8_t g_file_detected;
extern volatile uint8_t g_update_complete;
extern volatile uint8_t g_data_cluster_seen;




uint8_t Bootloader_CheckFirmwareSize(uint32_t file_size_bytes);
void Bootloader_StartFirmwareUpdate(void);
uint8_t Bootloader_WriteFirmwareChunk(uint32_t address, const uint8_t *data, uint32_t length);
void Bootloader_EndFirmwareUpdate(void);



typedef void (*pFunction)(void);

// Function to jump to the main application
void Bootloader_JumpToApplication(void);


void Bootloader_FormatFlashFAT16(void);

void screen_driver_SetCursor_WriteString(const char* str, screen_driver_Font_t font,
		 screen_driver_COLOR color,
		 uint8_t x_align,
		 uint8_t y_align);


#endif /* __BOOTLOADER_H__ */
