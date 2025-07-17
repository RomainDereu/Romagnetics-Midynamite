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

	else if (send_channels == MIDI_OUT_1_2 || send_channels == MIDI_OUT_SPLIT){
		UART_list[0] = &huart1;
		UART_list[1] = &huart2;
	}

	else{
		UART_list[0] = NULL;
		UART_list[1] = NULL;
	}

}



void utils_counter_change_i32(TIM_HandleTypeDef * timer,
		                       int32_t * data_to_change,
							   int32_t bottom_value,
							   int32_t max_value,
							   uint8_t menu_changed,
							   uint8_t multiplier,
							   uint8_t wrap_or_not){
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

void utils_counter_change(TIM_HandleTypeDef * timer,
                          uint8_t * data_to_change,
						  int32_t bottom_value,
						  int32_t max_value,
                          uint8_t menu_changed,
                          uint8_t multiplier,
                          uint8_t wrap_or_not)
{
    int32_t temp_data = *data_to_change;
    utils_counter_change_i32(timer, &temp_data, bottom_value, max_value, menu_changed, multiplier, wrap_or_not);
    *data_to_change = (uint8_t)temp_data;
}


void screen_driver_SetCursor_WriteString(const char* str, screen_driver_Font_t font,
										 screen_driver_COLOR color,
										 uint8_t x_align,
										 uint8_t y_align){
	screen_driver_SetCursor(x_align, y_align);
	screen_driver_WriteString(str, font , color);
}



void screen_driver_underline_WriteString(const char* str, screen_driver_Font_t font,
										  screen_driver_COLOR color,
										  uint8_t x_align,
										  uint8_t y_align,
										  uint8_t underlined){

	uint8_t line_height = font.height +1;
	screen_driver_SetCursor(x_align, y_align);
	screen_driver_WriteString(str, font , color);
	if(underlined == 1){
		screen_driver_SetCursor(x_align, y_align +1);
		uint8_t line_length = font.width * strlen(str);
		screen_driver_Line(x_align, y_align + line_height,
						   x_align+ line_length, y_align + line_height, White);

	}
}



// Panic on a single UART
void panic_midi_all_notes_off(UART_HandleTypeDef *huart) {
    uint8_t all_notes_off_msg[3];
    uint8_t reset_ctrl_msg[3];

    for (uint8_t channel = 0; channel < 16; channel++) {
        uint8_t status = 0xB0 | channel;

        // All Notes Off
        all_notes_off_msg[0] = status;
        all_notes_off_msg[1] = 123;  // Controller number for All Notes Off
        all_notes_off_msg[2] = 0;

        // Reset All Controllers
        reset_ctrl_msg[0] = status;
        reset_ctrl_msg[1] = 121;     // Controller number for Reset All Controllers
        reset_ctrl_msg[2] = 0;

        HAL_UART_Transmit(huart, all_notes_off_msg, 3, 1000);
        HAL_UART_Transmit(huart, reset_ctrl_msg, 3, 1000);
    }
}

// Panic on both UART1 and UART2
void panic_midi_all_notes_off_both(UART_HandleTypeDef *huart1, UART_HandleTypeDef *huart2) {
    panic_midi_all_notes_off(huart1);
    panic_midi_all_notes_off(huart2);
}



