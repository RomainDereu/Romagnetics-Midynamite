/*
 * bootloader.h
 *
 *  Created on: Jul 23, 2025
 *      Author: Astaa
 */

#ifndef __BOOTLOADER_H__
#define __BOOTLOADER_H__

#define APP_START_ADDRESS      0x08010000
#define MAX_FIRMWARE_SIZE_BYTES (320 * 1024)


#define FAT_BOOT_SECTOR_ADDR       APP_START_ADDRESS
#define FAT_BYTES_PER_SECTOR       512
#define FAT_RESERVED_SECTORS       1
#define FAT_NUM_FATS               2
#define FAT_ROOT_ENTRIES           16
#define FAT_SECTORS_PER_FAT        8
#define FAT_SECTORS_PER_CLUSTER    1

#define UPDATE_FILENAME  "FIRMWAREBIN"


#ifndef MIN
#define MIN(a,b) ((a) < (b) ? (a) : (b))
#endif


#include <stdint.h>

#include "screen_driver.h"
#include "screen_driver_fonts.h"



extern volatile uint8_t  g_file_detected;

extern volatile uint8_t g_crc_failed;
extern volatile uint8_t  g_update_complete;



extern uint8_t update_failed;

/* USER CODE END PV */




void Bootloader_StartFirmwareUpdate(void);

uint8_t Bootloader_WriteFirmwareChunk(uint32_t address, const uint8_t *data, uint32_t length);
uint8_t Bootloader_EndFirmwareUpdate(uint32_t g_expected_length);
typedef void (*pFunction)(void);
void Bootloader_JumpToApplication(void);

void Bootloader_InitCRC32(void);
uint32_t Bootloader_ComputeCRC32(uint32_t addr, uint32_t size);


void Bootloader_HandleFatalError();



#endif /* __BOOTLOADER_H__ */
