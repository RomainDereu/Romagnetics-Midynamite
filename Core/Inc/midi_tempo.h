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

void send_midi_tempo_out(UART_HandleTypeDef *UART_list[2], uint32_t current_tempo);


void mt_start_stop(UART_HandleTypeDef *UART_list[2],
		           TIM_HandleTypeDef * timer,
				   midi_tempo_data_struct * midi_tempo_data);

int compareArrays(double a[], double b[], int n);

void midi_tempo_update_menu(TIM_HandleTypeDef * timer3,
						    TIM_HandleTypeDef * timer4,
							midi_tempo_data_struct * midi_tempo_data,
							uint8_t * old_menu);


#endif /* INC_DEBUG_H_ */
