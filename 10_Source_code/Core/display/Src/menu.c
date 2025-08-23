/*
 * menu.c
 *
 *  Created on: Feb 22, 2025
 *      Author: Romain Dereu
 */



#include "cmsis_os.h"
#include "menu.h"
#include "main.h"
#include "utils.h"

void menu_display(const screen_driver_Font_t * font, const char * menu_message){
	screen_driver_Line(0, 10, 127, 10, White);
	screen_driver_SetCursor_WriteString(menu_message, *font , White, TEXT_LEFT_START, 0);
}




void menu_change_check(uint8_t * current_menu){
	 static uint8_t button_pressed = 0;
	  if(debounce_button(GPIOB, Btn4_Pin, &button_pressed, 50)){
          *current_menu+=1;
		  if(*current_menu > AMOUNT_OF_MENUS-1){
			  *current_menu = MIDI_TEMPO;
		  }
	  }
}
