/*
 * midi_tempo.h
 *
 *  Created on: Feb 1, 2025
 *      Author: Romain Dereu
 */


#ifndef INC_MIDI_TEMPO_H_
#define INC_MIDI_TEMO_H_
#include "main.h"
#include "screen_driver.h"

typedef enum {
	TEMPO_PRINT = 0,
	MIDI_OUT_PRINT,
	AMOUNT_OF_TEMPO_ITEMS
} midi_tempo_select_list_t;

void screen_update_midi_tempo(midi_tempo_data_struct * midi_tempo_data);

void send_midi_tempo_out(int32_t tempo_click_rate);

void mt_start_stop(TIM_HandleTypeDef * timer,
				   midi_tempo_data_struct * midi_tempo_data);

void midi_tempo_update_menu(TIM_HandleTypeDef * timer3,
						    TIM_HandleTypeDef * timer4,
							midi_tempo_data_struct * midi_tempo_data,
							uint8_t * old_menu);

#endif /* INC_DEBUG_H_ */
