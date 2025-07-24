/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : usbd_storage_if.c
  * @version        : v1.0_Cube
  * @brief          : Memory management layer.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2025 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "usbd_storage_if.h"

/* USER CODE BEGIN INCLUDE */
#include "bootloader.h"

/* USER CODE END INCLUDE */

/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/

/* USER CODE BEGIN PV */
/* Private variables ---------------------------------------------------------*/
volatile uint8_t  g_file_detected      = 0;
volatile uint8_t  g_data_cluster_seen  = 0;
volatile uint8_t  g_update_complete    = 0;
volatile uint8_t g_erase_requested = 0;
volatile uint32_t g_expected_length    = 0;
volatile uint32_t g_bytes_written      = 0;
volatile uint32_t g_file_data_start_sector = 0;
/* USER CODE END PV */

/** @addtogroup STM32_USB_OTG_DEVICE_LIBRARY
  * @brief Usb device.
  * @{
  */

/** @defgroup USBD_STORAGE
  * @brief Usb mass storage device module
  * @{
  */

/** @defgroup USBD_STORAGE_Private_TypesDefinitions
  * @brief Private types.
  * @{
  */

/* USER CODE BEGIN PRIVATE_TYPES */

/* USER CODE END PRIVATE_TYPES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Defines
  * @brief Private defines.
  * @{
  */
/* USER CODE BEGIN PRIVATE_DEFINES */

/* Private defines */
#define STORAGE_LUN_NBR            1
#define STORAGE_BLK_SIZ            512
#define STORAGE_BLK_NBR            ((320*1024) / STORAGE_BLK_SIZ)  // 320KB total

/* FAT16 layout constants */
#define FAT_BYTES_PER_SECTOR       512
#define FAT_SECTORS_PER_CLUSTER    1
#define FAT_RESERVED_SECTORS       1
#define FAT_NUM_FATS               2
#define FAT_ROOT_ENTRIES           16
#define FAT_SECTORS_PER_FAT        8
#define FAT_ROOT_DIR_SECTORS       ((FAT_ROOT_ENTRIES * 32 + FAT_BYTES_PER_SECTOR - 1) \ / FAT_BYTES_PER_SECTOR)
#define FAT_TOTAL_SECTORS          ((320*1024) / FAT_BYTES_PER_SECTOR)
#define FAT_FIRST_DATA_SECTOR      18




/* USER CODE END PRIVATE_DEFINES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Macros
  * @brief Private macros.
  * @{
  */

/* USER CODE BEGIN PRIVATE_MACRO */

/* USER CODE END PRIVATE_MACRO */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_Variables
  * @brief Private variables.
  * @{
  */

/* USER CODE BEGIN INQUIRY_DATA_FS */

/* In‑RAM FAT + root‑dir */
static uint8_t MSC_RamDisk[FAT_FIRST_DATA_SECTOR * FAT_BYTES_PER_SECTOR];


/** USB Mass storage Standard Inquiry Data. */
const int8_t STORAGE_Inquirydata_FS[] = {/* 36 */

  /* LUN 0 */
  0x00,
  0x80,
  0x02,
  0x02,
  (STANDARD_INQUIRY_DATA_LEN - 5),
  0x00,
  0x00,
  0x00,
  'S', 'T', 'M', ' ', ' ', ' ', ' ', ' ', /* Manufacturer : 8 bytes */
  'P', 'r', 'o', 'd', 'u', 'c', 't', ' ', /* Product      : 16 Bytes */
  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
  '0', '.', '0' ,'1'                      /* Version      : 4 Bytes */
};
/* USER CODE END INQUIRY_DATA_FS */

/* USER CODE BEGIN PRIVATE_VARIABLES */

/* USER CODE END PRIVATE_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Exported_Variables
  * @brief Public variables.
  * @{
  */

extern USBD_HandleTypeDef hUsbDeviceFS;

/* USER CODE BEGIN EXPORTED_VARIABLES */

/* USER CODE END EXPORTED_VARIABLES */

/**
  * @}
  */

/** @defgroup USBD_STORAGE_Private_FunctionPrototypes
  * @brief Private functions declaration.
  * @{
  */

static int8_t STORAGE_Init_FS(uint8_t lun);
static int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size);
static int8_t STORAGE_IsReady_FS(uint8_t lun);
static int8_t STORAGE_IsWriteProtected_FS(uint8_t lun);
static int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len);
static int8_t STORAGE_GetMaxLun_FS(void);

/* USER CODE BEGIN PRIVATE_FUNCTIONS_DECLARATION */

/* USER CODE END PRIVATE_FUNCTIONS_DECLARATION */

/**
  * @}
  */

