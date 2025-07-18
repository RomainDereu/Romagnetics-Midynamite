/*
 * midi_transpose_display.c
 *
 *  Created on: Jul 10, 2025
 *      Author: Romain Dereu
 */


#include <stdio.h>


#include "main.h"
#include "menu.h"
#include "midi_modify.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "text.h"
#include "utils.h"




uint8_t select_states_midi_transpose[3] = {0};






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
    screen_driver_underline_WriteString(modify_value, Font_6x8, White, 65, LINE_1_VERT, select_states_midi_transpose[0]);

	screen_driver_SetCursor_WriteString(message->semitones, Font_6x8 , White, TEXT_LEFT_START, LINE_2_VERT);


	screen_driver_SetCursor_WriteString(message->send_base, Font_6x8, White, TEXT_LEFT_START, LINE_4_VERT);
	const char * send_base_note_text = message_choices->no_yes[midi_transpose_data->send_original];
	screen_driver_underline_WriteString(send_base_note_text, Font_6x8, White, 65, LINE_4_VERT, select_states_midi_transpose[1]);



}


void midi_transpose_scaled_display(midi_transpose_data_struct * midi_transpose_data){


	//Base Note
	screen_driver_SetCursor_WriteString(message->root_note, Font_6x8, White, TEXT_LEFT_START, LINE_1_VERT);
	const char * base_note_text = message->twelve_notes_names[midi_transpose_data->transpose_base_note];
	screen_driver_underline_WriteString(base_note_text, Font_6x8, White, 62, LINE_1_VERT, select_states_midi_transpose[0]);


	//Transpose_interval
	screen_driver_SetCursor_WriteString(message->interval, Font_6x8, White, TEXT_LEFT_START, LINE_2_VERT);
	const char * interval_text = message_choices->intervals[midi_transpose_data->transpose_interval];
	screen_driver_underline_WriteString(interval_text, Font_6x8, White, 55, LINE_2_VERT, select_states_midi_transpose[1]);


	//Transpose_scale
	screen_driver_SetCursor_WriteString(message->scale, Font_6x8, White, TEXT_LEFT_START, LINE_3_VERT);
	const char * scale_text = message_choices->scales[midi_transpose_data->transpose_scale];
	screen_driver_underline_WriteString(scale_text, Font_6x8, White, 40, LINE_3_VERT, select_states_midi_transpose[2]);

	//Send base
	screen_driver_SetCursor_WriteString(message->send_base, Font_6x8, White, TEXT_LEFT_START, LINE_4_VERT);
	const char * send_base_note_text = message_choices->no_yes[midi_transpose_data->send_original];
	screen_driver_underline_WriteString(send_base_note_text, Font_6x8, White, 65, LINE_4_VERT, select_states_midi_transpose[3]);




}

