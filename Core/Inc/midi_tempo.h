/*
 * debug.h
 *
 *  Created on: Feb 1, 2025
 *      Author: Romain Dereu
 */


#ifndef INC_MIDI_TEMPO_H_
#define INC_MIDI_TEMO_H_
#endif /* INC_DEBUG_H_ */

#include "main.h"


void send_midi_to_midi_out(UART_HandleTypeDef huart_ptr, uint32_t* tempo_click_rate_ptr);