USBD_StorageTypeDef USBD_Storage_Interface_fops_FS =
{
  STORAGE_Init_FS,
  STORAGE_GetCapacity_FS,
  STORAGE_IsReady_FS,
  STORAGE_IsWriteProtected_FS,
  STORAGE_Read_FS,
  STORAGE_Write_FS,
  STORAGE_GetMaxLun_FS,
  (int8_t *)STORAGE_Inquirydata_FS
};

/* Private functions ---------------------------------------------------------*/
/**
  * @brief  Initializes the storage unit (medium) over USB FS IP
  * @param  lun: Logical unit number.
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Init_FS(uint8_t lun)
{
  /* USER CODE BEGIN 2 */
	UNUSED(lun);

	  // 1) clear the FAT+root area
	  memset(MSC_RamDisk, 0x00, sizeof(MSC_RamDisk));

	  // 2) --- BIOS Parameter Block (boot sector) ---
	  uint8_t *bpb = MSC_RamDisk;
	  bpb[0]=0xEB; bpb[1]=0x3C; bpb[2]=0x90;                        // JMP instruction
	  memcpy(&bpb[3], "MSDOS5.0", 8);                               // OEM Name
	  bpb[11]=LOBYTE(FAT_BYTES_PER_SECTOR);
	  bpb[12]=HIBYTE(FAT_BYTES_PER_SECTOR);                         // bytes per sector = 512
	  bpb[13]=FAT_SECTORS_PER_CLUSTER;                              // sectors per cluster = 1
	  bpb[14]=LOBYTE(FAT_RESERVED_SECTORS);
	  bpb[15]=HIBYTE(FAT_RESERVED_SECTORS);                         // reserved sectors = 1
	  bpb[16]=FAT_NUM_FATS;                                         // number of FAT copies = 2
	  bpb[17]=LOBYTE(FAT_ROOT_ENTRIES);
	  bpb[18]=HIBYTE(FAT_ROOT_ENTRIES);                             // max root entries = 16
	  bpb[19]=LOBYTE(FAT_TOTAL_SECTORS);
	  bpb[20]=HIBYTE(FAT_TOTAL_SECTORS);                            // total sectors = 640
	  bpb[21]=0xF8;                                                 // media descriptor
	  bpb[22]=LOBYTE(FAT_SECTORS_PER_FAT);
	  bpb[23]=HIBYTE(FAT_SECTORS_PER_FAT);                          // sectors per FAT = 8
	  // leave bpb[24..509] = 0
	  bpb[510]=0x55; bpb[511]=0xAA;                                 // boot signature

	  // 3) --- initialize both FAT copies ---
	  uint16_t *fat1 = (uint16_t *)&MSC_RamDisk[ FAT_BYTES_PER_SECTOR /* sector 1 */ ];
	  // Mark clusters 0 & 1 as reserved
	  fat1[0] = 0xFFF8;
	  fat1[1] = 0xFFFF;
	  // Force cluster 2 to be "allocated, end‑of‑chain"
	  fat1[2] = 0xFFFF;

	  // Mirror FAT1 into FAT2
	  uint8_t *fat2 = MSC_RamDisk
	      + (FAT_RESERVED_SECTORS + FAT_SECTORS_PER_FAT) * FAT_BYTES_PER_SECTOR;
	  memcpy(fat2,
	         &MSC_RamDisk[ FAT_BYTES_PER_SECTOR ],
	         FAT_SECTORS_PER_FAT * FAT_BYTES_PER_SECTOR);


	  return USBD_OK;
  /* USER CODE END 2 */
}

/**
  * @brief  Returns the medium capacity.
  * @param  lun: Logical unit number.
  * @param  block_num: Number of total block number.
  * @param  block_size: Block size.
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_GetCapacity_FS(uint8_t lun, uint32_t *block_num, uint16_t *block_size)
{
  /* USER CODE BEGIN 3 */
  UNUSED(lun);

  *block_num  = STORAGE_BLK_NBR;
  *block_size = STORAGE_BLK_SIZ;
  return (USBD_OK);
  /* USER CODE END 3 */
}

/**
  * @brief   Checks whether the medium is ready.
  * @param  lun:  Logical unit number.
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_IsReady_FS(uint8_t lun)
{
  /* USER CODE BEGIN 4 */
  UNUSED(lun);

  return (USBD_OK);
  /* USER CODE END 4 */
}

/**
  * @brief  Checks whether the medium is write protected.
  * @param  lun: Logical unit number.
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_IsWriteProtected_FS(uint8_t lun)
{
  /* USER CODE BEGIN 5 */
  UNUSED(lun);

  return (USBD_OK);
  /* USER CODE END 5 */
}

