/*
 * midi_modify_transpose.c
 *
 *  Created on: Jul 10, 2025
 *      Author: Romain Dereu
 */


#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "memory_ui_state.h"
#include "memory_main.h"

//under_here_header_checks
#include "menu.h"
#include "midi_modify.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "text.h"
#include "utils.h"


static void transpose_shift_build_select(uint8_t current_select, uint8_t select_changed)
{
    if (current_select == 0) {
    	update_value(SAVE_TRANSPOSE_SHIFT_VALUE, select_changed, 12);
    } else {
    	update_value(SAVE_TRANSPOSE_SEND_ORIGINAL, select_changed, 1);
    }
}

static void transpose_scaled_build_select(uint8_t current_select, uint8_t select_changed) {
    switch (current_select) {
    case 0:
    	update_value(SAVE_TRANSPOSE_BASE_NOTE, select_changed, 1);
        break;
    case 1:
    	update_value(SAVE_TRANSPOSE_INTERVAL, select_changed, 1);
        break;
    case 2:
    	update_value(SAVE_TRANSPOSE_SCALE, select_changed, 1);
        break;
    case 3:
    	update_value(SAVE_TRANSPOSE_SEND_ORIGINAL, select_changed, 1);
        break;
    }
}


void midi_transpose_update_menu(osThreadId_t * display_updateHandle){

	midi_transpose_data_struct old_transpose_data = save_snapshot_transpose();

	if (handle_menu_toggle(GPIOB, Btn1_Pin, Btn2_Pin)) {
		save_modify_u8(SAVE_TRANSPOSE_TYPE, SAVE_MODIFY_INCREMENT, 0);
	    }

	static uint8_t old_select = 0;

	uint8_t current_select = ui_state_get(UI_MIDI_TRANSPOSE_SELECT);
	uint8_t old_menu = ui_state_get(UI_OLD_MENU);
	uint8_t menu_changed = (old_menu != MIDI_TRANSPOSE);

	uint8_t transpose_type = save_get(SAVE_TRANSPOSE_TYPE);
    uint8_t amount_of_settings = (transpose_type == MIDI_TRANSPOSE_SCALED) ? 4 : 2;

    update_select(&current_select, 0, amount_of_settings - 1, menu_changed, 1, WRAP);
	ui_state_modify(UI_MIDI_TRANSPOSE_SELECT, UI_MODIFY_SET , current_select);

	uint8_t select_changed = (old_select != current_select);

	if (transpose_type == MIDI_TRANSPOSE_SHIFT) {
		transpose_shift_build_select(current_select,  select_changed);
	} else {
	    transpose_scaled_build_select(current_select, select_changed);
	}



	#define AMOUNT_OF_STATES 4
	uint8_t select_states[AMOUNT_OF_STATES] = {0};
	select_current_state(select_states, AMOUNT_OF_STATES, current_select);
	midi_transpose_data_struct new_transpose_data = save_snapshot_transpose();
	if(menu_check_for_updates(menu_changed,
							  &old_transpose_data,
							  &new_transpose_data,
							  sizeof new_transpose_data,
							  &current_select,
							  &old_select)){
		osThreadFlagsSet(display_updateHandle, FLAG_TRANSPOSE);
	}
	old_select = current_select;
}



static void midi_transpose_shift_display(uint8_t * select_states){

	screen_driver_SetCursor_WriteString(message->shift_by, Font_6x8 , White, TEXT_LEFT_START, LINE_1_VERT);
    char modify_value[5];
    int8_t plus_minus_i8 = (int8_t)(int32_t)save_get_u32(SAVE_TRANSPOSE_SHIFT_VALUE);
    sprintf(modify_value, "%+d", plus_minus_i8);
    screen_driver_underline_WriteString(modify_value, Font_6x8, White, 65, LINE_1_VERT, select_states[0]);

	screen_driver_SetCursor_WriteString(message->semitones, Font_6x8 , White, TEXT_LEFT_START, LINE_2_VERT);


	screen_driver_SetCursor_WriteString(message->send_base, Font_6x8, White, TEXT_LEFT_START, LINE_4_VERT);
	const char * send_base_note_text = message->choices.no_yes[save_get(SAVE_TRANSPOSE_SEND_ORIGINAL)];
	screen_driver_underline_WriteString(send_base_note_text, Font_6x8, White, 65, LINE_4_VERT, select_states[1]);
}


static void midi_transpose_scaled_display(uint8_t * select_states){


	//Base Note
	screen_driver_SetCursor_WriteString(message->root_note, Font_6x8, White, TEXT_LEFT_START, LINE_1_VERT);
	const char * base_note_text = message->twelve_notes_names[save_get(SAVE_TRANSPOSE_BASE_NOTE)];
	screen_driver_underline_WriteString(base_note_text, Font_6x8, White, 62, LINE_1_VERT, select_states[0]);


	//Transpose_interval
	screen_driver_SetCursor_WriteString(message->interval, Font_6x8, White, TEXT_LEFT_START, LINE_2_VERT);
	const char * interval_text = message->choices.intervals[save_get(SAVE_TRANSPOSE_INTERVAL)];
	screen_driver_underline_WriteString(interval_text, Font_6x8, White, 55, LINE_2_VERT, select_states[1]);


	//Transpose_scale
	screen_driver_SetCursor_WriteString(message->scale, Font_6x8, White, TEXT_LEFT_START, LINE_3_VERT);
	const char * scale_text = message->choices.scales[save_get(SAVE_TRANSPOSE_SCALE)];
	screen_driver_underline_WriteString(scale_text, Font_6x8, White, 40, LINE_3_VERT, select_states[2]);

	//Send base
	screen_driver_SetCursor_WriteString(message->send_base, Font_6x8, White, TEXT_LEFT_START, LINE_4_VERT);
	const char * send_base_note_text = message->choices.no_yes[save_get(SAVE_TRANSPOSE_SEND_ORIGINAL)];
	screen_driver_underline_WriteString(send_base_note_text, Font_6x8, White, 65, LINE_4_VERT, select_states[3]);

}



void screen_update_midi_transpose(){

	#define AMOUNT_OF_STATES 4
	uint8_t select_states[AMOUNT_OF_STATES] = {0};
	uint8_t current_select = ui_state_get(UI_MIDI_TRANSPOSE_SELECT);
	select_current_state(select_states, AMOUNT_OF_STATES, current_select);


	screen_driver_Fill(Black);
	menu_display(&Font_6x8, message->midi_transpose);

	uint8_t transpose_type = save_get(SAVE_TRANSPOSE_TYPE);

	if(transpose_type == MIDI_TRANSPOSE_SHIFT){
		midi_transpose_shift_display(select_states);
	}
	else if(transpose_type == MIDI_TRANSPOSE_SCALED){
		midi_transpose_scaled_display(select_states);
	}

	uint8_t currently_sending = save_get(SAVE_TRANSPOSE_CURRENTLY_SENDING);
	midi_display_on_off(currently_sending, 63);

    screen_driver_UpdateScreen();

}

