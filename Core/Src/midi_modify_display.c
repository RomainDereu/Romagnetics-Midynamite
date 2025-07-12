/*
 * midi_modify_display.c
 *
 *  Created on: Jul 2, 2025
 *      Author: Romain Dereu
 */


#include <stdio.h>
#include <stdint.h>
#include <inttypes.h>

#include "main.h"
#include "menu.h"
#include "midi_modify.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "text.h"
#include "utils.h"

#define BOTTOM_LINE_VERT LINE_4_VERT + 3


extern const Message * message;

uint8_t select_states_midi_modify[4] = {0};


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
	midi_modify_on_off(midi_modify_data->currently_sending, LINE_4_VERT);

    screen_driver_UpdateScreen();

}


//On/ Off Part
void midi_modify_on_off(uint8_t on_or_off, uint8_t bottom_line){
	screen_driver_Line(85, 10, 85, bottom_line, White);
	uint8_t text_position = bottom_line/2;
    screen_driver_SetCursor(90, text_position);

    if(on_or_off ==0){
  	  screen_driver_WriteString(message->off, Font_11x18 , White);
    }
    else if (on_or_off ==1){
  	  screen_driver_WriteString(message->on, Font_11x18 , White);
    }

}



//Channel
void screen_update_channel_change(midi_modify_data_struct * midi_modify_data){
	screen_driver_SetCursor_WriteString(message->change, Font_6x8 , White, TEXT_LEFT_START, LINE_1_VERT);
    uint8_t channel = midi_modify_data->send_to_midi_channel;

    screen_driver_SetCursor_WriteString(message->to_channel, Font_6x8 , White, TEXT_LEFT_START, LINE_2_VERT);
    char channel_text[5];
    sprintf(channel_text, "%d", channel);
    screen_driver_underline_WriteString(channel_text, Font_6x8 , White, 70, LINE_2_VERT, select_states_midi_modify[0]);
}


void screen_update_channel_split(midi_modify_data_struct * midi_modify_data){

    screen_driver_SetCursor_WriteString(message->low_to_ch, Font_6x8 , White, TEXT_LEFT_START, LINE_1_VERT);
    uint8_t low_channel = midi_modify_data->split_midi_channel_1;
    char low_channel_text[6];
    sprintf(low_channel_text, "%d" , low_channel);
    screen_driver_underline_WriteString(low_channel_text, Font_6x8, White, 70, LINE_1_VERT, select_states_midi_modify[0]);

    screen_driver_SetCursor_WriteString(message->high_to_ch, Font_6x8 , White, TEXT_LEFT_START, LINE_2_VERT);
    uint8_t high_channel = midi_modify_data->split_midi_channel_2;
    char high_channel_text[6];
    sprintf(high_channel_text, "%d", high_channel);
    screen_driver_underline_WriteString(high_channel_text, Font_6x8, White, 70, LINE_2_VERT, select_states_midi_modify[1]);

	screen_driver_SetCursor_WriteString(message->split, Font_6x8, White, TEXT_LEFT_START, LINE_3_VERT);
    const char * note_to_write = message->midi_note_names[midi_modify_data->split_note];
    //Needs more clearance than the other items due to sharps and flats
    screen_driver_underline_WriteString(note_to_write, Font_6x8, White, 40, LINE_3_VERT, select_states_midi_modify[2]);


}


//Velocity
void screen_update_velocity_change(midi_modify_data_struct * midi_modify_data){
	screen_driver_SetCursor_WriteString(message->change_velocity, Font_6x8 , White, TEXT_LEFT_START, BOTTOM_LINE_VERT);
	//Depending on the value of midi modify, this will either be item 1 or 3
	uint8_t current_line = (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE) ? 1 : 3;
	int8_t plus_minus_i8 = midi_modify_data->velocity_plus_minus;
    char modify_value[5];
    sprintf(modify_value, "%d", plus_minus_i8);
    screen_driver_underline_WriteString(modify_value, Font_6x8, White, 100, BOTTOM_LINE_VERT, select_states_midi_modify[current_line]);
}
void screen_update_velocity_fixed(midi_modify_data_struct * midi_modify_data){
	screen_driver_SetCursor_WriteString(message->fixed_velocity, Font_6x8 , White, TEXT_LEFT_START, BOTTOM_LINE_VERT);
	//Depending on the value of midi modify, this will either be item 1 or 3
	uint8_t current_line = (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE) ? 1 : 3;
    char modify_value[5];
    sprintf(modify_value, "%d", midi_modify_data->velocity_absolute);
    screen_driver_underline_WriteString(modify_value, Font_6x8, White, 100, LINE_4_VERT+3, select_states_midi_modify[current_line]);
}

