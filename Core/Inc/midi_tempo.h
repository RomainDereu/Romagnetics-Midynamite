/*
 * midi_tempo.h
 *
 *  Created on: Feb 1, 2025
 *      Author: Romain Dereu
 */


#ifndef INC_MIDI_TEMPO_H_
#define INC_MIDI_TEMO_H_

#define CURRENTLY_STOP 0
#define CURRENTLY_START 1


#include "main.h"
#include "screen_driver.h"


void send_midi_to_midi_out(UART_HandleTypeDef huart_ptr, uint32_t* tempo_click_rate_ptr);


void mt_start_stop(UART_HandleTypeDef * uart, TIM_HandleTypeDef * timer, const screen_driver_Font_t * font);

int compareArrays(double a[], double b[], int n);

void midi_tempo_counter(TIM_HandleTypeDef * timer, const screen_driver_Font_t * font);


#endif /* INC_DEBUG_H_ */
