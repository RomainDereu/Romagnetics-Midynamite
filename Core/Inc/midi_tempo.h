/*
 * midi_tempo.h
 *
 *  Created on: Feb 1, 2025
 *      Author: Romain Dereu
 */


#ifndef INC_MIDI_TEMPO_H_
#define INC_MIDI_TEMO_H_

#define ENCODER_CENTER 1000
#define TICKS_PER_STEP 4


#include "main.h"
#include "screen_driver.h"


void screen_update_midi_tempo(midi_tempo_data_struct * midi_tempo_data);

void send_midi_tempo_out(UART_HandleTypeDef *UART_list[2], uint32_t current_tempo);


void mt_start_stop(UART_HandleTypeDef *UART_list[2],
		           TIM_HandleTypeDef * timer,
				   midi_tempo_data_struct * midi_tempo_data);

int compareArrays(double a[], double b[], int n);

void midi_tempo_select_counter(TIM_HandleTypeDef * timer,
                               midi_tempo_data_struct * midi_tempo_data,
							   uint8_t menu_changed);

void midi_tempo_value_counter(TIM_HandleTypeDef * timer,
                              midi_tempo_data_struct * midi_tempo_data,
						      uint8_t menu_changed);




#endif /* INC_DEBUG_H_ */
