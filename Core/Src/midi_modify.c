/*
 * midi_modify.c
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */
#include <stdio.h>
#include <stdint.h>

#include "midi_modify.h"
#include "cmsis_os.h"

#include "menu.h"
#include "utils.h"
#include "main.h"

extern osThreadId display_updateHandle;

static uint8_t  current_select = 0;
static uint8_t  old_select = 0;




//midi modify menu
void midi_modify_update_menu(TIM_HandleTypeDef * timer3,
		                     TIM_HandleTypeDef * timer4,
						     midi_modify_data_struct * midi_modify_data,
							 uint8_t * old_menu){

	midi_modify_data_struct old_modify_data = * midi_modify_data;
	uint8_t menu_changed = (*old_menu != MIDI_MODIFY);


	//The amount of values to be changed depends on the MIDI_MODIFY setting
    uint8_t amount_of_settings = (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE) ? 2 : 4;

	//Updating the selected item and see if it has changed
	utils_counter_change(timer3, &current_select, 0, amount_of_settings-1, menu_changed, 1, WRAP);
	uint8_t select_changed = (old_select != current_select);
	// Selecting the current item being selected
	for (uint8_t x=0; x < amount_of_settings; x++){
		select_states_midi_modify[x] = 0;
	}
	select_states_midi_modify[current_select] = 1;

	if (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE){
		switch (current_select) {
			case 0:
				utils_counter_change(timer4, &(midi_modify_data->send_to_midi_channel), 1, 16, select_changed, 1, NO_WRAP);
				break;
			case 1:
				if (midi_modify_data->velocity_type == MIDI_MODIFY_CHANGED_VEL){
					utils_counter_change_i32(timer4, &(midi_modify_data->velocity_plus_minus), -50, 50, select_changed, 10, NO_WRAP);
					break;
				}
				else if (midi_modify_data->velocity_type == MIDI_MODIFY_FIXED_VEL){
					utils_counter_change(timer4, &(midi_modify_data->velocity_absolute), 1, 16, select_changed, 1, NO_WRAP);
					break;
			}
		}

	}


	else if (midi_modify_data->change_or_split == MIDI_MODIFY_SPLIT){
		switch (current_select) {
		case 0:
			utils_counter_change(timer4, &(midi_modify_data->split_midi_channel_1), 1, 16, select_changed, 1, NO_WRAP);
			break;
		case 1:
			utils_counter_change(timer4, &(midi_modify_data->split_midi_channel_2), 1, 16, select_changed, 1, NO_WRAP);
			break;
		case 2:
			utils_counter_change(timer4, &(midi_modify_data->split_note), 0, 127, select_changed, 12, NO_WRAP);
			break;
		case 3:
			if (midi_modify_data->velocity_type == MIDI_MODIFY_CHANGED_VEL){
				utils_counter_change_i32(timer4, &(midi_modify_data->velocity_plus_minus), -50, 50, select_changed, 10, NO_WRAP);
				break;
			}
			else if (midi_modify_data->velocity_type == MIDI_MODIFY_FIXED_VEL){
				utils_counter_change(timer4, &(midi_modify_data->velocity_absolute), 0, 127, select_changed, 10, NO_WRAP);
				break;
			}
		}

	}

	if (menu_changed == 1 || old_select != current_select ||
		old_modify_data.send_to_midi_channel != midi_modify_data->send_to_midi_channel ||
		old_modify_data.split_note != midi_modify_data->split_note ||

		old_modify_data.split_midi_channel_1 != midi_modify_data->split_midi_channel_1 ||
		old_modify_data.split_midi_channel_2 != midi_modify_data->split_midi_channel_2 ||

		old_modify_data.velocity_plus_minus != midi_modify_data->velocity_plus_minus ||
		old_modify_data.velocity_absolute != midi_modify_data->velocity_absolute) {
		osThreadFlagsSet(display_updateHandle, 0x02);
		}
	*old_menu = MIDI_MODIFY;
	old_select = current_select;
}



