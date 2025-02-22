/*
 * menu.c
 *
 *  Created on: Feb 22, 2025
 *      Author: Romain Dereu
 */



#include "menu.h"


void menu_display(const screen_driver_Font_t * font, uint8_t * current_menu){
	screen_driver_Line(0, 10, 127, 10, White);
	if (*current_menu == MIDI_TEMPO){
		screen_driver_SetCursor(0, 0);
		screen_driver_WriteString("Send Midi Tempo", *font , White);
		screen_driver_UpdateScreen();
	}
}



void menu_change(TIM_HandleTypeDef * timer, uint8_t * current_menu){
	  uint8_t current_menu_counter = __HAL_TIM_GET_COUNTER(timer);
	  if (current_menu_counter > 3 && current_menu_counter < 60000)
	  {
	    __HAL_TIM_SET_COUNTER(timer,4);
	    current_menu_counter = 4;
	  }
	  else if (current_menu_counter > 60000)
	  {
	    __HAL_TIM_SET_COUNTER(timer,0);
	    current_menu_counter = 0;
	  }
	  *current_menu = current_menu_counter/4;
}
