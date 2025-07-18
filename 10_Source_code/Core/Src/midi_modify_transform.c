/*
 * midi_modify_transform.c
 *
 *  Created on: Feb 27, 2025
 *      Author: Romain Dereu
 */
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>  // for abs()
#include <string.h>

#include "midi_modify.h"
#include "main.h"
#include "midi_usb.h"

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;


// Circular buffer instance declared externally
extern midi_modify_circular_buffer midi_modify_buff;

static uint8_t midi_message[3];
static uint8_t original_midi_message[3];
static uint8_t byte_count = 0;

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

void calculate_incoming_midi(midi_modify_data_struct * midi_modify_data,
                            midi_transpose_data_struct *midi_transpose_data) {
    uint8_t byte;
    uint8_t midi_status;

    while (midi_buffer_pop(&byte)) {
        if (byte >= 0xF8) {
            // Real-Time messages (1 byte only)
            uint8_t rt_msg[1] = {byte};
            send_midi_out(rt_msg, 1, midi_modify_data, MIDI_NOTE_ORIGINAL);
            continue;
        }

        if (byte == 0xF0) {
            // Skip SysEx messages
            while (midi_buffer_pop(&byte) && byte != 0xF7) {
                // Discard SysEx bytes
            }
            continue;
        }

        if (byte & 0x80) {
            // Status byte - start a new message
            midi_message[0] = byte;
            midi_status = byte & 0xF0;
            byte_count = 1;
        } else if (byte_count > 0 && byte_count < 3) {
            // Data byte
            midi_message[byte_count++] = byte;
        }

        // Check for complete message length:
        // Program Change (0xC0) and Channel Pressure (0xD0) are 2 bytes
        if ((byte_count == 2) && (midi_status == 0xC0 || midi_status == 0xD0)) {
            process_complete_midi_message(midi_message, 2, midi_modify_data, midi_transpose_data);
            byte_count = 0;
        }
        // Other channel messages (Note On/Off, Control Change, etc.) 3 bytes
        else if (byte_count == 3) {
            process_complete_midi_message(midi_message, 3, midi_modify_data, midi_transpose_data);
            byte_count = 0;
        }
    }
}

void change_midi_channel(uint8_t *midi_msg, midi_modify_data_struct * midi_modify_data) {
    uint8_t status = midi_msg[0];
    uint8_t new_channel;

    if (status >= 0x80 && status <= 0xEF) {
        uint8_t status_nibble = status & 0xF0;
        if(midi_modify_data->change_or_split == MIDI_MODIFY_CHANGE){
            new_channel = midi_modify_data->send_to_midi_channel;
        }
        else if(midi_modify_data->change_or_split == MIDI_MODIFY_SPLIT){
            new_channel = (midi_msg[1] >= midi_modify_data->split_note) ?
                                midi_modify_data->split_midi_channel_2 : midi_modify_data->split_midi_channel_1;
        }
        midi_msg[0] = status_nibble | ((new_channel - 1) & 0x0F);
    }
}

void change_velocity(uint8_t *midi_msg, midi_modify_data_struct * midi_modify_data){
    // int16 in case of overflow
    int16_t velocity = midi_msg[2];
    if(midi_modify_data->velocity_type == MIDI_MODIFY_CHANGED_VEL){
        velocity += midi_modify_data->velocity_plus_minus;
    }
    else if(midi_modify_data->velocity_type == MIDI_MODIFY_FIXED_VEL){
        velocity = midi_modify_data->velocity_absolute;
    }

    // Clamp to 0-127 range
    if (velocity < 0) velocity = 0;
    if (velocity > 127) velocity = 127;

    midi_msg[2] = (uint8_t)velocity;
}

// Transpose functions

