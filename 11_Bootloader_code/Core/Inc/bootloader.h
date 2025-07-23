/*
 * bootloader.h
 *
 *  Created on: Jul 23, 2025
 *      Author: Astaa
 */

#ifndef INC_BOOTLOADER_C_
#define INC_BOOTLOADER_C_

#include "screen_driver.h"
#include "screen_driver_fonts.h"



typedef void (*pFunction)(void);

// Function to jump to the main application
void Bootloader_JumpToApplication(void);

void screen_driver_SetCursor_WriteString(const char* str, screen_driver_Font_t font,
		 screen_driver_COLOR color,
		 uint8_t x_align,
		 uint8_t y_align);


#endif /* SRC_BOOTLOADER_H_ */
// Initialize GPIO for button input



