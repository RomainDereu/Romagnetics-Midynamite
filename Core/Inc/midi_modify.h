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

extern uint8_t select_states_midi_modify[4];

//midi_modify_display
void screen_update_midi_modify(midi_modify_data_struct * midi_modify_data);

//On Off part
void midi_modify_on_off(uint8_t data_to_modify, uint8_t bottom_line);

//Channel
void screen_update_channel_change(midi_modify_data_struct * midi_modify_data);
void screen_update_channel_split(midi_modify_data_struct * midi_modify_data);

//Velocity
void screen_update_velocity_change(midi_modify_data_struct * midi_modify_data);
void screen_update_velocity_fixed(midi_modify_data_struct * midi_modify_data);



//midi_modify
void midi_buffer_push(uint8_t byte);

uint8_t midi_buffer_pop(uint8_t *byte);

void calculate_incoming_midi(midi_modify_data_struct * midi_modify_data);

void change_midi_channel(uint8_t *midi_msg, midi_modify_data_struct * midi_modify_data);

void change_velocity(uint8_t *midi_msg, midi_modify_data_struct * midi_modify_data);


void send_midi_out(uint8_t *midi_message, uint8_t length);

void midi_modify_update_menu(TIM_HandleTypeDef * timer3,
		                     TIM_HandleTypeDef * timer4,
						     midi_modify_data_struct * midi_modify_data,
							 uint8_t * old_menu);


//midi transpose display
void screen_update_midi_transpose(midi_transpose_data_struct * midi_transpose_data);


#endif /* INC_MIDI_MODIFY_H_ */
