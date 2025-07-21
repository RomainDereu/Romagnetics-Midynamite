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

#define MIDI_NOTE_ORIGINAL 0
#define MIDI_NOTE_SHIFTED  1

extern uint8_t select_states_midi_modify[4];
extern uint8_t select_states_midi_transpose[3];

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

void midi_modify_update_menu(TIM_HandleTypeDef * timer3,
		                     TIM_HandleTypeDef * timer4,
						     midi_modify_data_struct * midi_modify_data,
							 uint8_t * old_menu);


//midi transpose display
void screen_update_midi_transpose(midi_transpose_data_struct * midi_transpose_data);


void midi_transpose_shift_display(midi_transpose_data_struct * midi_transpose_data);
void midi_transpose_scaled_display(midi_transpose_data_struct * midi_transpose_data);

void midi_transpose_update_menu(TIM_HandleTypeDef * timer3,
		                     TIM_HandleTypeDef * timer4,
						     midi_transpose_data_struct * midi_transpose_data,
							 uint8_t * old_menu);


//midi_modify_transform
void midi_buffer_push(uint8_t byte);
uint8_t midi_buffer_pop(uint8_t *byte);

void calculate_incoming_midi();

void change_midi_channel(uint8_t *midi_msg);
void change_velocity(uint8_t *midi_msg);



//Transpose functions
void get_mode_scale(uint8_t mode, uint8_t *scale_out);
int find_scale_degree(uint8_t note_in_scale, uint8_t *scale);
int note_in_scale(uint8_t note, uint8_t *scale, uint8_t base_note);
uint8_t snap_note_to_scale(uint8_t note, uint8_t *scale, uint8_t base_note);
void midi_pitch_shift(uint8_t *midi_msg);
int midi_transpose_notes(uint8_t note);

uint8_t is_channel_blocked(uint8_t status_byte);
void process_complete_midi_message(uint8_t *midi_msg, uint8_t length) ;

void send_midi_out(uint8_t *midi_message, uint8_t length);

#endif /* INC_MIDI_MODIFY_H_ */
