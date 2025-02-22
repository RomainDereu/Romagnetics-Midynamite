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


void send_midi_to_midi_out(UART_HandleTypeDef huart_ptr, uint32_t* tempo_click_rate_ptr);


void mt_press_btn3(UART_HandleTypeDef * uart, TIM_HandleTypeDef * timer, screen_driver_Font_t * font);

void mt_press_btn4(UART_HandleTypeDef * uart, TIM_HandleTypeDef * timer, screen_driver_Font_t * font);


void midi_tempo_counter(TIM_HandleTypeDef * timer, screen_driver_Font_t * font);


#endif /* INC_DEBUG_H_ */
