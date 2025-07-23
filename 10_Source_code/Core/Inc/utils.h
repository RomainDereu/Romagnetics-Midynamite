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

#define LINE_1_VERT 15
#define LINE_2_VERT 25
#define LINE_3_VERT 35
#define LINE_4_VERT 45

#define TEXT_LEFT_START 5


#define WRAP 0
#define NO_WRAP 1




//Making an array with the list of uarts that will be used
void list_of_UART_to_send_to(uint8_t send_channels,
	                       	 UART_HandleTypeDef **UART_list);


void utils_counter_change_i32(TIM_HandleTypeDef * timer,
                              int32_t * data_to_change,
                              int32_t bottom_value,
                              int32_t max_value,
                              uint8_t menu_changed,
                              uint8_t multiplier,
                              uint8_t wrap_or_not);

void utils_counter_change(TIM_HandleTypeDef * timer,
		                   uint8_t * data_to_change,
						   int32_t bottom_value,
						   int32_t max_value,
						   uint8_t menu_changed,
						   uint8_t multiplier,
						   uint8_t wrap_or_not);



void screen_driver_SetCursor_WriteString(const char* str, screen_driver_Font_t font,
										 screen_driver_COLOR color,
										 uint8_t x_align,
										 uint8_t y_align);


void screen_driver_underline_WriteString(const char* str, screen_driver_Font_t font,
										  screen_driver_COLOR color,
										  uint8_t x_align,
										  uint8_t y_align,
										  uint8_t underlined);


void panic_midi_all_notes_off(UART_HandleTypeDef *huart);
void panic_midi_all_notes_off_both(UART_HandleTypeDef *huart1, UART_HandleTypeDef *huart2);




#endif /* SRC_UTILS_H_ */
