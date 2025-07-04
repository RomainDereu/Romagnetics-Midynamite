/*
 * midi_modify_display.c
 *
 *  Created on: Jul 2, 2025
 *      Author: Romain Dereu
 */


#include <stdio.h>
#include <stdint.h>

#include "main.h"
#include "menu.h"
#include "midi_modify.h"
#include "screen_driver.h"
#include "screen_driver_fonts.h"
#include "text.h"
#include "utils.h"


extern const Message * message;


uint8_t select_states[4] = {0};


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



    screen_driver_UpdateScreen();

}


//Channel
void screen_update_channel_change(midi_modify_data_struct * midi_modify_data){
	screen_driver_SetCursor_WriteString(message->change_midi_channel, Font_6x8 , White, 0, LINE_1_VERT);
    uint8_t channel = midi_modify_data->send_to_midi_channel;

    screen_driver_SetCursor_WriteString(message->to_channel, Font_6x8 , White, 0, LINE_2_VERT);
    char channel_text[5];
    sprintf(channel_text, "%d", channel);
    //Needs more clearance than the other items due to sharps and flats
    screen_driver_underline_WriteString(channel_text, Font_6x8 , White, 80, LINE_2_VERT, select_states[0]);
}


void screen_update_channel_split(midi_modify_data_struct * midi_modify_data){
	screen_driver_SetCursor_WriteString(message->split_point, Font_6x8, White, 0, LINE_1_VERT);
    const char * note_to_write = message->midi_note_names[midi_modify_data->split_note];
    screen_driver_underline_WriteString(note_to_write, Font_6x8, White, 100, LINE_1_VERT, select_states[0]);


    screen_driver_SetCursor_WriteString(message->send_low_to_ch, Font_6x8 , White, 0, LINE_2_VERT);
    uint8_t low_channel = midi_modify_data->split_midi_channel_1;
    char low_channel_text[6];
    sprintf(low_channel_text, "%d" , low_channel);
    screen_driver_underline_WriteString(low_channel_text, Font_6x8, White, 100, LINE_2_VERT, select_states[1]);

    screen_driver_SetCursor_WriteString(message->send_high_to_ch, Font_6x8 , White, 0, LINE_3_VERT);
    uint8_t high_channel = midi_modify_data->split_midi_channel_2;
    char high_channel_text[6];
    sprintf(high_channel_text, "%d", high_channel);
    screen_driver_underline_WriteString(high_channel_text, Font_6x8, White, 100, LINE_3_VERT, select_states[2]);


}


//Velocity
void screen_update_velocity_change(midi_modify_data_struct * midi_modify_data){
	screen_driver_SetCursor_WriteString(message->change_velocity, Font_6x8 , White, 0, LINE_4_VERT +3);
	//Depending on the value of midi modify, this will either be item 1 or 3
	uint8_t current_line = (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE) ? 1 : 3;
    char modify_value[5];
    sprintf(modify_value, "%+d", midi_modify_data->velocity_plus_minus);
    screen_driver_underline_WriteString(modify_value, Font_6x8, White, 100, LINE_4_VERT+3, select_states[current_line]);
}
void screen_update_velocity_fixed(midi_modify_data_struct * midi_modify_data){
	screen_driver_SetCursor_WriteString(message->fixed_velocity, Font_6x8 , White, 0, LINE_4_VERT +3);
	//Depending on the value of midi modify, this will either be item 1 or 3
	uint8_t current_line = (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE) ? 1 : 3;
    char modify_value[5];
    sprintf(modify_value, "%d", midi_modify_data->velocity_absolute);
    screen_driver_underline_WriteString(modify_value, Font_6x8, White, 100, LINE_4_VERT+3, select_states[current_line]);
}

