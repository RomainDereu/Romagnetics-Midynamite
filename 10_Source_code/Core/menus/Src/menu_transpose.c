/*
 * menu_transpose.c
 *
 *  Created on: Jul 10, 2025
 *      Author: Romain Dereu
 */


#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "memory_ui_state.h"
#include "memory_main.h"

#include "midi_transform.h"

#include "screen_driver.h"

//under_here_header_checks
#include "menu.h"


#include "text.h"
#include "threads.h"
#include "utils.h"



void midi_transpose_update_menu(){

	midi_transpose_data_struct old_transpose_data = save_snapshot_transpose();

	if (handle_menu_toggle(GPIOB, Btn1_Pin, Btn2_Pin)) {
		save_modify_u8(MIDI_TRANSPOSE_TRANSPOSE_TYPE, SAVE_MODIFY_INCREMENT, 0);
	    }

	static uint8_t old_select = 0;
	uint8_t current_select = ui_state_get(UI_MIDI_TRANSPOSE_SELECT);
	uint8_t transpose_type = save_get(MIDI_TRANSPOSE_TRANSPOSE_TYPE);


    ui_group_t group = (transpose_type == MIDI_TRANSPOSE_SCALED)
                       ? UI_GROUP_TRANSPOSE_SCALED
                       : UI_GROUP_TRANSPOSE_SHIFT;
    uint8_t count = build_select_states(group, current_select, NULL, 0);
    update_select(&current_select, 0, count - 1, 1, WRAP);
	ui_state_modify(UI_MIDI_TRANSPOSE_SELECT, UI_MODIFY_SET , current_select);


    toggle_underline_items(group, current_select);

	midi_transpose_data_struct new_transpose_data = save_snapshot_transpose();
	if(menu_check_for_updates(&old_transpose_data,
							  &new_transpose_data,
							  sizeof new_transpose_data,
							  &current_select,
							  &old_select)){
		threads_display_notify(FLAG_TRANSPOSE);
	}
	old_select = current_select;
}



static void midi_transpose_shift_display(uint8_t * select_states){

	write_68(message->shift_by, TEXT_LEFT_START, LINE_1_VERT);
    char modify_value[5];
    int8_t plus_minus_i8 = (int8_t)(int32_t)save_get_u32(MIDI_TRANSPOSE_MIDI_SHIFT_VALUE);
    sprintf(modify_value, "%+d", plus_minus_i8);
    write_underline_68(modify_value, 65, LINE_1_VERT, select_states[0]);

    write_68(message->semitones, TEXT_LEFT_START, LINE_2_VERT);


    write_68(message->send_base, TEXT_LEFT_START, LINE_4_VERT);
	const char * send_base_note_text = message->choices.no_yes[save_get(MIDI_TRANSPOSE_SEND_ORIGINAL)];
	write_underline_68(send_base_note_text, 65, LINE_4_VERT, select_states[1]);
}


static void midi_transpose_scaled_display(uint8_t * select_states){


	//Base Note
	write_68(message->root_note, TEXT_LEFT_START, LINE_1_VERT);
	const char * base_note_text = message->twelve_notes_names[save_get(MIDI_TRANSPOSE_BASE_NOTE)];
	write_underline_68(base_note_text, 62, LINE_1_VERT, select_states[0]);


	//Transpose_interval
	write_68(message->interval, TEXT_LEFT_START, LINE_2_VERT);
	const char * interval_text = message->choices.intervals[save_get(MIDI_TRANSPOSE_INTERVAL)];
	write_underline_68(interval_text, 55, LINE_2_VERT, select_states[1]);


	//Transpose_scale
	write_68(message->scale, TEXT_LEFT_START, LINE_3_VERT);
	const char * scale_text = message->choices.scales[save_get(MIDI_TRANSPOSE_TRANSPOSE_SCALE)];
	write_underline_68(scale_text, 40, LINE_3_VERT, select_states[2]);

	//Send base
	write_68(message->send_base, TEXT_LEFT_START, LINE_4_VERT);
	const char * send_base_note_text = message->choices.no_yes[save_get(MIDI_TRANSPOSE_SEND_ORIGINAL)];
	write_underline_68(send_base_note_text, 65, LINE_4_VERT, select_states[3]);

}



void screen_update_midi_transpose(){

	#define AMOUNT_OF_STATES 4
	uint8_t select_states[AMOUNT_OF_STATES] = {0};
	uint8_t current_select = ui_state_get(UI_MIDI_TRANSPOSE_SELECT);
	select_current_state(select_states, AMOUNT_OF_STATES, current_select);


	screen_driver_Fill(Black);
	menu_display(message->midi_transpose);

	uint8_t transpose_type = save_get(MIDI_TRANSPOSE_TRANSPOSE_TYPE);
    ui_group_t group = (transpose_type == MIDI_TRANSPOSE_SCALED)
                       ? UI_GROUP_TRANSPOSE_SCALED
                       : UI_GROUP_TRANSPOSE_SHIFT;

    // Build underline map to match handler ordering
    (void)build_select_states(group, current_select, select_states, AMOUNT_OF_STATES);

	if(transpose_type == MIDI_TRANSPOSE_SHIFT){
		midi_transpose_shift_display(select_states);
	}
	else if(transpose_type == MIDI_TRANSPOSE_SCALED){
		midi_transpose_scaled_display(select_states);
	}

	uint8_t currently_sending = save_get(MIDI_TRANSPOSE_CURRENTLY_SENDING);
	midi_display_on_off(currently_sending, 63);

    screen_driver_UpdateScreen();

}

