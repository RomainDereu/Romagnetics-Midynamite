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
#include "utils.h"
#include "main.h"

//Messages
char message_midi_modify_print[30] = "Midi Modify                   ";
char type_of_action_print[19] = "Change Midi Channel";



extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern osThreadId display_updateHandle;

// Circular buffer instance declared externally
extern midi_modify_circular_buffer midi_modify_buff;

//Used for debug
//static char byte_print_hex[11];

static uint8_t midi_message[3];
static uint8_t byte_count = 0;



void screen_update_midi_modify(midi_modify_data_struct * midi_modify_data){
	screen_driver_Fill(Black);

	menu_display(&Font_6x8, &message_midi_modify_print);
    screen_driver_SetCursor(0, 20);
    screen_driver_WriteString(type_of_action_print, Font_6x8 , White);

    uint8_t channel = midi_modify_data->send_to_midi_channel;
    char channel_text[15];
    sprintf(channel_text, "To channel %d", channel);
    screen_driver_SetCursor(0, 30);
    screen_driver_WriteString(channel_text, Font_6x8 , White);


	//screen_driver_SetCursor(0, 54);
	//screen_driver_WriteString(byte_print_hex, Font_6x8 , White);

    screen_driver_UpdateScreen();

}


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


void calculate_incoming_midi(uint8_t * sending_to_midi_channel) {
    uint8_t byte;

    while (midi_buffer_pop(&byte)) {
        if (byte >= 0xF8) {
            // Real-Time messages (1 byte only)
            uint8_t rt_msg[1] = {byte};
            send_midi_out(rt_msg, 1);
            continue;
        }

        if (byte == 0xF0) {
            // Start of SysEx – optional: skip or buffer full message
            // For now, skip SysEx messages
            while (midi_buffer_pop(&byte) && byte != 0xF7) {
                // discard or handle sysEx byte
            }
            continue;
        }

        if (byte & 0x80) {
            // Status byte: start a new message
            midi_message[0] = byte;
            byte_count = 1;
        } else if (byte_count > 0 && byte_count < 3) {
            midi_message[byte_count++] = byte;
        }

        if (byte_count == 2 && (midi_message[0] & 0xF0) == 0xC0) {
            // Program Change or Channel Pressure: only 2 bytes
            change_midi_channel(midi_message, sending_to_midi_channel, 2);
            send_midi_out(midi_message, 2);
            byte_count = 0;
        }
        else if (byte_count == 3) {
            change_midi_channel(midi_message, sending_to_midi_channel, 3);
            send_midi_out(midi_message, 3);
            byte_count = 0;
        }
    }
}

void change_midi_channel(uint8_t *midi_msg, uint8_t *new_channel, uint8_t length) {
    if (*new_channel < 1 || *new_channel > 16) return;

    // Only change if it's a Channel Voice message (0x80–0xEF)
    uint8_t status = midi_msg[0];
    if (status >= 0x80 && status <= 0xEF) {
        uint8_t status_nibble = status & 0xF0;
        midi_msg[0] = status_nibble | ((*new_channel - 1) & 0x0F);
    }
}


void send_midi_out(uint8_t *midi_message, uint8_t length) {
    HAL_UART_Transmit(&huart1, midi_message, length, 1000);
}



void midi_modify_update_menu(TIM_HandleTypeDef * timer3,
		                     TIM_HandleTypeDef * timer4,
						     midi_modify_data_struct * midi_modify_data,
							 uint8_t * old_menu){
	uint8_t menu_changed = (*old_menu != MIDI_MODIFY);
	uint8_t old_midi_value = midi_modify_data->send_to_midi_channel;
	utils_counter_change(timer4, &(midi_modify_data->send_to_midi_channel), 1, 16, menu_changed);
	calculate_incoming_midi(&midi_modify_data->send_to_midi_channel);

	if (menu_changed == 1 || old_midi_value != midi_modify_data->send_to_midi_channel) {
		osThreadFlagsSet(display_updateHandle, 0x02);
		}
	*old_menu = MIDI_MODIFY;
}



