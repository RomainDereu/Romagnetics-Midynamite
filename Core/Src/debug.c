/*
 * debug.c
 *
 *  Created on: Feb 1, 2025
 *      Author: Romain Dereu
 */

#include "debug.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

 void debug_testbutton(GPIO_TypeDef * button_GPIO_Port, uint16_t  button_pin,
		  	  	  	  	char message_idle[], char message_pressed[],
						int cursor_x, int cursor_y, UART_HandleTypeDef uart_p){

		screen_driver_SetCursor(cursor_x, cursor_y);
		if (HAL_GPIO_ReadPin(button_GPIO_Port, button_pin)== 0){
			screen_driver_WriteString(message_pressed, Font_6x8, White);
		}
		else{
			screen_driver_WriteString(message_idle, Font_6x8, White);
		}

		screen_driver_UpdateScreen();
  }



 void debug_rotaryencoder(uint32_t counter,  TIM_HandleTypeDef timer_p,
		                  int cursor_x, int cursor_y){
	  if (counter > 60000)
	  {
	    __HAL_TIM_SET_COUNTER(&timer_p,0);
	    counter=0;
	  }
	  if (counter > 300)
	  {
	    __HAL_TIM_SET_COUNTER(&timer_p,300);
	    counter=300;
	  }
	  char number_print[3];
	  itoa(counter ,number_print,10);
	  //blank spaces are added to delete any remaining numbers on the screen
      char fullmessage[7];
      sprintf(fullmessage, "%s   ", number_print);
	  screen_driver_SetCursor(cursor_x, cursor_y);
	  screen_driver_WriteString(fullmessage, Font_16x24, White);
	  screen_driver_UpdateScreen();
	  HAL_Delay(counter);
 }
