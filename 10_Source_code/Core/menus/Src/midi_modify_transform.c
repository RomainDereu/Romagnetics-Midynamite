/*
 * midi_modify_transform.c
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "midi_modify.h"
#include "main.h"
#include "midi_usb.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;

extern midi_modify_data_struct midi_modify_data;
extern midi_transpose_data_struct midi_transpose_data;
extern settings_data_struct settings_data;

// Circular buffer instance declared externally
extern midi_modify_circular_buffer midi_modify_buff;

// Logic functions
void midi_buffer_push(uint8_t byte) {
    uint16_t next = (midi_modify_buff.head + 1) % MIDI_MODIFY_BUFFER_SIZE;
    if (next != midi_modify_buff.tail) { // Not full
        midi_modify_buff.data[midi_modify_buff.head] = byte;
        midi_modify_buff.head = next;
    }
}

uint8_t midi_buffer_pop(uint8_t *byte) {
    if (midi_modify_buff.head == midi_modify_buff.tail)
        return 0; // Empty
    *byte = midi_modify_buff.data[midi_modify_buff.tail];
    midi_modify_buff.tail = (midi_modify_buff.tail + 1) % MIDI_MODIFY_BUFFER_SIZE;
    return 1;
}

void calculate_incoming_midi() {
    static uint8_t running_status = 0;
    static midi_note msg;
    static uint8_t byte_count = 0;
    static uint8_t expected_length = 0;

    uint8_t byte;
    while (midi_buffer_pop(&byte)) {
        // Real-time messages (do not affect parsing)
        if (byte >= 0xF8) {
            if (settings_data.midi_thru == 1) {
                midi_note rt = { .status = byte, .note = 0, .velocity = 0 };
                pipeline_final(&rt, 1);
            }
            continue;
        }

        // SysEx (skip until 0xF7)
        if (byte == 0xF0) {
            running_status = 0;
            while (midi_buffer_pop(&byte) && byte != 0xF7);
            continue;
        }

        if (byte & 0x80) {
            // Status byte
            if (byte >= 0xF0) {
                // System common — unsupported
                running_status = 0;
                continue;
            }

            running_status = byte;
            msg.status = byte;

            uint8_t status_nibble = byte & 0xF0;
            expected_length = (status_nibble == 0xC0 || status_nibble == 0xD0) ? 2 : 3;
            byte_count = 1; // waiting for data
            continue;
        }

        // Data byte
        if (running_status == 0) continue; // no valid running status

        if (byte_count == 0) {
            // We assume running status is in effect, start a new message
            msg.status = running_status;

            uint8_t status_nibble = running_status & 0xF0;
            expected_length = (status_nibble == 0xC0 || status_nibble == 0xD0) ? 2 : 3;

            msg.note = byte;
            byte_count = 2;
            if (expected_length == 2) {
                pipeline_start(&msg);
                byte_count = 0;
            }
        } else if (byte_count == 1) {
            msg.note = byte;
            byte_count++;
            if (expected_length == 2) {
                pipeline_start(&msg);
                byte_count = 0;
            }
        } else if (byte_count == 2) {
            msg.velocity = byte;
            pipeline_start(&msg);
            byte_count = 0;
        }
    }
}

static uint8_t is_channel_blocked(uint8_t status_byte) {
    if (!settings_data.channel_filter) return 0;

    uint8_t status_nibble = status_byte & 0xF0;
    uint8_t channel = status_byte & 0x0F;

    if (status_nibble >= 0x80 && status_nibble <= 0xE0) {
        return (settings_data.filtered_channels >> channel) & 0x01;
    }

    return 0;
}



void pipeline_start(midi_note *midi_msg) {
    uint8_t status = midi_msg->status;
    uint8_t length = ((status & 0xF0) == 0xC0 || (status & 0xF0) == 0xD0) ? 2 : 3;

    if (is_channel_blocked(status)) return;

    // Nothing active: do only MIDI/USB thru
    if (midi_modify_data.currently_sending == 0 &&
        midi_transpose_data.currently_sending == 0) {

        if (settings_data.midi_thru == 1) {
            send_midi_out(midi_msg, length);
        }

        if (settings_data.usb_thru == 1) {
            send_usb_midi_out(midi_msg, length);
        }

        return;
    }

    // Send to appropriate pipeline
    if (midi_modify_data.currently_sending == 1) {
        pipeline_midi_modify(midi_msg);
        return;
    } else if (midi_transpose_data.currently_sending == 1) {
        pipeline_midi_transpose(midi_msg);
        return;
    }
}


static void change_midi_channel(midi_note *midi_msg, uint8_t * send_to_midi_channel) {
    uint8_t status = midi_msg->status;
    uint8_t new_channel;

    if (status >= 0x80 && status <= 0xEF) {
        uint8_t status_nibble = status & 0xF0;
        if(midi_modify_data.change_or_split == MIDI_MODIFY_CHANGE){
            new_channel = * send_to_midi_channel;
        }
        else if(midi_modify_data.change_or_split == MIDI_MODIFY_SPLIT){
            new_channel = (midi_msg->note >= midi_modify_data.split_note) ?
                                midi_modify_data.split_midi_channel_2 : midi_modify_data.split_midi_channel_1;
        }
        midi_msg->status = status_nibble | ((new_channel - 1) & 0x0F);
    }
}

static void change_velocity(midi_note *midi_msg){
    // int16 in case of overflow
    int16_t velocity = midi_msg->velocity;
    if(midi_modify_data.velocity_type == MIDI_MODIFY_CHANGED_VEL){
        velocity += midi_modify_data.velocity_plus_minus;
    }
    else if(midi_modify_data.velocity_type == MIDI_MODIFY_FIXED_VEL){
        velocity = midi_modify_data.velocity_absolute;
    }

    // Clamp to 0-127 range
    if (velocity < 0) velocity = 0;
    if (velocity > 127) velocity = 127;

    midi_msg->velocity = (uint8_t)velocity;
}


void pipeline_midi_modify(midi_note *midi_msg){

	change_velocity(midi_msg);

	if(midi_modify_data.send_to_midi_channel_2 != 0){
		midi_note midi_note_1 = * midi_msg;
		change_midi_channel(&midi_note_1, &midi_modify_data.send_to_midi_channel_1);
		pipeline_midi_transpose(&midi_note_1);

		midi_note midi_note_2 = * midi_msg;
		change_midi_channel(&midi_note_2, &midi_modify_data.send_to_midi_channel_2);
		pipeline_midi_transpose(&midi_note_2);

	}
	else{
		midi_note midi_note_1 = * midi_msg;
		change_midi_channel(&midi_note_1, &midi_modify_data.send_to_midi_channel_1);
		pipeline_midi_transpose(&midi_note_1);
	}

}

// Transpose functions
static void get_mode_scale(uint8_t mode, uint8_t *scale) {
    const uint8_t mode_intervals[7][7] = {
        {0, 2, 4, 5, 7, 9, 11},  // Ionian
        {0, 2, 3, 5, 7, 9, 10},  // Dorian
        {0, 1, 3, 5, 7, 8, 10},  // Phrygian
        {0, 2, 4, 6, 7, 9, 11},  // Lydian
        {0, 2, 4, 5, 7, 9, 10},  // Mixolydian
        {0, 2, 3, 5, 7, 8, 10},  // Aeolian
        {0, 1, 3, 5, 6, 8, 10}   // Locrian
    };
    memcpy(scale, mode_intervals[mode], 7);
}

// Helper: check if note is in scale
static uint8_t note_in_scale(uint8_t note, uint8_t *scale, uint8_t base_note) {
    int semitone_from_root = ((note - base_note) + 120) % 12;
    for (int i = 0; i < 7; i++) {
        if (scale[i] == semitone_from_root) return 1;
    }
    return 0;
}

// Snap note up/down by semitones until in scale (max 12 semitones search)
static uint8_t snap_note_to_scale(uint8_t note, uint8_t *scale, uint8_t base_note) {
    uint8_t up_note = note;
    uint8_t down_note = note;

    for (int i = 0; i < 12; i++) {
        if (note_in_scale(down_note, scale, base_note)) return down_note;
        if (note_in_scale(up_note, scale, base_note)) return up_note;

        if (down_note > 0) down_note--;
        if (up_note < 127) up_note++;
    }
    return note;  // fallback (should not happen)
}

static uint8_t midi_transpose_notes(uint8_t note) {
    uint8_t mode = midi_transpose_data.transpose_scale % AMOUNT_OF_MODES;

    uint8_t scale_intervals[7];
    get_mode_scale(mode, scale_intervals);

    // Snap note to scale if not in scale
    if (!note_in_scale(note, scale_intervals, midi_transpose_data.transpose_base_note)) {
        note = snap_note_to_scale(note, scale_intervals, midi_transpose_data.transpose_base_note);
    }

    int16_t semitone_from_root = ((note - midi_transpose_data.transpose_base_note) + 120) % 12;
    int16_t base_octave_offset = (note - midi_transpose_data.transpose_base_note) / 12;

    int degree = -1;
    for (int i = 0; i < 7; i++) {
        if (scale_intervals[i] == semitone_from_root) {
            degree = i;
            break;
        }
    }

    // Safety fallback
    if (degree == -1) {
        return note;
    }

    const int degree_shifts[10] = {
        -7, -5, -4, -3, -2, 2, 3, 4, 5, 7
    };
    int shift = degree_shifts[midi_transpose_data.transpose_interval];

    int new_degree = degree + shift;
    int octave_shift = 0;

    while (new_degree < 0) {
        new_degree += 7;
        octave_shift -= 1;
    }
    while (new_degree >= 7) {
        new_degree -= 7;
        octave_shift += 1;
    }

    int new_note = midi_transpose_data.transpose_base_note
                 + scale_intervals[new_degree]
                 + (base_octave_offset + octave_shift) * 12;

    if (new_note < 0) new_note = 0;
    if (new_note > 127) new_note = 127;

    return new_note;
}

static void midi_pitch_shift(midi_note *midi_msg) {
    uint8_t status = midi_msg->status & 0xF0;

    if (status == 0x90 || status == 0x80) {
        int16_t note = midi_msg->note;

        if (midi_transpose_data.transpose_type == MIDI_TRANSPOSE_SCALED) {
            note = midi_transpose_notes(note);
        }
        else if (midi_transpose_data.transpose_type == MIDI_TRANSPOSE_SHIFT) {
            note += midi_transpose_data.midi_shift_value;
        }

        if (note < 0) note = 0;
        if (note > 127) note = 127;

        midi_msg->note = (uint8_t)note;
    }
}



void pipeline_midi_transpose(midi_note *midi_msg){
	if (midi_transpose_data.currently_sending == 0){
        pipeline_final(midi_msg, 3);
        return;
	}


	if (midi_transpose_data.send_original == 1) {
		midi_note pre_shift_msg = *midi_msg;

		if (midi_transpose_data.transpose_type == MIDI_TRANSPOSE_SCALED) {
			uint8_t mode = midi_transpose_data.transpose_scale % AMOUNT_OF_MODES;
			uint8_t scale_intervals[7];
			get_mode_scale(mode, scale_intervals);
			pre_shift_msg.note = snap_note_to_scale(pre_shift_msg.note, scale_intervals, midi_transpose_data.transpose_base_note);
		}

		pipeline_final(&pre_shift_msg, 3);

		midi_note shifted_msg = pre_shift_msg;
		midi_pitch_shift(&shifted_msg);
		pipeline_final(&shifted_msg, 3);
	} else {
		midi_pitch_shift(midi_msg);
		pipeline_final(midi_msg, 3);
	}
}


void pipeline_final(midi_note *midi_msg, uint8_t length){
    send_midi_out(midi_msg, length);
    send_usb_midi_out(midi_msg, length);

}




void send_midi_out(midi_note *midi_message_raw, uint8_t length) {
    if (midi_message_raw->status < 0x80)
        return;

    uint8_t midi_bytes[3] = {0};

    midi_bytes[0] = midi_message_raw->status;
    if (length > 1) midi_bytes[1] = midi_message_raw->note;
    if (length > 2) midi_bytes[2] = midi_message_raw->velocity;

    uint8_t note = (length > 1) ? midi_bytes[1] : 0;

    // Send to UART(s)
    switch (midi_modify_data.send_to_midi_out) {
        case MIDI_OUT_1:
            HAL_UART_Transmit(&huart1, midi_bytes, length, 1000);
            break;
        case MIDI_OUT_2:
            HAL_UART_Transmit(&huart2, midi_bytes, length, 1000);
            break;
        case MIDI_OUT_1_2:
            HAL_UART_Transmit(&huart1, midi_bytes, length, 1000);
            HAL_UART_Transmit(&huart2, midi_bytes, length, 1000);
            break;
        case MIDI_OUT_SPLIT:
            if (midi_modify_data.change_or_split == MIDI_MODIFY_SPLIT) {
                if (note < midi_modify_data.split_note)
                    HAL_UART_Transmit(&huart1, midi_bytes, length, 1000);
                else
                    HAL_UART_Transmit(&huart2, midi_bytes, length, 1000);
            } else {
                HAL_UART_Transmit(&huart1, midi_bytes, length, 1000);
                HAL_UART_Transmit(&huart2, midi_bytes, length, 1000);
            }
            break;
        default:
            break;
    }
}



void send_usb_midi_out(midi_note *msg, uint8_t length) {
    if (settings_data.send_to_usb == 0)
        return;

    uint8_t bytes[3] = {
        msg->status,
        msg->note,
        msg->velocity
    };
    send_usb_midi_message(bytes, length);
}


