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

 void debug_Testbutton(GPIO_TypeDef * button_GPIO_Port, uint16_t  button_pin,
		  	  	  	  	char message_idle[], char message_pressed[],
						int cursor_x, int cursor_y, UART_HandleTypeDef uart_p){

		ssd1306_SetCursor(cursor_x, cursor_y);
		if (HAL_GPIO_ReadPin(button_GPIO_Port, button_pin)== 0){
			ssd1306_WriteString(message_pressed, Font_6x8, White);
		}
		else{
			ssd1306_WriteString(message_idle, Font_6x8, White);
		}

		ssd1306_UpdateScreen();
  }



 void debug_Rotaryencoder(uint32_t counter,  TIM_HandleTypeDef timer_p,
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
	  ssd1306_SetCursor(cursor_x, cursor_y);
	  ssd1306_WriteString(fullmessage, Font_16x24, White);
	  ssd1306_UpdateScreen();
	  HAL_Delay(counter);
 }




 void send_midi_note(UART_HandleTypeDef uart_p, uint8_t cmd, uint8_t pitch, uint8_t velocity){
	 uint8_t note_info[3] = {cmd, pitch, velocity};
	 HAL_UART_Transmit(&uart_p, note_info, 3, 1000);
 }

