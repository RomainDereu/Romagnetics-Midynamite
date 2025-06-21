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
static char byte_print_hex[11];


void calculate_incoming_midi(uint8_t * midi_rx_buff){
    snprintf(byte_print_hex, sizeof(byte_print_hex), "%02X %02X %02X",
    midi_rx_buff[0], midi_rx_buff[1], midi_rx_buff[2]);
}

void display_incoming_midi(){

	menu_display(&Font_6x8, &message_midi_modify);
	//screen_driver_WriteString(byte_print_hex, *font , White);
	screen_driver_SetCursor(0, 54);
	screen_driver_WriteString(byte_print_hex, Font_6x8 , White);
	screen_driver_UpdateScreen();

}


void midi_modify_update_menu(uint8_t * midi_rx_buff, uint8_t * old_menu){
	if(*old_menu != MIDI_MODIFY){
		screen_driver_Fill(Black);
		}
	calculate_incoming_midi(midi_rx_buff);
	display_incoming_midi();
	*old_menu = MIDI_MODIFY;
}



