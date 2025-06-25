/*
 * utils.c
 *
 *  Created on: Jun 21, 2025
 *      Author: Astaa
 */
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "cmsis_os.h"

#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "saving.h"

extern osThreadId display_updateHandle;



void list_of_UART_to_send_to(uint8_t send_channels,
                           	 UART_HandleTypeDef **UART_list){
	extern UART_HandleTypeDef huart1;
	extern UART_HandleTypeDef huart2;

	if (send_channels == MIDI_OUT_1){
		UART_list[0] = &huart1;
		UART_list[1] = NULL;
	}

	else if (send_channels == MIDI_OUT_2){
		UART_list[0] = &huart2;
		UART_list[1] = NULL;
	}

	else if (send_channels == MIDI_OUT_1_2){
		UART_list[0] = &huart1;
		UART_list[1] = &huart2;
	}

	else{
		UART_list[0] = NULL;
		UART_list[1] = NULL;
	}

}



void utils_counter_change(TIM_HandleTypeDef * timer,
							   uint8_t * data_to_change,
							   uint8_t bottom_value,
							   uint8_t max_value,
							   uint8_t menu_changed){
	static uint32_t old_value;
    if (menu_changed == 0) {

		int32_t timer_count = __HAL_TIM_GET_COUNTER(timer);
		int32_t delta = timer_count - ENCODER_CENTER;

        if (delta >= ENCODER_THRESHOLD) {
        	if (*data_to_change == max_value)
        		*data_to_change = bottom_value;
        	else
        		(*data_to_change)++;
        	__HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER);
        }


        else if (delta <= -ENCODER_THRESHOLD) {
        	if (*data_to_change == bottom_value)
        		*data_to_change = max_value;
        	else
        		(*data_to_change)--;
        	__HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER);
        }


        if (old_value != * data_to_change) {
        	old_value = * data_to_change;
            osThreadFlagsSet(display_updateHandle, 0x01);
        }
    }
	if (menu_changed == 1) {
		__HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER);
	}
}


void screen_driver_SetCursor_WriteString(char* str, screen_driver_Font_t Font,
										 screen_driver_COLOR color,
										 uint8_t x_align,
										 uint8_t y_align){
	screen_driver_SetCursor(x_align, y_align);
	screen_driver_WriteString(str, Font_6x8 , color);
}



void screen_driver_underline_WriteString(char* str, screen_driver_Font_t Font,
										  screen_driver_COLOR color,
										  uint8_t x_align,
										  uint8_t y_align,
										  uint8_t underlined){

	uint8_t line_height = Font.height +1;
	screen_driver_SetCursor(x_align, y_align);
	screen_driver_WriteString(str, Font_6x8 , color);
	if(underlined == 1){
		screen_driver_SetCursor(x_align, y_align +1);
		uint8_t line_length = Font.width * strlen(str);
		screen_driver_Line(x_align, y_align + line_height,
						   x_align+ line_length, y_align + line_height, White);

	}
}






