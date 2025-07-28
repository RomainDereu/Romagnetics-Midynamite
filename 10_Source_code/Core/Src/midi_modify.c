/*
 * midi_modify.c
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */
#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "cmsis_os.h"
#include "main.h"
#include "menu.h"
#include "midi_modify.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "text.h"
#include "utils.h"


#define BOTTOM_LINE_VERT LINE_4_VERT + 3


extern osThreadId display_updateHandle;
extern const Message * message;


static uint8_t current_select = 0;
static uint8_t old_select = 0;
static uint8_t select_states[5] = {0};



static void handle_modify_change(
    TIM_HandleTypeDef         *timer4,
    midi_modify_data_struct   *d,
    uint8_t                    select_changed,
    uint8_t                    current_select
) {
    switch (current_select) {
      case 0:
        utils_counter_change(timer4, &d->send_to_midi_channel_1, MIDI_CH_1, MIDI_CH_16, select_changed, NO_MULT, NO_WRAP);
        break;
      case 1:
        utils_counter_change(timer4, &d->send_to_midi_channel_2, NO_MIDI_CH, OCT_MULT, select_changed, NO_MULT, NO_WRAP);
        break;
      case 2:
        utils_counter_change(timer4, &d->send_to_midi_out, MIDI_OUT_1, MIDI_OUT_SPLIT, select_changed, NO_MULT, NO_WRAP);
        break;
      case 3:
        if (d->velocity_type == MIDI_MODIFY_CHANGED_VEL) {
            utils_counter_change_i32(timer4, &d->velocity_plus_minus, VOL_DEC_MAX, VOL_INC_MAX, select_changed, TEN_MULT, NO_WRAP);
        } else {
            utils_counter_change(timer4,  &d->velocity_absolute, VEL_MIN, VEL_MAX, select_changed, TEN_MULT, NO_WRAP);
        }
        break;
    }
}

static void handle_modify_split(
    TIM_HandleTypeDef         *timer4,
    midi_modify_data_struct   *d,
    uint8_t                    select_changed,
    uint8_t                    current_select
) {
    switch (current_select) {
      case 0:
        utils_counter_change(timer4, &d->split_midi_channel_1, MIDI_CH_1, MIDI_CH_16, select_changed, NO_MULT, NO_WRAP);
        break;
      case 1:
        utils_counter_change(timer4, &d->split_midi_channel_2, NO_MIDI_CH, MIDI_CH_16, select_changed, NO_MULT, NO_WRAP);
        break;
      case 2:
        utils_counter_change(timer4,  &d->split_note, VEL_MIN, VEL_MAX, select_changed, OCT_MULT, NO_WRAP);
        break;
      case 3:
        utils_counter_change(timer4, &d->send_to_midi_out,  MIDI_OUT_1, MIDI_OUT_SPLIT, select_changed, OCT_MULT, NO_WRAP);
        break;
      case 4:
        if (d->velocity_type == MIDI_MODIFY_CHANGED_VEL) {
            utils_counter_change_i32(timer4, &d->velocity_plus_minus, VOL_DEC_MAX, VOL_INC_MAX, select_changed, TEN_MULT, NO_WRAP);
        } else {
            utils_counter_change(timer4, &d->velocity_absolute, VEL_MIN, VEL_MAX, select_changed, TEN_MULT,  NO_WRAP);
        }
        break;
    }
}


//midi modify menu
void midi_modify_update_menu(TIM_HandleTypeDef * timer3,
		                     TIM_HandleTypeDef * timer4,
						     midi_modify_data_struct * midi_modify_data,
							 uint8_t * old_menu){

	midi_modify_data_struct old_modify_data = * midi_modify_data;
	uint8_t menu_changed = (*old_menu != MIDI_MODIFY);

	//The amount of values to be changed depends on the MIDI_MODIFY setting
    uint8_t amount_of_settings = (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE) ? 4 : 5;

	//Updating the selected item and see if it has changed
	utils_counter_change(timer3, &current_select, 0, amount_of_settings-1, menu_changed, 1, WRAP);
	uint8_t select_changed = (old_select != current_select);

	if (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE){
	    handle_modify_change(timer4,  midi_modify_data, select_changed, current_select);
	}


	else if (midi_modify_data->change_or_split == MIDI_MODIFY_SPLIT){
	    handle_modify_split(timer4, midi_modify_data, select_changed, current_select);
	}

	//The last line will always be velocity
	if (handle_menu_toggle(GPIOB, Btn1_Pin, Btn2_Pin)) {
	    if (current_select < amount_of_settings - 1) {
	        // any but the last → flip change_vs_split
	        utils_change_settings(&midi_modify_data->change_or_split, 0, 1);
	    } else {
	        // last row → flip velocity type
	        utils_change_settings(&midi_modify_data->velocity_type, 0, 1);
	    }

	    // always go back to the first row
	    current_select = 0;
	}

	select_current_state(select_states, amount_of_settings, current_select);

    if (menu_check_for_updates(menu_changed, &old_modify_data,
                               midi_modify_data, sizeof *midi_modify_data,
                               &old_select, &current_select)) {
        osThreadFlagsSet(display_updateHandle, FLAG_MODIFY);
    }
    old_select  = current_select;
    *old_menu   = MIDI_MODIFY;
}


