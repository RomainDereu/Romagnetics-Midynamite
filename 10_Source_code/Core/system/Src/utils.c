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

#include "text.h"

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


void utils_change_settings(uint8_t * data_to_change, int8_t bottom_value, int32_t max_value){
	(*data_to_change)++;
	if(*data_to_change > max_value){
		*data_to_change = bottom_value;
	}

}






// Panic on a single UART
void all_notes_off(UART_HandleTypeDef *huart) {
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
void panic_midi(UART_HandleTypeDef *huart1,
			    UART_HandleTypeDef *huart2,
				GPIO_TypeDef *port,
				uint16_t pin1,
				uint16_t pin2) {

	if (HAL_GPIO_ReadPin(port, pin1) == GPIO_PIN_RESET &&
	    HAL_GPIO_ReadPin(port, pin2) == GPIO_PIN_RESET) {

    osDelay(300); // Debounce delay

    if (HAL_GPIO_ReadPin(port, pin1) == GPIO_PIN_RESET &&
        HAL_GPIO_ReadPin(port, pin2) == GPIO_PIN_RESET) {

    	all_notes_off(huart1);
    	all_notes_off(huart2);
    }
}

}




//On/ Off Part
void midi_display_on_off(uint8_t on_or_off, uint8_t bottom_line){
	screen_driver_Line(92, 10, 92, bottom_line, White);
	uint8_t text_position = bottom_line/2;
    screen_driver_SetCursor(95, text_position);

    if(on_or_off ==0){
  	  screen_driver_WriteString(message->off, Font_11x18 , White);
    }
    else if (on_or_off ==1){
  	  screen_driver_WriteString(message->on, Font_11x18 , White);
    }

}




//


uint8_t handle_menu_toggle(GPIO_TypeDef *port,
                           uint16_t pin1,
                           uint16_t pin2)
{
    static uint8_t prev_state = 1;
    uint8_t s1 = HAL_GPIO_ReadPin(port, pin1);
    uint8_t s2 = HAL_GPIO_ReadPin(port, pin2);

    // detect a fresh press of pin1 while pin2 is held
    if (s1 == 0 && prev_state == 1 && s2 == 1) {
        osDelay(100);
        if (HAL_GPIO_ReadPin(port, pin1) == 0 &&
            HAL_GPIO_ReadPin(port, pin2) == 1)
        {
            prev_state = 0;
            return 1;
        }
    }

    prev_state = s1;
    return 0;
}


uint8_t debounce_button(GPIO_TypeDef *port,
		                uint16_t      pin,
		                uint8_t     *prev_state,
		                uint32_t      db_ms)
		{
		    uint8_t cur = HAL_GPIO_ReadPin(port, pin);

		    // If we have a prev_state pointer, only fire on high→low (prev==1 && cur==0).
		    // Otherwise, fire on cur==0 immediately.
		    if (cur == 0 && (!prev_state || *prev_state == 1)) {
		        osDelay(db_ms);
		        cur = HAL_GPIO_ReadPin(port, pin);
		        if (cur == 0) {
		            if (prev_state) *prev_state = 0;
		            return 1;
		        }
		    }

		    // remember state if tracking
		    if (prev_state) *prev_state = cur;
		    return 0;
		}


//GUI function setting a flag on the currently selected item
void select_current_state(uint8_t *select_states,
                          uint8_t  amount,
                          uint8_t  current_select)
{
    for (uint8_t i = 0; i < amount; i++) {
        select_states[i] = 0;
    }
    select_states[current_select] = 1;
}



//Checks for updates to a menu and refreshes the screen if needed
uint8_t menu_check_for_updates(
    uint8_t   menu_changed,
    const void *old_data,
    const void *data_ptr,
    size_t    sz,
    uint8_t       *old_select,
    uint8_t       *current_select) {
	uint8_t changed =  ( menu_changed
      || (*old_select != *current_select)
      || memcmp(old_data, data_ptr, sz) != 0
    );
	return changed;
}



