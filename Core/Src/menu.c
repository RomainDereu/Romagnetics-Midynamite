/*
 * menu.c
 *
 *  Created on: Feb 22, 2025
 *      Author: Romain Dereu
 */

#ifndef INC_MENU_C_
#define INC_MENU_C_

#include "menu.h"

extern uint8_t current_menu;

void menu_display(screen_driver_Font_t * font){
	screen_driver_Line(0, 10, 127, 10, White);
	if (current_menu == MIDI_TEMPO){
		screen_driver_SetCursor(0, 0);
		screen_driver_WriteString("Send Midi Tempo", *font , White);
		screen_driver_UpdateScreen();
	}
}

#endif /* INC_MENU_C_ */