//Channel
static void screen_update_channel_change(midi_modify_data_struct * midi_modify_data){
	screen_driver_SetCursor_WriteString(message->send_1_sem, Font_6x8 , White, TEXT_LEFT_START, LINE_1_VERT);
    const char * channel_1_text = message->choices.midi_channels[midi_modify_data->send_to_midi_channel_1];
    screen_driver_underline_WriteString(channel_1_text, Font_6x8 , White, 50, LINE_1_VERT, select_states[0]);

	screen_driver_SetCursor_WriteString(message->send_2_sem, Font_6x8 , White, TEXT_LEFT_START, LINE_2_VERT);
    const char * channel_2_text = message->choices.midi_channels[midi_modify_data->send_to_midi_channel_2];
    screen_driver_underline_WriteString(channel_2_text, Font_6x8 , White, 50, LINE_2_VERT, select_states[1]);

	screen_driver_SetCursor_WriteString(message->output_sem, Font_6x8 , White, TEXT_LEFT_START, LINE_3_VERT);
    const char * midi_out_text = message->choices.midi_outs[midi_modify_data->send_to_midi_out];
    screen_driver_underline_WriteString(midi_out_text, Font_6x8 , White, 50, LINE_3_VERT, select_states[2]);

}


static void screen_update_channel_split(midi_modify_data_struct * midi_modify_data){

    screen_driver_SetCursor_WriteString(message->low_sem, Font_6x8 , White, TEXT_LEFT_START, LINE_1_VERT);
    uint8_t low_channel = midi_modify_data->split_midi_channel_1;
    char low_channel_text[6];
    sprintf(low_channel_text, "%d" , low_channel);
    screen_driver_underline_WriteString(low_channel_text, Font_6x8, White, 30, LINE_1_VERT, select_states[0]);

    screen_driver_SetCursor_WriteString(message->high_sem, Font_6x8 , White, 45, LINE_1_VERT);
    uint8_t high_channel = midi_modify_data->split_midi_channel_2;
    char high_channel_text[6];
    sprintf(high_channel_text, "%d", high_channel);
    screen_driver_underline_WriteString(high_channel_text, Font_6x8, White, 80, LINE_1_VERT, select_states[1]);

	screen_driver_SetCursor_WriteString(message->split, Font_6x8, White, TEXT_LEFT_START, LINE_2_VERT);
    const char * note_to_write = message->midi_note_names[midi_modify_data->split_note];
    //Needs more clearance than the other items due to sharps and flats
    screen_driver_underline_WriteString(note_to_write, Font_6x8, White, 40, LINE_2_VERT, select_states[2]);

	screen_driver_SetCursor_WriteString(message->output_sem, Font_6x8 , White, TEXT_LEFT_START, LINE_3_VERT);
    const char * midi_out_text = message->choices.midi_outs[midi_modify_data->send_to_midi_out];
    screen_driver_underline_WriteString(midi_out_text, Font_6x8 , White, 50, LINE_3_VERT, select_states[3]);


}


//Velocity
static void screen_update_velocity_change(midi_modify_data_struct * midi_modify_data){
	screen_driver_SetCursor_WriteString(message->change_velocity, Font_6x8 , White, TEXT_LEFT_START, BOTTOM_LINE_VERT);
	//Depending on the value of midi modify, this will either be item 1 or 3
	uint8_t current_line = (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE) ? 3 : 4;
	int8_t plus_minus_i8 = midi_modify_data->velocity_plus_minus;
    char modify_value[5];
    sprintf(modify_value, "%d", plus_minus_i8);
    screen_driver_underline_WriteString(modify_value, Font_6x8, White, 100, BOTTOM_LINE_VERT, select_states[current_line]);
}

static void screen_update_velocity_fixed(midi_modify_data_struct * midi_modify_data){
	screen_driver_SetCursor_WriteString(message->fixed_velocity, Font_6x8 , White, TEXT_LEFT_START, BOTTOM_LINE_VERT);
	//Depending on the value of midi modify, this will either be item 1 or 3
	uint8_t current_line = (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE) ? 3 : 4;
    char modify_value[5];
    sprintf(modify_value, "%d", midi_modify_data->velocity_absolute);
    screen_driver_underline_WriteString(modify_value, Font_6x8, White, 100, LINE_4_VERT+3, select_states[current_line]);
}


void screen_update_midi_modify(midi_modify_data_struct * midi_modify_data){
	screen_driver_Fill(Black);

	menu_display(&Font_6x8, message->midi_modify);

	//Top part of the screen (Channel)
	if (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE){
		screen_update_channel_change(midi_modify_data);
	}

	else if(midi_modify_data->change_or_split == MIDI_MODIFY_SPLIT){
		screen_update_channel_split(midi_modify_data);
	}

	screen_driver_Line(0, LINE_4_VERT, 127, LINE_4_VERT, White);

	//Bottom part of the screen (velocity)
	if (midi_modify_data->velocity_type == MIDI_MODIFY_CHANGED_VEL){
		screen_update_velocity_change(midi_modify_data);
	}

	else if(midi_modify_data->velocity_type == MIDI_MODIFY_FIXED_VEL){
		screen_update_velocity_fixed(midi_modify_data);
	}

	//On/Off part
	midi_display_on_off(midi_modify_data->currently_sending, LINE_4_VERT);

    screen_driver_UpdateScreen();

}
