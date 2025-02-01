/*
 * debug.c
 *
 *  Created on: Feb 1, 2025
 *      Author: Romain Dereu
 */

#include "debug.h"
#include <string.h>
#include <stdlib.h>

 void debug_Testbutton(GPIO_TypeDef * button_GPIO_Port, uint16_t  button_pin,
		  	  	  	  	char message_idle[], char message_pressed[],
						int cursor_x, int cursor_y){

		ssd1306_SetCursor(cursor_x, cursor_y);
		if (HAL_GPIO_ReadPin(button_GPIO_Port, button_pin)== 0){
			ssd1306_WriteString(message_pressed, Font_6x8, White);
		}
		else{
			ssd1306_WriteString(message_idle, Font_6x8, White);
		}

		ssd1306_UpdateScreen();
  }







