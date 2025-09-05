/*
 * midi_tranform.h
 *
 *  Created on: Sep 5, 2025
 *      Author: Astaa
 */

#ifndef MIDI_INC_MIDI_TRANSFORM_H_
#define MIDI_INC_MIDI_TRANSFORM_H_

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "menu_modify.h"
#include "main.h"
#include "midi_usb.h"
#include "memory_main.h"

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


#endif /* MIDI_INC_MIDI_TRANSFORM_H_ */
