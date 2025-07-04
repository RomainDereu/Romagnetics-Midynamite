/*
 * utils.c
 *
 *  Created on: Jun 21, 2025
 *      Author: Romain Dereu
 */
#include <stdio.h>
#include <string.h>

#include "utils.h"
#include "cmsis_os.h"

#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "saving.h"


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
		                       int32_t * data_to_change,
							   int32_t bottom_value,
							   int32_t max_value,
							   uint8_t menu_changed,
							   uint8_t multiplier,
							   uint8_t wrap_or_not){
	static uint32_t old_value;
    if (menu_changed == 0) {

    	uint8_t active_multiplier = 1;
    	if(multiplier != 1){
            uint8_t Btn2State = HAL_GPIO_ReadPin(GPIOB, Btn2_Pin);
            active_multiplier = (Btn2State == 0) ? multiplier : 1;
    	}


        int32_t delta = __HAL_TIM_GET_COUNTER(timer) - ENCODER_CENTER;
        if (delta >= ENCODER_THRESHOLD) {
            if (*data_to_change + active_multiplier > max_value) {
                *data_to_change = (wrap_or_not == WRAP) ? bottom_value : max_value;
            } else {
                *data_to_change += active_multiplier;
            }
            __HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER);
        }
        else if (delta <= -ENCODER_THRESHOLD) {
            if (*data_to_change - active_multiplier < bottom_value) {
                *data_to_change = (wrap_or_not == WRAP) ? max_value : bottom_value;
            } else {
                *data_to_change -= active_multiplier;
            }
            __HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER);
        }
    }
	if (menu_changed == 1) {
		__HAL_TIM_SET_COUNTER(timer, ENCODER_CENTER);
	}
}


void screen_driver_SetCursor_WriteString(const char* str, screen_driver_Font_t Font,
										 screen_driver_COLOR color,
										 uint8_t x_align,
										 uint8_t y_align){
	screen_driver_SetCursor(x_align, y_align);
	screen_driver_WriteString(str, Font_6x8 , color);
}



void screen_driver_underline_WriteString(const char* str, screen_driver_Font_t Font,
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






