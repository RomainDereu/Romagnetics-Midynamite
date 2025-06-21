/*
 * midi_modify.c
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */


#include "midi_modify.h"
#include "cmsis_os.h"

#include "screen_driver.h"
#include "screen_driver_fonts.h"

#include "menu.h"

#include <stdio.h>
#include <stdint.h>

char message_midi_modify[30] = "Midi Modify                   ";
//Roro test purposes only
char byte_print_hex[11];


void calculate_incoming_midi(uint8_t * midi_rx_buff){
			uint8_t midi_one_byte_uint = * midi_rx_buff;
			uint8_t midi_two_byte_uint = * (midi_rx_buff + 1);
			uint8_t midi_three_byte_uint = * (midi_rx_buff + 2);

			char midi_one_byte_hex[3];
			char midi_two_byte_hex[3];
			char midi_three_byte_hex[3];

			sprintf(midi_one_byte_hex, "%02X", midi_one_byte_uint);
			sprintf(midi_two_byte_hex, "%02X", midi_two_byte_uint);
			sprintf(midi_three_byte_hex, "%02X", midi_three_byte_uint);
		    snprintf(byte_print_hex, sizeof(byte_print_hex), "%s %s %s", midi_one_byte_hex, midi_two_byte_hex, midi_three_byte_hex);
}

void display_incoming_midi(){

	menu_display(&Font_6x8, &message_midi_modify);
	//screen_driver_WriteString(byte_print_hex, *font , White);
	screen_driver_SetCursor(0, 54);
	screen_driver_WriteString(byte_print_hex, Font_6x8 , White);
	screen_driver_UpdateScreen();

}


void screen_update_midi_modify(uint8_t * midi_rx_buff, uint8_t * old_menu){
	if(*old_menu != MIDI_MODIFY){
		screen_driver_Fill(Black);
		}
	calculate_incoming_midi(midi_rx_buff);
	display_incoming_midi();
	*old_menu = MIDI_MODIFY;
}



