/*
 * midi_modify.c
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */


#include "midi_modify.h"
#include "cmsis_os.h"

#include <stdio.h>
#include <stdint.h>



void display_incoming_midi(UART_HandleTypeDef huart_ptr, uint8_t (* midi_rx_buff_ptr)[3], const screen_driver_Font_t * font){

			screen_driver_SetCursor(50, 50);
			uint8_t (*midi_rx_buff)[3] = midi_rx_buff_ptr;
			uint8_t midi_one_byte_uint = (*midi_rx_buff)[0];
			uint8_t midi_two_byte_uint = (*midi_rx_buff)[1];
			uint8_t midi_three_byte_uint = (*midi_rx_buff)[2];

			char midi_one_byte_hex[3];
			char midi_two_byte_hex[3];
			char midi_three_byte_hex[3];

			snprintf(midi_one_byte_hex, sizeof(midi_one_byte_hex), "%02X", midi_one_byte_uint);
			snprintf(midi_two_byte_hex, sizeof(midi_two_byte_hex), "%02X", midi_two_byte_uint);
			snprintf(midi_three_byte_hex, sizeof(midi_three_byte_hex), "%02X", midi_three_byte_uint);

		    char byte_print_hex[11];
		    snprintf(byte_print_hex, sizeof(byte_print_hex), "%s %s %s", midi_one_byte_hex, midi_two_byte_hex, midi_three_byte_hex);


			//screen_driver_WriteString(byte_print_hex, *font , White);
			screen_driver_WriteString("Roger", *font , White);
			screen_driver_UpdateScreen();


			//Launching the next loop
	    	HAL_UART_Receive_IT(&huart_ptr, *midi_rx_buff_ptr, 3);

	    	osDelay(1);
}
