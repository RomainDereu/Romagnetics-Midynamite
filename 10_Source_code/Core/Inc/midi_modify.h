/*
 * midi_modify.h
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */

#ifndef INC_MIDI_MODIFY_H_
#define INC_MIDI_MODIFY_H_

#include "cmsis_os.h"
#include "main.h"
#include "screen_driver.h"

#define MIDI_NOTE_ORIGINAL 0
#define MIDI_NOTE_SHIFTED  1

extern uint8_t select_states_midi_modify[4];
extern uint8_t select_states_midi_transpose[3];

typedef struct {
    uint8_t status;   ///< 0x8n = Note Off, 0x9n = Note On, etc.
    uint8_t note;     ///< 0–127 pitch value
    uint8_t velocity; ///< 0–127 velocity
} midi_note;


//midi_modify_menu
void screen_update_midi_modify(midi_modify_data_struct * midi_modify_data);
void midi_modify_update_menu(TIM_HandleTypeDef * timer3,
		                     TIM_HandleTypeDef * timer4,
						     midi_modify_data_struct * midi_modify_data,
							 uint8_t * old_menu,
							 osThreadId_t * display_updateHandle);


//midi transpose menu
void screen_update_midi_transpose(midi_transpose_data_struct * midi_transpose_data,
								  uint8_t * current_select);
void midi_transpose_update_menu(TIM_HandleTypeDef * timer3,
		                     TIM_HandleTypeDef * timer4,
						     midi_transpose_data_struct * midi_transpose_data,
							 uint8_t * old_menu,
							 uint8_t * current_select,
							 osThreadId_t * display_updateHandle);


//midi_modify_transform
void midi_buffer_push(uint8_t byte);
uint8_t midi_buffer_pop(uint8_t *byte);

void calculate_incoming_midi();

void pipeline_start(midi_note *midi_msg);
void pipeline_midi_modify(midi_note *midi_msg);
void pipeline_midi_transpose(midi_note *midi_msg);
void pipeline_final(midi_note *midi_msg, uint8_t length) ;

void send_midi_out(midi_note *midi_message_raw, uint8_t length);
void send_usb_midi_out(midi_note *midi_message_raw, uint8_t length);

#endif /* INC_MIDI_MODIFY_H_ */
