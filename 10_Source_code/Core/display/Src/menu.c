/*
 * menu.c
 *
 *  Created on: Feb 22, 2025
 *      Author: Romain Dereu
 */


#include "memory_ui_state.h"
#include "screen_driver.h"

//under_here_header_checks
#include "cmsis_os.h"
#include "menu.h"
#include "main.h"
#include "threads.h"
#include "utils.h"

void menu_display(const char * menu_message){
	draw_line(0, 10, 127, 10);
	write_68(menu_message, TEXT_LEFT_START, 0);
}


void menu_change_check(){
	 static uint8_t button_pressed = 0;
	  if(debounce_button(GPIOB, Btn4_Pin, &button_pressed, 50)){
		  ui_state_modify(UI_CURRENT_MENU, UI_MODIFY_INCREMENT, 0);
	  }
}
