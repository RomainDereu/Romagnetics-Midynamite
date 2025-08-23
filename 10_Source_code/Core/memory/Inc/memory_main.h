/*
 * memory_main.h
 *
 *  Created on: Aug 23, 2025
 *      Author: Astaa
 */

#ifndef MEMORY_SAVE_H_
#define MEMORY_SAVE_H_

#include <stdint.h>
#include "stm32f4xx_hal.h"
#include "stm32f4xx_hal_flash.h"

// ---------------------
// MIDI Tempo Data
// ---------------------
typedef struct {
    int32_t current_tempo;
    int32_t tempo_click_rate;
    uint8_t currently_sending;
    uint8_t send_to_midi_out;
} midi_tempo_data_struct;

// ---------------------
// MIDI Modify Data
// ---------------------
typedef struct {
    uint8_t change_or_split;
    uint8_t velocity_type;
    uint8_t send_to_midi_out;
    uint8_t send_to_midi_channel_1;
    uint8_t send_to_midi_channel_2;
    uint8_t split_note;
    uint8_t split_midi_channel_1;
    uint8_t split_midi_channel_2;
    int32_t velocity_plus_minus;
    uint8_t velocity_absolute;
    uint8_t currently_sending;
} midi_modify_data_struct;

// ---------------------
// MIDI Transpose Data
// ---------------------
typedef struct {
    uint8_t transpose_type;
    int32_t midi_shift_value;
    uint8_t send_original;
    uint8_t transpose_base_note;
    uint8_t transpose_interval;
    uint8_t transpose_scale;
    uint8_t currently_sending;
} midi_transpose_data_struct;

// ---------------------
// Settings Data
// ---------------------
typedef struct {
    uint8_t start_menu;
    uint8_t send_to_usb;
    uint8_t brightness;
    uint8_t channel_filter;
    uint8_t midi_thru;
    uint8_t usb_thru;
    uint16_t filtered_channels;
} settings_data_struct;

// ---------------------
// Combined Save Struct
// ---------------------
typedef struct {
    midi_tempo_data_struct midi_tempo_data;
    midi_modify_data_struct midi_modify_data;
    midi_transpose_data_struct midi_transpose_data;
    settings_data_struct settings_data;
    uint32_t check_data_validity;
} save_struct;

// ---------------------
// Generic return if busy
// ---------------------
#define SAVE_STATE_BUSY 0xFF
#define DATA_VALIDITY_CHECKSUM 0xA5A5A5A5
#define FLASH_SECTOR7_ADDR  ((uint32_t)0x08060000)


typedef enum {
    SAVE_MODIFY_INCREMENT = 0,
    SAVE_MODIFY_SET
} save_modify_op_t;

// ---------------------
// Fields enumeration
// ---------------------
typedef enum {
    // midi_tempo_data
    SAVE_MIDI_TEMPO_CURRENT,
    SAVE_MIDI_TEMPO_CLICK_RATE,
    SAVE_MIDI_TEMPO_CURRENTLY_SENDING,
    SAVE_MIDI_TEMPO_SEND_TO_OUT,

    // midi_modify_data
    SAVE_MIDI_MODIFY_CHANGE_OR_SPLIT,
    SAVE_MIDI_MODIFY_VELOCITY_TYPE,
    SAVE_MIDI_MODIFY_SEND_TO_OUT,
    SAVE_MIDI_MODIFY_CHANNEL_1,
    SAVE_MIDI_MODIFY_CHANNEL_2,
    SAVE_MIDI_MODIFY_SPLIT_NOTE,
    SAVE_MIDI_MODIFY_SPLIT_CH1,
    SAVE_MIDI_MODIFY_SPLIT_CH2,
    SAVE_MIDI_MODIFY_VELOCITY_PLUS_MINUS,
    SAVE_MIDI_MODIFY_VELOCITY_ABS,
    SAVE_MIDI_MODIFY_CURRENTLY_SENDING,

    // midi_transpose_data
    SAVE_TRANSPOSE_TYPE,
    SAVE_TRANSPOSE_SHIFT_VALUE,
    SAVE_TRANSPOSE_SEND_ORIGINAL,
    SAVE_TRANSPOSE_BASE_NOTE,
    SAVE_TRANSPOSE_INTERVAL,
    SAVE_TRANSPOSE_SCALE,
    SAVE_TRANSPOSE_CURRENTLY_SENDING,

    // settings_data
    SAVE_SETTINGS_START_MENU,
    SAVE_SETTINGS_SEND_USB,
    SAVE_SETTINGS_BRIGHTNESS,
    SAVE_SETTINGS_CHANNEL_FILTER,
    SAVE_SETTINGS_MIDI_THRU,
    SAVE_SETTINGS_USB_THRU,
    SAVE_SETTINGS_FILTERED_CHANNELS,

    // checksum
    SAVE_DATA_VALIDITY,

    SAVE_FIELD_COUNT
} save_field_t;

// ---------------------
// API
// ---------------------
void save_load_from_flash(void);

HAL_StatusTypeDef store_settings(save_struct *data);

uint32_t save_get_u32(save_field_t field);
uint8_t  save_get_u8(save_field_t field);

uint8_t  save_modify_u32(save_field_t field, save_modify_op_t op, uint32_t value_if_set);
uint8_t  save_modify_u8(save_field_t field, save_modify_op_t op, uint8_t value_if_set);

// Original helpers
save_struct make_default_settings(void);
save_struct creating_save(midi_tempo_data_struct * midi_tempo_data_to_save,
                          midi_modify_data_struct * midi_modify_data_to_save,
                          midi_transpose_data_struct * midi_transpose_data_to_save,
                          settings_data_struct *settings_data_to_save);

#endif /* MEMORY_SAVE_H_ */
