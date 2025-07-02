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

// List of current select
#define CHANNEL_OR_SPLIT 0
#define SEND_CHANNEL_1 1
#define SEND_CHANNEL_2 2
#define VOLUME_SETTING 3
#define AMOUNT_OF_SETTINGS 4

static uint8_t select_states[AMOUNT_OF_SETTINGS] = {0};
static uint8_t current_select = 0;
static uint8_t old_select = 0;

void screen_update_midi_modify(midi_modify_data_struct * midi_modify_data){
	screen_driver_Fill(Black);

	menu_display(&Font_6x8, message->midi_modify);

	if (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE){
		screen_update_channel_change(midi_modify_data);
	}

	else if(midi_modify_data->change_or_split == MIDI_MODIFY_SPLIT){
		screen_update_channel_split(midi_modify_data);
	}

    screen_driver_UpdateScreen();

}


void screen_update_channel_change(midi_modify_data_struct * midi_modify_data){
	screen_driver_SetCursor_WriteString(message->change_midi_channel, Font_6x8 , White, 0, LINE_1_VERT);

    uint8_t channel = midi_modify_data->send_to_midi_channel;
    char channel_text[15];
    sprintf(channel_text, "To channel %d", channel);
    screen_driver_SetCursor(0, LINE_2_VERT);
    screen_driver_WriteString(channel_text, Font_6x8 , White);
}


void screen_update_channel_split(midi_modify_data_struct * midi_modify_data){
	screen_driver_SetCursor_WriteString(message->split_point, Font_6x8, White, 0, LINE_1_VERT);


    const char * note_to_write = message->midi_note_names[midi_modify_data->split_note];
    screen_driver_SetCursor(80, LINE_1_VERT);
    screen_driver_WriteString(note_to_write, Font_6x8, White);




}
