/*
 * midi_modify.c
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */


#include "midi_modify.h"
#include "cmsis_os.h"

void display_incoming_midi(UART_HandleTypeDef huart_ptr, uint8_t (* midi_rx_buff_ptr)[10], const screen_driver_Font_t * font){

	  if(HAL_UART_Receive(&huart_ptr, midi_rx_buff_ptr, 10, 1000)==HAL_OK) //if transfer is successful
	  {
			screen_driver_SetCursor(50, 50);
			char* midi_rx_buff_char_ptr = (char*) midi_rx_buff_ptr;
			screen_driver_WriteString(midi_rx_buff_char_ptr, *font , White);
			screen_driver_UpdateScreen();
	  } else {
	    __NOP();
	  }

	  osDelay(1);
}
