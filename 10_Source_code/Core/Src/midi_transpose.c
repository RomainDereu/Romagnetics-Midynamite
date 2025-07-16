/*
 * midi_modify_transpose.c
 *
 *  Created on: Jul 10, 2025
 *      Author: Astaa
 */


#include <stdio.h>
#include <stdint.h>

#include "midi_modify.h"
#include "cmsis_os.h"

#include "menu.h"
#include "utils.h"
#include "main.h"


static uint8_t current_select = 0;
static uint8_t old_select = 0;

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern osThreadId display_updateHandle;


void midi_transpose_update_menu(TIM_HandleTypeDef * timer3,
		                     TIM_HandleTypeDef * timer4,
						     midi_transpose_data_struct * midi_transpose_data,
							 uint8_t * old_menu){

	midi_transpose_data_struct old_transpose_data = * midi_transpose_data;
	uint8_t menu_changed = (*old_menu != MIDI_TRANSPOSE);

	//The amount of values to be changed depends on the transpose_type setting
    uint8_t amount_of_settings = (midi_transpose_data->transpose_type == MIDI_TRANSPOSE_SCALED) ? 4 : 2;

	utils_counter_change(timer3, &current_select, 0, amount_of_settings-1, menu_changed, 1, WRAP);
	uint8_t select_changed = (old_select != current_select);

	// Finding urrent item being selected
	for (uint8_t x=0; x < amount_of_settings; x++){
		select_states_midi_transpose[x] = 0;
	}
	select_states_midi_transpose[current_select] = 1;

	if (midi_transpose_data->transpose_type == MIDI_TRANSPOSE_SHIFT){
		switch (current_select) {
			case 0:
				utils_counter_change_i32(timer4, &(midi_transpose_data->midi_shift_value), -24, 24, select_changed, 12, NO_WRAP);
				break;

			case 1:
				utils_counter_change(timer4, &(midi_transpose_data->send_original), 0, 1, select_changed, 1, WRAP);
				break;

		}
	}

	else if (midi_transpose_data->transpose_type == MIDI_TRANSPOSE_SCALED){
		switch (current_select) {
			case 0:
				utils_counter_change(timer4, &(midi_transpose_data->transpose_base_note),
																					0, 11, select_changed, 12, NO_WRAP);
				break;

			case 1:
				utils_counter_change(timer4, &(midi_transpose_data->transpose_interval),
																					OCTAVE_DOWN, OCTAVE_UP, select_changed, 1, WRAP);
				break;

			case 2:
				utils_counter_change(timer4, &(midi_transpose_data->transpose_scale),
																					IONIAN, LOCRIAN, select_changed, 1, WRAP);
				break;

			case 3:
				utils_counter_change(timer4, &(midi_transpose_data->send_original), 0, 1, select_changed, 1, WRAP);
				break;

		}

	}



	if (menu_changed == 1 || old_select != current_select ||
	    old_transpose_data.midi_shift_value != midi_transpose_data->midi_shift_value ||
	    old_transpose_data.send_original != midi_transpose_data-> send_original||
	    old_transpose_data.transpose_base_note != midi_transpose_data->transpose_base_note ||
	    old_transpose_data.transpose_scale != midi_transpose_data->transpose_scale ||
	    old_transpose_data.transpose_interval != midi_transpose_data->transpose_interval ||
	    old_transpose_data.currently_sending != midi_transpose_data->currently_sending){
		osThreadFlagsSet(display_updateHandle, 0x04);
	}

	*old_menu = MIDI_TRANSPOSE;
	old_select = current_select;

}
