/*
 * menu.c
 *
 *  Created on: Feb 22, 2025
 *      Author: Romain Dereu
 */



#include "menu.h"


void menu_display(const screen_driver_Font_t * font, char (* message)[30]){
	screen_driver_Line(0, 10, 127, 10, White);
	screen_driver_SetCursor(0, 0);
	screen_driver_WriteString(*message, *font , White);
	screen_driver_UpdateScreen();
}



void menu_change(TIM_HandleTypeDef * timer, uint8_t * current_menu){
	  uint8_t current_menu_counter = __HAL_TIM_GET_COUNTER(timer);
	  if (current_menu_counter > 9 && current_menu_counter < 60000)
	  {
	    __HAL_TIM_SET_COUNTER(timer,8);
	    current_menu_counter = 8;
	  }
	  else if (current_menu_counter > 60000)
	  {
	    __HAL_TIM_SET_COUNTER(timer,0);
	    current_menu_counter = 0;
	  }
	  *current_menu = current_menu_counter/4;
}
