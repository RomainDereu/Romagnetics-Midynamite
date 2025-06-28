/*
 * midi_modify.h
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */

#ifndef INC_MIDI_MODIFY_H_
#define INC_MIDI_MODIFY_H_

#include "main.h"
#include "screen_driver.h"

void screen_update_midi_modify();

void calculate_incoming_midi(uint8_t * sending_to_midi_channel);

void change_midi_channel(uint8_t *midi_msg, uint8_t *new_channel, uint8_t length);

void midi_buffer_push(uint8_t byte);

uint8_t midi_buffer_pop(uint8_t *byte);

void send_midi_out(uint8_t *midi_message, uint8_t length);

void midi_modify_update_menu(TIM_HandleTypeDef * timer3,
		                     TIM_HandleTypeDef * timer4,
						     midi_modify_data_struct * midi_modify_data,
							 uint8_t * old_menu);


#endif /* INC_MIDI_MODIFY_H_ */
