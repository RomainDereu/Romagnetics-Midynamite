/*
 * midi_modify.h
 *
 *  Created on: Feb 27, 2025
 *      Author: Astaa
 */

#ifndef INC_MIDI_MODIFY_H_
#define INC_MIDI_MODIFY_H_

#include "main.h"
#include "screen_driver.h"


void calculate_incoming_midi(uint8_t * midi_rx_buff);

void display_incoming_midi();

void midi_modify_update_menu(uint8_t * midi_rx_buff, uint8_t * old_menu);


#endif /* INC_MIDI_MODIFY_H_ */
