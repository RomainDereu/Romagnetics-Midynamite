/*
 * memory_main.h
 *
 *  Created on: Aug 23, 2025
 *      Author: Astaa
 */

#ifndef MEMORY_SAVE_H_
#define MEMORY_SAVE_H_

#include "stm32f4xx_hal.h"




// ---------------------
// Constants
// ---------------------
// Use a 32-bit sentinel so both u8/u32 getters can return it as int32_t.
#define SAVE_STATE_BUSY         ((int32_t)0x7FFFFFFF)
#define SAVE_U8_BUSY    ((uint8_t)0xFF)

#define DATA_VALIDITY_CHECKSUM  0xA5A5C6A4u
#define FLASH_SECTOR7_ADDR      ((uint32_t)0x08060000)


typedef struct {
    int32_t min;
    int32_t max;
    int32_t def;
} save_limits_t;




// ---------------------
// Modify op
// ---------------------
typedef enum {
    SAVE_MODIFY_INCREMENT = 0,
    SAVE_MODIFY_SET
} save_modify_op_t;

// ---------------------
// Fields enumeration
// ---------------------
typedef enum {
    // midi_tempo_data
    MIDI_TEMPO_CURRENT_TEMPO,
    MIDI_TEMPO_TEMPO_CLICK_RATE,
    MIDI_TEMPO_CURRENTLY_SENDING,
    MIDI_TEMPO_SEND_TO_MIDI_OUT,

    // midi_modify_data
    MIDI_MODIFY_CHANGE_OR_SPLIT,
    MIDI_MODIFY_VELOCITY_TYPE,

    MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_1,
    MIDI_MODIFY_SEND_TO_MIDI_CHANNEL_2,

    MIDI_MODIFY_SPLIT_MIDI_CHANNEL_1,
    MIDI_MODIFY_SPLIT_MIDI_CHANNEL_2,
    MIDI_MODIFY_SPLIT_NOTE,

    MIDI_MODIFY_SEND_TO_MIDI_OUT,

    MIDI_MODIFY_VELOCITY_PLUS_MINUS,
    MIDI_MODIFY_VELOCITY_ABSOLUTE,
    MIDI_MODIFY_CURRENTLY_SENDING,

    // midi_transpose_data
    MIDI_TRANSPOSE_TRANSPOSE_TYPE,
    MIDI_TRANSPOSE_MIDI_SHIFT_VALUE,
    MIDI_TRANSPOSE_BASE_NOTE,
    MIDI_TRANSPOSE_INTERVAL,
    MIDI_TRANSPOSE_TRANSPOSE_SCALE,
    MIDI_TRANSPOSE_SEND_ORIGINAL,
    MIDI_TRANSPOSE_CURRENTLY_SENDING,

    // settings_data
    SETTINGS_START_MENU,
    SETTINGS_SEND_USB,
    SETTINGS_BRIGHTNESS,
    SETTINGS_MIDI_THRU,
    SETTINGS_USB_THRU,
    SETTINGS_CHANNEL_FILTER,
    SETTINGS_FILTERED_CHANNELS,
    SETTINGS_ABOUT,

    // checksum
    SAVE_DATA_VALIDITY,

    SAVE_FIELD_COUNT
} save_field_t;



extern const save_limits_t save_limits[SAVE_FIELD_COUNT];
extern int32_t* u32_fields[SAVE_FIELD_COUNT];
extern uint8_t*  u8_fields[SAVE_FIELD_COUNT];

// (Your encoder constants still live here if you need them across modules)
#define ENCODER_CENTER     32768
#define ENCODER_THRESHOLD  4

// Some enums you already use elsewhere
typedef enum { MIDI_OUT_1, MIDI_OUT_2, MIDI_OUT_1_2, MIDI_OUT_SPLIT } midi_outs_t;
typedef enum { MIDI_MODIFY_CHANGE = 0, MIDI_MODIFY_SPLIT } midi_modify_change_split_t;
typedef enum { MIDI_MODIFY_CHANGED_VEL = 0, MIDI_MODIFY_FIXED_VEL } midi_modify_velocity_t;
typedef enum { MIDI_TRANSPOSE_SHIFT = 0, MIDI_TRANSPOSE_SCALED } midi_transpose_types_t;

typedef enum {
    IONIAN = 0, DORIAN, PHRYGIAN, LYDIAN, MIXOLYDIAN, AEOLIAN, LOCRIAN, AMOUNT_OF_MODES
} modes_t;

typedef enum {
    OCTAVE_DOWN = 0, SIXTH_DOWN, FIFTH_DOWN, FOURTH_DOWN, THIRD_DOWN,
    THIRD_UP, FOURTH_UP, FIFTH_UP, SIXTH_UP, OCTAVE_UP
} intervals_t;

typedef enum { USB_MIDI_OFF, USB_MIDI_SEND } midi_mode_t;
typedef enum { NOT_SENDING, SENDING } sending_t;
typedef enum { FALSE, TRUE } boolean_t;

typedef enum {
    FLAG_TEMPO      = (1 << 0),
    FLAG_MODIFY     = (1 << 1),
    FLAG_TRANSPOSE  = (1 << 2),
    FLAG_SETTINGS   = (1 << 3)
} DisplayFlags_t;



// ---------------------
// The following expositions are for memory_flash.c
// ---------------------
void save_unlock(void);
uint8_t save_lock_with_retries(void);


// Main getter
int32_t save_get(save_field_t field);



// Setters / modifiers
uint8_t save_modify_u32(save_field_t field, save_modify_op_t op, uint32_t value_if_set);
uint8_t save_modify_u8 (save_field_t field, save_modify_op_t op, uint8_t  value_if_set);



//Save functions in memory_flash
void save_load_from_flash(void);
HAL_StatusTypeDef store_settings(void);



#ifdef UNIT_TEST
// Re-init RAM image to defaults and notify UI
void memory_init_defaults(void);

// Simple test helpers that write via the public API
void memory_set_midi_thru(uint8_t v);

// (Optional) generic setters if you want easy test wiring everywhere:
void ut_set_u8(save_field_t f, uint8_t v);
void ut_set_u32(save_field_t f, uint32_t v);
#endif


#endif /* MEMORY_SAVE_H_ */
