/*
 * midi_modify_transpose.c
 *
 *  Created on: Jul 10, 2025
 *      Author: Romain Dereu
 */


#include <stdio.h>
#include <stdint.h>


#include "cmsis_os.h"
#include "main.h"
#include "menu.h"
#include "midi_modify.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "text.h"
#include "utils.h"


static uint8_t current_select = 0;
static uint8_t old_select = 0;
static uint8_t select_states[3] = {0};

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
		select_states[x] = 0;
	}
	select_states[current_select] = 1;

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
																					OCTAVE_DOWN, OCTAVE_UP, select_changed, 1, NO_WRAP);
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



void screen_update_midi_transpose(midi_transpose_data_struct * midi_transpose_data){

	screen_driver_Fill(Black);
	menu_display(&Font_6x8, message->midi_transpose);

	if(midi_transpose_data->transpose_type == MIDI_TRANSPOSE_SHIFT){
		midi_transpose_shift_display(midi_transpose_data);
	}
	else if(midi_transpose_data->transpose_type == MIDI_TRANSPOSE_SCALED){
		midi_transpose_scaled_display(midi_transpose_data);
	}



	midi_modify_on_off(midi_transpose_data->currently_sending, 63);

    screen_driver_UpdateScreen();

}


void midi_transpose_shift_display(midi_transpose_data_struct * midi_transpose_data){

	screen_driver_SetCursor_WriteString(message->shift_by, Font_6x8 , White, TEXT_LEFT_START, LINE_1_VERT);
    char modify_value[5];
	int8_t plus_minus_i8 = midi_transpose_data->midi_shift_value;
    sprintf(modify_value, "%+d", plus_minus_i8);
    screen_driver_underline_WriteString(modify_value, Font_6x8, White, 65, LINE_1_VERT, select_states[0]);

	screen_driver_SetCursor_WriteString(message->semitones, Font_6x8 , White, TEXT_LEFT_START, LINE_2_VERT);


	screen_driver_SetCursor_WriteString(message->send_base, Font_6x8, White, TEXT_LEFT_START, LINE_4_VERT);
	const char * send_base_note_text = message->choices.no_yes[midi_transpose_data->send_original];
	screen_driver_underline_WriteString(send_base_note_text, Font_6x8, White, 65, LINE_4_VERT, select_states[1]);
}


void midi_transpose_scaled_display(midi_transpose_data_struct * midi_transpose_data){


	//Base Note
	screen_driver_SetCursor_WriteString(message->root_note, Font_6x8, White, TEXT_LEFT_START, LINE_1_VERT);
	const char * base_note_text = message->twelve_notes_names[midi_transpose_data->transpose_base_note];
	screen_driver_underline_WriteString(base_note_text, Font_6x8, White, 62, LINE_1_VERT, select_states[0]);


	//Transpose_interval
	screen_driver_SetCursor_WriteString(message->interval, Font_6x8, White, TEXT_LEFT_START, LINE_2_VERT);
	const char * interval_text = message->choices.intervals[midi_transpose_data->transpose_interval];
	screen_driver_underline_WriteString(interval_text, Font_6x8, White, 55, LINE_2_VERT, select_states[1]);


	//Transpose_scale
	screen_driver_SetCursor_WriteString(message->scale, Font_6x8, White, TEXT_LEFT_START, LINE_3_VERT);
	const char * scale_text = message->choices.scales[midi_transpose_data->transpose_scale];
	screen_driver_underline_WriteString(scale_text, Font_6x8, White, 40, LINE_3_VERT, select_states[2]);

	//Send base
	screen_driver_SetCursor_WriteString(message->send_base, Font_6x8, White, TEXT_LEFT_START, LINE_4_VERT);
	const char * send_base_note_text = message->choices.no_yes[midi_transpose_data->send_original];
	screen_driver_underline_WriteString(send_base_note_text, Font_6x8, White, 65, LINE_4_VERT, select_states[3]);

}
