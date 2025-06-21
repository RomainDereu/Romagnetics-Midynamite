/*
 * midi_modify.c
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */
#include <stdio.h>
#include <stdint.h>

#include "midi_modify.h"
#include "cmsis_os.h"

#include "screen_driver.h"
#include "screen_driver_fonts.h"

#include "menu.h"

#include "main.h"


char message_midi_modify[30] = "Midi Modify                   ";
//Roro test purposes only

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

static char byte_print_hex[11];
static uint8_t midi_message[3];
static uint8_t byte_count = 0;

// Circular buffer instance declared externally
extern midi_modify_circular_buffer midi_modify_buff;

void midi_buffer_push(uint8_t byte) {
    uint16_t next = (midi_modify_buff.head + 1) % MIDI_MODIFY_BUFFER_SIZE;
    if (next != midi_modify_buff.tail) { // Not full
    	midi_modify_buff.data[midi_modify_buff.head] = byte;
    	midi_modify_buff.head = next;
    }
    // else: buffer full, drop byte or flag overflow
}

uint8_t midi_buffer_pop(uint8_t *byte) {
    if (midi_modify_buff.head == midi_modify_buff.tail)
        return 0; // Empty
    *byte = midi_modify_buff.data[midi_modify_buff.tail];
    midi_modify_buff.tail = (midi_modify_buff.tail + 1) % MIDI_MODIFY_BUFFER_SIZE;
    return 1;
}


void calculate_incoming_midi() {
    uint8_t byte;

    while (midi_buffer_pop(&byte)) {
        if (byte & 0x80) {
            // Status byte: start a new message
            midi_message[0] = byte;
            byte_count = 1;
        } else if (byte_count > 0 && byte_count < 3) {
            midi_message[byte_count++] = byte;
        }

        if (byte_count == 3) {
            snprintf(byte_print_hex, sizeof(byte_print_hex), "%02X %02X %02X",
                     midi_message[0], midi_message[1], midi_message[2]);
            		 send_midi_out(&midi_message);
            byte_count = 0;
            break;  // Only show one message per update call
        }
    }
}

void send_midi_out(uint8_t *midi_message) {
	  HAL_UART_Transmit(&huart1, midi_message, 3, 1000);
}

void display_incoming_midi(){

	menu_display(&Font_6x8, &message_midi_modify);
	//screen_driver_WriteString(byte_print_hex, *font , White);
	//Vertical line
	screen_driver_Line(64, 10, 64, 64, White);


	screen_driver_SetCursor(0, 54);
	screen_driver_WriteString(byte_print_hex, Font_6x8 , White);
	screen_driver_UpdateScreen();

}


void midi_modify_update_menu(uint8_t * old_menu){
	if(*old_menu != MIDI_MODIFY){
		screen_driver_Fill(Black);
		}
	calculate_incoming_midi();
	display_incoming_midi();
	*old_menu = MIDI_MODIFY;
}