/**
  * @brief  Reads data from the medium.
  * @param  lun: Logical unit number.
  * @param  buf: data buffer.
  * @param  blk_addr: Logical block address.
  * @param  blk_len: Blocks number.
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Read_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
	  UNUSED(lun);
	  if (blk_addr < FAT_FIRST_DATA_SECTOR) {
	    memcpy(buf,
	           MSC_RamDisk + blk_addr*FAT_BYTES_PER_SECTOR,
	           blk_len*FAT_BYTES_PER_SECTOR);
	    return USBD_OK;
	  }
	  // data region: map to flash
	  uint32_t flash_offset = (blk_addr - FAT_FIRST_DATA_SECTOR)*FAT_BYTES_PER_SECTOR;
	  uint8_t *src = (uint8_t*)(APP_START_ADDRESS + flash_offset);
	  memcpy(buf, src, blk_len*FAT_BYTES_PER_SECTOR);
	  return USBD_OK;
  /* USER CODE END 6 */
}

/**
  * @brief  Writes data into the medium.
  * @param  lun: Logical unit number.
  * @param  buf: data buffer.
  * @param  blk_addr: Logical block address.
  * @param  blk_len: Blocks number.
  * @retval USBD_OK if all operations are OK else USBD_FAIL
  */
int8_t STORAGE_Write_FS(uint8_t lun, uint8_t *buf, uint32_t blk_addr, uint16_t blk_len)
{
  /* USER CODE BEGIN 7 */
    UNUSED(lun);


    // 1) Mirror FAT+root‑dir so host sees a valid FAT16 FS:
    if (blk_addr < FAT_FIRST_DATA_SECTOR) {
        memcpy(&MSC_RamDisk[blk_addr * FAT_BYTES_PER_SECTOR],
               buf,
               blk_len * FAT_BYTES_PER_SECTOR);

        // If this is a root‑dir sector, scan for our 8.3 entry:
        uint32_t rootStart = FAT_RESERVED_SECTORS
                           + FAT_NUM_FATS * FAT_SECTORS_PER_FAT;
        if (blk_addr >= rootStart) {
            for (int off = 0; off < blk_len * FAT_BYTES_PER_SECTOR; off += 32) {
                if (memcmp(buf + off, UPDATE_FILENAME, 11) == 0) {
                    // found "FIRMWAREBIN"
                    uint16_t firstCluster =
                        buf[off + 26]
                      | (buf[off + 27] << 8);
                    // host will start writing data at cluster #2 => LBA = FAT_FIRST_DATA_SECTOR,
                    // so cluster N lands at FAT_FIRST_DATA_SECTOR + (N‑2)
                    g_file_data_start_sector = FAT_FIRST_DATA_SECTOR
                                             + (firstCluster - 2);
                    // pull expected file size (bytes 28..31)
                    g_expected_length =
                          buf[off + 28]
                        | (buf[off + 29] << 8)
                        | (buf[off + 30] << 16)
                        | (buf[off + 31] << 24);
                    g_file_detected = 1;
                    g_erase_requested = 1;  // notify main loop to do the big sector erase
                    break;
                }
            }
        }
        return USBD_OK;
    }

    // 2) If the host hasn't created our .BIN entry yet, ignore data‐cluster writes
    if (!g_file_detected) {
        return USBD_OK;
    }

    // 3) First data‐cluster write: kick off the erase in main context
    if (!g_data_cluster_seen) {
        g_data_cluster_seen = 1;
        g_bytes_written    = 0;
        Bootloader_StartFirmwareUpdate();
    }

    // 4) Compute file‐relative offset
    uint32_t data_idx   = blk_addr - g_file_data_start_sector;
    uint32_t flash_addr = APP_START_ADDRESS
                        + data_idx * FAT_BYTES_PER_SECTOR;

    // 5) Program each 4‑byte word
    for (uint32_t off = 0; off < blk_len * FAT_BYTES_PER_SECTOR; off += 4) {
        uint32_t word = 0xFFFFFFFF;
        memcpy(&word, buf + off, 4);
        __disable_irq();
        HAL_StatusTypeDef st = HAL_FLASH_Program(
            FLASH_TYPEPROGRAM_WORD,
            flash_addr + off,
            word
        );
        __enable_irq();
        if (st != HAL_OK) {
            // flash error
            return USBD_FAIL;
        }
    }

    // 6) Track progress & finish
    g_bytes_written += blk_len * FAT_BYTES_PER_SECTOR;
    if (g_bytes_written >= g_expected_length) {
        Bootloader_EndFirmwareUpdate();
        g_update_complete = 1;
    }

    return USBD_OK;
  /* USER CODE END 7 */
}

/**
  * @brief  Returns the Max Supported LUNs.
  * @param  None
  * @retval Lun(s) number.
  */
int8_t STORAGE_GetMaxLun_FS(void)
{
  /* USER CODE BEGIN 8 */
  return (STORAGE_LUN_NBR - 1);
  /* USER CODE END 8 */
}

/* USER CODE BEGIN PRIVATE_FUNCTIONS_IMPLEMENTATION */

/* USER CODE END PRIVATE_FUNCTIONS_IMPLEMENTATION */

/**
  * @}
  */

/**
  * @}
  */