// Helper: rotate scale intervals to get mode scale degrees
void get_mode_scale(uint8_t mode, uint8_t *scale) {
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
int note_in_scale(uint8_t note, uint8_t *scale, uint8_t base_note) {
    int semitone_from_root = ((note - base_note) + 120) % 12;
    for (int i = 0; i < 7; i++) {
        if (scale[i] == semitone_from_root) return 1;
    }
    return 0;
}

// Snap note up/down by semitones until in scale (max 12 semitones search)
uint8_t snap_note_to_scale(uint8_t note, uint8_t *scale, uint8_t base_note) {
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

int midi_transpose_notes(uint8_t note, midi_transpose_data_struct *transpose_data) {
    uint8_t mode = transpose_data->transpose_scale % AMOUNT_OF_MODES;

    uint8_t scale_intervals[7];
    get_mode_scale(mode, scale_intervals);

    // Snap note to scale if not in scale
    if (!note_in_scale(note, scale_intervals, transpose_data->transpose_base_note)) {
        note = snap_note_to_scale(note, scale_intervals, transpose_data->transpose_base_note);
    }

    int16_t semitone_from_root = ((note - transpose_data->transpose_base_note) + 120) % 12;
    int16_t base_octave_offset = (note - transpose_data->transpose_base_note) / 12;

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
    int shift = degree_shifts[transpose_data->transpose_interval];

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

    int new_note = transpose_data->transpose_base_note
                 + scale_intervals[new_degree]
                 + (base_octave_offset + octave_shift) * 12;

    if (new_note < 0) new_note = 0;
    if (new_note > 127) new_note = 127;

    return new_note;
}

void midi_pitch_shift(uint8_t *midi_msg, midi_transpose_data_struct *transpose_data) {
    uint8_t status = midi_msg[0] & 0xF0;

    if (status == 0x90 || status == 0x80) {
        int16_t note = midi_msg[1];

        if (transpose_data->transpose_type == MIDI_TRANSPOSE_SCALED) {
            note = midi_transpose_notes(note, transpose_data);
        }
        else if (transpose_data->transpose_type == MIDI_TRANSPOSE_SHIFT) {
            note += transpose_data->midi_shift_value;
        }

        if (note < 0) note = 0;
        if (note > 127) note = 127;

        midi_msg[1] = (uint8_t)note;
    }
}

void process_complete_midi_message(uint8_t *midi_msg, uint8_t length,
                                   midi_modify_data_struct *midi_modify_data,
                                   midi_transpose_data_struct *transpose_data) {

    for (int i = 0; i < length; ++i) {
    	original_midi_message[i] = midi_msg[i];
    }


    if (midi_modify_data->currently_sending == 1) {
        if (length == 3) {
            change_velocity(midi_msg, midi_modify_data);
        }
        change_midi_channel(midi_msg, midi_modify_data);
    }

    uint8_t status_nibble = midi_msg[0] & 0xF0;

    if ((transpose_data->transpose_type == MIDI_TRANSPOSE_SHIFT || transpose_data->transpose_type == MIDI_TRANSPOSE_SCALED)
        && transpose_data->currently_sending == 1 &&
        (status_nibble == 0x90 || status_nibble == 0x80)) {

        if (transpose_data->send_original == 1) {
            // Snap the original note to scale but don't apply interval shift
            if (transpose_data->transpose_type == MIDI_TRANSPOSE_SCALED) {
                uint8_t mode = transpose_data->transpose_scale % AMOUNT_OF_MODES;
                uint8_t scale_intervals[7];
                get_mode_scale(mode, scale_intervals);
                midi_msg[1] = snap_note_to_scale(midi_msg[1], scale_intervals, transpose_data->transpose_base_note);
            }

            send_midi_out(midi_msg, length, midi_modify_data, MIDI_NOTE_ORIGINAL);

            // Create a copy for shifted message
            uint8_t modified_msg[3];
            for (int i = 0; i < length; ++i) {
                modified_msg[i] = midi_msg[i];
            }

            // Apply full pitch shift (snap + interval) on copy
            midi_pitch_shift(modified_msg, transpose_data);

            send_midi_out(modified_msg, length, midi_modify_data, MIDI_NOTE_SHIFTED);
        } else {
            // Only send shifted note (modify original before sending)
            midi_pitch_shift(midi_msg, transpose_data);
            send_midi_out(midi_msg, length, midi_modify_data, MIDI_NOTE_SHIFTED);
        }
    } else {
        // Normal send (no pitch shift)
        send_midi_out(midi_msg, length, midi_modify_data, MIDI_NOTE_ORIGINAL);
    }
}



void send_midi_out(uint8_t *midi_message, uint8_t length, midi_modify_data_struct *midi_modify_data, uint8_t note_type) {
    uint8_t status = midi_message[0];
    if (status >= 0x80 && status <= 0xEF) {
        uint8_t note = (length > 1) ? midi_message[1] : 0;

        switch (midi_modify_data->send_to_midi_out) {
            case MIDI_OUT_1:
                HAL_UART_Transmit(&huart1, midi_message, length, 1000);
                break;

            case MIDI_OUT_2:
                HAL_UART_Transmit(&huart2, midi_message, length, 1000);
                break;

            case MIDI_OUT_1_2:
                HAL_UART_Transmit(&huart1, midi_message, length, 1000);
                HAL_UART_Transmit(&huart2, midi_message, length, 1000);
                break;

            case MIDI_OUT_SPLIT:
                if(midi_modify_data->change_or_split == MIDI_MODIFY_SPLIT){
                    if (note < midi_modify_data->split_note) {
                        HAL_UART_Transmit(&huart1, midi_message, length, 1000);
                    } else {
                        HAL_UART_Transmit(&huart2, midi_message, length, 1000);
                    }
                } else {
                    HAL_UART_Transmit(&huart1, midi_message, length, 1000);
                    HAL_UART_Transmit(&huart2, midi_message, length, 1000);
                }
                break;

            default:
                break;
        }

        send_usb_midi_message(original_midi_message, length);
        send_usb_midi_message(midi_message, length);

    }
}
