/*
 * bootloader.h
 *
 *  Created on: Jul 23, 2025
 *      Author: Astaa
 */

#ifndef INC_BOOTLOADER_C_
#define INC_BOOTLOADER_C_

uint8_t check_bootloader_button(void);

void jump_to_application(void);

void poll_mass_storage_for_firmware_update(void);

uint8_t find_firmware_file_in_ramdisk(uint32_t *fw_offset,
										uint32_t *fw_size);

void flash_write_application_area(uint8_t *data, uint32_t size);

void flash_erase_application_area(void);


#endif /* SRC_BOOTLOADER_H_ */
