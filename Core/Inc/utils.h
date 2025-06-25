/*
 * utils.h
 *
 *  Created on: Jun 21, 2025
 *      Author: Romain Dereu
 */

#ifndef SRC_UTILS_H_
#define SRC_UTILS_H_

#include "stdio.h"
#include "main.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"

#endif /* SRC_UTILS_H_ */



void utils_counter_change(TIM_HandleTypeDef * timer,
		                   uint8_t * data_to_change,
						   uint8_t bottom_value,
						   uint8_t max_value,
						   uint8_t menu_changed);


void screen_update_settings();

void screen_driver_underline_WriteString(char* str, screen_driver_Font_t Font,
										  screen_driver_COLOR color,
										  uint8_t x_align,
										  uint8_t y_align,
										  uint8_t underlined);
