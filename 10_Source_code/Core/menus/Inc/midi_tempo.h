/*
 * midi_tempo.h
 *
 *  Created on: Feb 1, 2025
 *      Author: Romain Dereu
 */


#ifndef INC_MIDI_TEMPO_H_
#define INC_MIDI_TEMO_H_

#include "memory_main.h"

#include "cmsis_os.h"
#include "screen_driver.h"

void midi_tempo_update_menu();

void screen_update_midi_tempo();

void send_midi_tempo_out(int32_t tempo_click_rate, uint8_t send_to_midi_out);

void mt_start_stop(TIM_HandleTypeDef * timer);



#endif /* INC_DEBUG_H_ */
