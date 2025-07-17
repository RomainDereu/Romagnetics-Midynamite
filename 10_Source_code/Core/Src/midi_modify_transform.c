/*
 * midi_modify.c
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */
#include <stdio.h>
#include <stdint.h>

#include "midi_modify.h"

#include "main.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;


// Circular buffer instance declared externally
extern midi_modify_circular_buffer midi_modify_buff;



static uint8_t midi_message[3];
static uint8_t byte_count = 0;


//Logic functions
void midi_buffer_push(uint8_t byte) {
    uint16_t next = (midi_modify_buff.head + 1) % MIDI_MODIFY_BUFFER_SIZE;
    if (next != midi_modify_buff.tail) { // Not full
    	midi_modify_buff.data[midi_modify_buff.head] = byte;
    	midi_modify_buff.head = next;
    }
}

uint8_t midi_buffer_pop(uint8_t *byte) {
    if (midi_modify_buff.head == midi_modify_buff.tail)
        return 0; // Empty
    *byte = midi_modify_buff.data[midi_modify_buff.tail];
    midi_modify_buff.tail = (midi_modify_buff.tail + 1) % MIDI_MODIFY_BUFFER_SIZE;
    return 1;
}


void calculate_incoming_midi(midi_modify_data_struct * midi_modify_data) {
    uint8_t byte;
    uint8_t midi_status;

    while (midi_buffer_pop(&byte)) {
        if (byte >= 0xF8) {
            // Real-Time messages (1 byte only)
            uint8_t rt_msg[1] = {byte};
            send_midi_out(rt_msg, 1, midi_modify_data);
            continue;
        }

        if (byte == 0xF0) {
            // Skip SysEx messages
            while (midi_buffer_pop(&byte) && byte != 0xF7) {
                // Discard SysEx bytes
            }
            continue;
        }

        if (byte & 0x80) {
            // Status byte - start a new message
            midi_message[0] = byte;
            midi_status = byte & 0xF0;
            byte_count = 1;
        } else if (byte_count > 0 && byte_count < 3) {
            // Data byte
            midi_message[byte_count++] = byte;
        }

        // Check for complete message length:
        // Program Change (0xC0) and Channel Pressure (0xD0) are 2 bytes
        if ((byte_count == 2) && (midi_status == 0xC0 || midi_status == 0xD0)) {
            process_complete_midi_message(midi_message, 2, midi_modify_data);
            byte_count = 0;
        }
        // Other channel messages (Note On/Off, Control Change, etc.) 3 bytes
        else if (byte_count == 3) {
            process_complete_midi_message(midi_message, 3, midi_modify_data);
            byte_count = 0;
        }
    }
}

void process_complete_midi_message(uint8_t *midi_msg, uint8_t length,
                                   midi_modify_data_struct *midi_modify_data) {
    if (midi_modify_data->currently_sending == 1) {
        if (length == 3) {
            change_velocity(midi_msg, midi_modify_data);
        }
        change_midi_channel(midi_msg, midi_modify_data);
    }
    send_midi_out(midi_msg, length, midi_modify_data);
}

void change_midi_channel(uint8_t *midi_msg, midi_modify_data_struct * midi_modify_data) {
    uint8_t status = midi_msg[0];
    uint8_t new_channel;

    if (status >= 0x80 && status <= 0xEF) {
        uint8_t status_nibble = status & 0xF0;
    	if(midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE){
    		new_channel = midi_modify_data->send_to_midi_channel;
    	}
    	else if(midi_modify_data->change_or_split == MIDI_MODIFY_SPLIT){
    		new_channel = (midi_msg[1] >= midi_modify_data->split_note) ?
    							midi_modify_data->split_midi_channel_2 : midi_modify_data->split_midi_channel_1;
    	}
        midi_msg[0] = status_nibble | ((new_channel - 1) & 0x0F);
    }
}


void change_velocity(uint8_t *midi_msg, midi_modify_data_struct * midi_modify_data){
	//int16 in case of overflow
	int16_t  velocity = midi_msg[2];
	if(midi_modify_data->velocity_type == MIDI_MODIFY_CHANGED_VEL){
		velocity += midi_modify_data->velocity_plus_minus;
	}

	else if(midi_modify_data->velocity_type == MIDI_MODIFY_FIXED_VEL){
		velocity = midi_modify_data->velocity_absolute;
	}

    // Clamp to 0-127 range
    if (velocity < 0) velocity = 0;
    if (velocity > 127) velocity = 127;

    midi_msg[2] = (uint8_t)velocity;

}


void send_midi_out(uint8_t *midi_message, uint8_t length, midi_modify_data_struct *midi_modify_data) {
	uint8_t status = midi_message[0];
	if (status >= 0x80 && status <= 0xEF) {
		uint8_t note = (length > 1) ? midi_message[1] : 0;

		switch (midi_modify_data->send_to_midi_out) {
			case MIDI_OUT_1:
				HAL_UART_Transmit(&huart1, midi_message, length, 1000);
				break;

			case MIDI_OUT_2:
				HAL_UART_Transmit(&huart2, midi_message, length, 1000);
				break;

			case MIDI_OUT_1_2:
				HAL_UART_Transmit(&huart1, midi_message, length, 1000);
				HAL_UART_Transmit(&huart2, midi_message, length, 1000);
				break;

			case MIDI_OUT_SPLIT:

				if(midi_modify_data->change_or_split ==MIDI_MODIFY_SPLIT){
					if (note < midi_modify_data->split_note) {
						HAL_UART_Transmit(&huart1, midi_message, length, 1000);
					} else {
						HAL_UART_Transmit(&huart2, midi_message, length, 1000);
					}
				}
				else{
					HAL_UART_Transmit(&huart1, midi_message, length, 1000);
					HAL_UART_Transmit(&huart2, midi_message, length, 1000);
				}
				break;

			default:
				break;
		}
	}
}
