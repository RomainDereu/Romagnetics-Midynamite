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


void screen_update_midi_tempo(midi_tempo_data_struct * midi_tempo_data);

void send_midi_tempo_out(UART_HandleTypeDef huart_ptr, uint32_t current_tempo);


void mt_start_stop(UART_HandleTypeDef * uart,
		           TIM_HandleTypeDef * timer,
				   midi_tempo_data_struct * midi_tempo_data);

int compareArrays(double a[], double b[], int n);

void midi_tempo_counter(TIM_HandleTypeDef * timer,
						midi_tempo_data_struct * midi_tempo_data,
						uint8_t needs_refresh);


#endif /* INC_DEBUG_H_ */
