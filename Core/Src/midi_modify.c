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

#include "menu.h"
#include "utils.h"
#include "main.h"


extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern osThreadId display_updateHandle;

// Circular buffer instance declared externally
extern midi_modify_circular_buffer midi_modify_buff;

static uint8_t midi_message[3];
static uint8_t byte_count = 0;

static uint8_t  current_select = 0;
static uint8_t  old_select = 0;


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

    while (midi_buffer_pop(&byte)) {
        if (byte >= 0xF8) {
            // Real-Time messages (1 byte only)
            uint8_t rt_msg[1] = {byte};
            send_midi_out(rt_msg, 1);
            continue;
        }

        if (byte == 0xF0) {
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
            change_midi_channel(midi_message, midi_modify_data);
            send_midi_out(midi_message, 2);
            byte_count = 0;
        }
        else if (byte_count == 3) {
            change_midi_channel(midi_message, midi_modify_data);
            send_midi_out(midi_message, 3);
            byte_count = 0;
        }
    }
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


void send_midi_out(uint8_t *midi_message, uint8_t length) {
    HAL_UART_Transmit(&huart2, midi_message, length, 1000);
}



void midi_modify_update_menu(TIM_HandleTypeDef * timer3,
		                     TIM_HandleTypeDef * timer4,
						     midi_modify_data_struct * midi_modify_data,
							 uint8_t * old_menu){

	midi_modify_data_struct old_modify_data = * midi_modify_data;
	uint8_t menu_changed = (*old_menu != MIDI_MODIFY);


	//The amount of values to be changed depends on the MIDI_MODIFY setting
    uint8_t amount_of_settings = (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE) ? 2 : 4;

	//Updating the selected item and see if it has changed
	utils_counter_change(timer3, &current_select, 0, amount_of_settings-1, menu_changed, 1, WRAP);
	uint8_t select_changed = (old_select != current_select);
	// Selecting the current item being selected
	for (uint8_t x=0; x < amount_of_settings; x++){
		select_states[x] = 0;
	}
	select_states[current_select] = 1;

	if (midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE){
		switch (current_select) {
			case 0:
				utils_counter_change(timer4, &(midi_modify_data->send_to_midi_channel), 1, 16, select_changed, 1, NO_WRAP);
				break;
			case 1:
				if (midi_modify_data->velocity_type == MIDI_MODIFY_CHANGED_VEL){
					utils_counter_change_i32(timer4, &(midi_modify_data->velocity_plus_minus), -50, 50, select_changed, 10, NO_WRAP);
					break;
				}
				else if (midi_modify_data->velocity_type == MIDI_MODIFY_FIXED_VEL){
					utils_counter_change(timer4, &(midi_modify_data->velocity_absolute), 1, 16, select_changed, 1, NO_WRAP);
					break;
			}
		}

	}


	else if (midi_modify_data->change_or_split == MIDI_MODIFY_SPLIT){
		switch (current_select) {
		case 0:
		utils_counter_change(timer4, &(midi_modify_data->split_note), 0, 127, select_changed, 12, NO_WRAP);
			break;
		case 1:
			utils_counter_change(timer4, &(midi_modify_data->split_midi_channel_1), 1, 16, select_changed, 1, NO_WRAP);
			break;
		case 2:
			utils_counter_change(timer4, &(midi_modify_data->split_midi_channel_2), 1, 16, select_changed, 1, NO_WRAP);
			break;
		case 3:
			if (midi_modify_data->velocity_type == MIDI_MODIFY_CHANGED_VEL){
				utils_counter_change_i32(timer4, &(midi_modify_data->velocity_plus_minus), -50, 50, select_changed, 10, NO_WRAP);
				break;
			}
			else if (midi_modify_data->velocity_type == MIDI_MODIFY_FIXED_VEL){
				utils_counter_change(timer4, &(midi_modify_data->velocity_absolute), 1, 16, select_changed, 10, NO_WRAP);
				break;
			}
		}

	}


	calculate_incoming_midi(midi_modify_data);

	if (menu_changed == 1 || old_select != current_select ||
		old_modify_data.send_to_midi_channel != midi_modify_data->send_to_midi_channel ||
		old_modify_data.split_note != midi_modify_data->split_note ||


		old_modify_data.split_midi_channel_1 != midi_modify_data->split_midi_channel_1 ||
		old_modify_data.split_midi_channel_2 != midi_modify_data->split_midi_channel_2 ||

		old_modify_data.velocity_plus_minus != midi_modify_data->velocity_plus_minus ||
		old_modify_data.velocity_absolute != midi_modify_data->velocity_absolute) {
		osThreadFlagsSet(display_updateHandle, 0x02);
		}
	*old_menu = MIDI_MODIFY;
	old_select = current_select;
}



